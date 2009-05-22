/*
 * Process Hacker Driver - 
 *   hooks
 * 
 * Copyright (C) 2009 wj32
 * 
 * This file is part of Process Hacker.
 * 
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "include/hook.h"
#include "include/sync.h"

typedef struct _MAPPED_MDL
{
    PMDL Mdl;
    PVOID Address;
} MAPPED_MDL, *PMAPPED_MDL;

NTSTATUS KphpCreateMappedMdl(
    PVOID Address,
    ULONG Length,
    PMAPPED_MDL MappedMdl
    );

VOID KphpFreeMappedMdl(
    PMAPPED_MDL MappedMdl
    );

static KPH_PROCESSOR_LOCK HookProcessorLock;

/* KphHookInit
 * 
 * Initializes the hooking module.
 */
NTSTATUS KphHookInit()
{
    KphInitializeProcessorLock(&HookProcessorLock);
    
    return STATUS_SUCCESS;
}

/* KphHook
 * 
 * Hooks a kernel-mode function.
 * WARNING: DO NOT HOOK A FUNCTION THAT IS CALLABLE ABOVE APC_LEVEL.
 * 
 * Thread safety: Full
 * IRQL: <= APC_LEVEL
 */
NTSTATUS KphHook(
    PKPH_HOOK Hook
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    MAPPED_MDL mappedMdl;
    PCHAR function;
    
    status = KphpCreateMappedMdl(
        Hook->Function,
        5,
        &mappedMdl
        );
    
    if (!NT_SUCCESS(status))
        return status;
    
    function = (PCHAR)mappedMdl.Address;
    
    /* Acquire a lock on all other processors. */
    if (KphAcquireProcessorLock(&HookProcessorLock))
    {
        /* Note that this is completely safe even though we are at 
         * DISPATCH_LEVEL because we are using the mapped MDL.
         */
        /* Copy the original five bytes (for unhooking). */
        memcpy(Hook->Bytes, function, 5);
        /* Hook the function by writing a jump instruction. */
        Hook->Hooked = TRUE;
        /* jmp Target */
        *function = 0xe9;
        *(PULONG_PTR)(function + 1) = (ULONG_PTR)Hook->Target - (ULONG_PTR)Hook->Function - 5;
        
        /* Release the processor lock. */
        KphReleaseProcessorLock(&HookProcessorLock);
    }
    else
    {
        dprintf("KphHook: Could not acquire processor lock!\n");
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    KphpFreeMappedMdl(&mappedMdl);
    
    return status;
}

/* KphUnhook
 * 
 * Unhooks a kernel-mode function.
 * WARNING: DO NOT UNHOOK A FUNCTION THAT IS CALLABLE ABOVE APC_LEVEL.
 * 
 * Thread safety: Full
 * IRQL: <= APC_LEVEL
 */
NTSTATUS KphUnhook(
    PKPH_HOOK Hook
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    MAPPED_MDL mappedMdl;
    
    if (!Hook->Hooked)
        return STATUS_UNSUCCESSFUL;
    
    status = KphpCreateMappedMdl(
        Hook->Function,
        5,
        &mappedMdl
        );
    
    if (!NT_SUCCESS(status))
        return status;
    
    /* Acquire a lock on all other processors. */
    if (KphAcquireProcessorLock(&HookProcessorLock))
    {
        /* Unpatch the function. */
        memcpy(mappedMdl.Address, Hook->Bytes, 5);
        Hook->Hooked = FALSE;
        /* Release the processor lock. */
        KphReleaseProcessorLock(&HookProcessorLock);
    }
    else
    {
        dprintf("KphUnhook: Could not acquire processor lock!\n");
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    KphpFreeMappedMdl(&mappedMdl);
    
    return status;
}

/* KphObOpenCall
 * 
 * Calls the original open procedure for an object type.
 * 
 * AccessMode: If this argument is unavailable, specify KernelMode.
 */
NTSTATUS NTAPI KphObOpenCall(
    PKPH_OB_OPEN_HOOK ObOpenHook,
    OB_OPEN_REASON OpenReason,
    KPROCESSOR_MODE AccessMode,
    PEPROCESS Process,
    PVOID Object,
    ACCESS_MASK GrantedAccess,
    ULONG HandleCount
    )
{
    /* If there wasn't any original open procedure, exit. */
    if (!ObOpenHook->Function)
        return STATUS_SUCCESS;
    
    if (WindowsVersion == WINDOWS_XP)
    {
        return ((OB_OPEN_METHOD_51)ObOpenHook->Function)(
            OpenReason,
            Process,
            Object,
            GrantedAccess,
            HandleCount
            );
    }
    else if (
        WindowsVersion == WINDOWS_VISTA || 
        WindowsVersion == WINDOWS_7
        )
    {
        return ((OB_OPEN_METHOD_60)ObOpenHook->Function)(
            OpenReason,
            AccessMode,
            Process,
            Object,
            GrantedAccess,
            HandleCount
            );
    }
    else
    {
        return STATUS_NOT_SUPPORTED;
    }
}

/* KphObOpenHook
 * 
 * Hooks the open procedure for an object type.
 * 
 * Thread safety: Full
 * IRQL: <= APC_LEVEL
 */
NTSTATUS KphObOpenHook(
    PKPH_OB_OPEN_HOOK ObOpenHook
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    MAPPED_MDL mappedMdl;
    PVOID *openProcedure;
    
    status = KphpCreateMappedMdl(
        KVOFF(ObOpenHook->ObjectType, OffOtiOpenProcedure),
        sizeof(PVOID),
        &mappedMdl
        );
    
    if (!NT_SUCCESS(status))
        return status;
    
    openProcedure = (PVOID *)mappedMdl.Address;
    
    /* Acquire a lock on all other processors. */
    if (KphAcquireProcessorLock(&HookProcessorLock))
    {
        /* Save the original open procedure pointer. */
        ObOpenHook->Function = *openProcedure;
        
        /* Choose the correct target open procedure and hook. */
        if (WindowsVersion == WINDOWS_XP)
        {
            if (ObOpenHook->Target51)
                *openProcedure = ObOpenHook->Target51;
            else
                status = STATUS_INVALID_PARAMETER;
        }
        else if (
            WindowsVersion == WINDOWS_VISTA || 
            WindowsVersion == WINDOWS_7
            )
        {
            if (ObOpenHook->Target60)
                *openProcedure = ObOpenHook->Target60;
            else
                status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            status = STATUS_NOT_SUPPORTED;
        }
        
        ObOpenHook->Hooked = TRUE;
        
        /* Release the processor lock. */
        KphReleaseProcessorLock(&HookProcessorLock);
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    KphpFreeMappedMdl(&mappedMdl);
    
    return status;
}

/* KphObOpenUnhook
 * 
 * Unhooks the open procedure for an object type.
 * 
 * Thread safety: Full
 * IRQL: <= APC_LEVEL
 */
NTSTATUS KphObOpenUnhook(
    PKPH_OB_OPEN_HOOK ObOpenHook
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    MAPPED_MDL mappedMdl;
    PVOID *openProcedure;
    
    if (!ObOpenHook->Hooked)
        return STATUS_UNSUCCESSFUL;
    
    status = KphpCreateMappedMdl(
        KVOFF(ObOpenHook->ObjectType, OffOtiOpenProcedure),
        sizeof(PVOID),
        &mappedMdl
        );
    
    if (!NT_SUCCESS(status))
        return status;
    
    openProcedure = (PVOID *)mappedMdl.Address;
    
    /* Acquire a lock on all other processors. */
    if (KphAcquireProcessorLock(&HookProcessorLock))
    {
        /* Restore the original open procedure pointer. */
        *openProcedure = ObOpenHook->Function;
        ObOpenHook->Hooked = FALSE;
        
        /* Release the processor lock. */
        KphReleaseProcessorLock(&HookProcessorLock);
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    KphpFreeMappedMdl(&mappedMdl);
    
    return status;
}

/* KphpCreateMappedMdl
 * 
 * Creates and maps a MDL.
 * 
 * Thread safety: Full
 * IRQL: Any
 */
NTSTATUS KphpCreateMappedMdl(
    PVOID Address,
    ULONG Length,
    PMAPPED_MDL MappedMdl
    )
{
    PMDL mdl;
    
    MappedMdl->Mdl = NULL;
    MappedMdl->Address = NULL;
    
    mdl = IoAllocateMdl(Address, Length, FALSE, FALSE, NULL);
    
    if (mdl == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;
    
    MmBuildMdlForNonPagedPool(mdl);
    mdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;
    MappedMdl->Address = MmMapLockedPagesSpecifyCache(
        mdl,
        KernelMode,
        MmNonCached,
        NULL,
        FALSE,
        HighPagePriority
        );
    MappedMdl->Mdl = mdl;
    
    if (!MappedMdl->Address)
    {
        KphpFreeMappedMdl(MappedMdl);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    return STATUS_SUCCESS;
}

/* KphpFreeMappedMdl
 * 
 * Unmaps and frees a MDL.
 * 
 * Thread safety: Full
 * IRQL: Any
 */
VOID KphpFreeMappedMdl(
    PMAPPED_MDL MappedMdl
    )
{
    if (MappedMdl->Mdl != NULL)
    {
        if (MappedMdl->Address != NULL)
        {
            MmUnmapLockedPages(MappedMdl->Address, MappedMdl->Mdl);
            MappedMdl->Address = NULL;
        }
        
        IoFreeMdl(MappedMdl->Mdl);
        MappedMdl->Mdl = NULL;
    }
}
