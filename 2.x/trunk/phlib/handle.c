/*
 * Process Hacker - 
 *   handle table
 * 
 * Copyright (C) 2010 wj32
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

#include <phbase.h>
#include <handlep.h>

PPH_FREE_LIST PhHandleTableLevel0FreeList;
PPH_FREE_LIST PhHandleTableLevel1FreeList;

VOID PhHandleTableInitialization()
{
    PhHandleTableLevel0FreeList = PhCreateFreeList(
        sizeof(PH_HANDLE_TABLE_ENTRY) * PH_HANDLE_TABLE_LEVEL_ENTRIES,
        64
        );
    PhHandleTableLevel1FreeList = PhCreateFreeList(
        sizeof(PPH_HANDLE_TABLE_ENTRY) * PH_HANDLE_TABLE_LEVEL_ENTRIES,
        32
        );
}

PPH_HANDLE_TABLE PhCreateHandleTable()
{
    PPH_HANDLE_TABLE handleTable;
    ULONG i;

    handleTable = PhAllocate(sizeof(PH_HANDLE_TABLE));

    PhInitializeQueuedLock(&handleTable->Lock);
    PhInitializeQueuedLock(&handleTable->LockedCondition);

    handleTable->NextValue = 0;

    handleTable->Count = 0;
    handleTable->TableValue = (ULONG_PTR)PhpCreateHandleTableLevel0(handleTable, TRUE);

    // We have now created the level 0 table.
    // The free list can now be set up to point to handle 0, which 
    // points to the rest of the free list (1 -> 2 -> 3 -> ...).
    // The next batch of handles that need to be created start at 
    // PH_HANDLE_TABLE_LEVEL_ENTRIES. 

    handleTable->FreeValue = 0;
    handleTable->NextValue = PH_HANDLE_TABLE_LEVEL_ENTRIES;

    handleTable->FreeValueAlt = PH_HANDLE_VALUE_INVALID; // no entries in alt. free list

    handleTable->Flags = 0;

    for (i = 0; i < PH_HANDLE_TABLE_LOCKS; i++)
        PhInitializeQueuedLock(&handleTable->Locks[i]);

    return handleTable;
}

VOID PhDestroyHandleTable(
    __in __post_invalid PPH_HANDLE_TABLE HandleTable
    )
{
    ULONG_PTR tableValue;
    ULONG tableLevel;
    PPH_HANDLE_TABLE_ENTRY table0;
    PPH_HANDLE_TABLE_ENTRY *table1;
    PPH_HANDLE_TABLE_ENTRY **table2;
    ULONG i;
    ULONG j;

    tableValue = HandleTable->TableValue;
    tableLevel = tableValue & PH_HANDLE_TABLE_LEVEL_MASK;
    tableValue -= tableLevel;

    switch (tableLevel)
    {
    case 0:
        {
            table0 = (PPH_HANDLE_TABLE_ENTRY)tableValue;

            PhpFreeHandleTableLevel0(table0);
        }
        break;
    case 1:
        {
            table1 = (PPH_HANDLE_TABLE_ENTRY *)tableValue;

            for (i = 0; i < PH_HANDLE_TABLE_LEVEL_ENTRIES; i++)
            {
                if (!table1[i])
                    break;

                PhpFreeHandleTableLevel0(table1[i]);
            }

            PhpFreeHandleTableLevel1(table1);
        }
        break;
    case 2:
        {
            table2 = (PPH_HANDLE_TABLE_ENTRY **)tableValue;

            for (i = 0; i < PH_HANDLE_TABLE_LEVEL_ENTRIES; i++)
            {
                if (!table2[i])
                    break;

                for (j = 0; j < PH_HANDLE_TABLE_LEVEL_ENTRIES; j++)
                {
                    if (!table2[i][j])
                        break;

                    PhpFreeHandleTableLevel0(table2[i][j]);
                }

                PhpFreeHandleTableLevel1(table2[i]);
            }

            PhpFreeHandleTableLevel2(table2);
        }
        break;
    }

    PhFree(HandleTable);
}

VOID PhLockHandleTableEntry(
    __inout PPH_HANDLE_TABLE HandleTable,
    __inout PPH_HANDLE_TABLE_ENTRY HandleTableEntry
    )
{
    while (!_interlockedbittestandreset(
        (PLONG)&HandleTableEntry->Value,
        PH_HANDLE_TABLE_ENTRY_LOCKED_SHIFT
        ))
    {
        PhWaitForCondition(&HandleTable->LockedCondition, NULL, NULL);
    }
}

BOOLEAN PhLockInUseHandleTableEntry(
    __inout PPH_HANDLE_TABLE HandleTable,
    __inout PPH_HANDLE_TABLE_ENTRY HandleTableEntry
    )
{
    ULONG_PTR value;

    while (TRUE)
    {
        value = HandleTableEntry->Value;

        if ((value & PH_HANDLE_TABLE_ENTRY_TYPE) != PH_HANDLE_TABLE_ENTRY_IN_USE)
            return FALSE;

        if (value & PH_HANDLE_TABLE_ENTRY_LOCKED)
        {
            if ((ULONG_PTR)_InterlockedCompareExchangePointer(
                (PVOID *)&HandleTableEntry->Value,
                (PVOID)(value - PH_HANDLE_TABLE_ENTRY_LOCKED),
                (PVOID)value
                ) == value)
            {
                return TRUE;
            }
        }

        PhWaitForCondition(&HandleTable->LockedCondition, NULL, NULL);
    }
}

VOID PhUnlockHandleTableEntry(
    __inout PPH_HANDLE_TABLE HandleTable,
    __inout PPH_HANDLE_TABLE_ENTRY HandleTableEntry
    )
{
    _interlockedbittestandset(
        (PLONG)&HandleTableEntry->Value,
        PH_HANDLE_TABLE_ENTRY_LOCKED_SHIFT
        );
    PhPulseAllCondition(&HandleTable->LockedCondition);
}

HANDLE PhCreateHandle(
    __inout PPH_HANDLE_TABLE HandleTable,
    __in PPH_HANDLE_TABLE_ENTRY HandleTableEntry
    )
{
    PPH_HANDLE_TABLE_ENTRY entry;
    ULONG handleValue;

    entry = PhpAllocateHandleTableEntry(HandleTable, &handleValue);

    if (!entry)
        return NULL;

    // Copy the given handle table entry to the allocated entry.
    entry->TypeAndValue.Type = PH_HANDLE_TABLE_ENTRY_IN_USE | PH_HANDLE_TABLE_ENTRY_LOCKED;
    entry->TypeAndValue.Value = HandleTableEntry->TypeAndValue.Value;
    entry->Value2 = HandleTableEntry->Value2;

    return PhpEncodeHandle(handleValue);
}

BOOLEAN PhDestroyHandle(
    __inout PPH_HANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __in_opt __assumeLocked PPH_HANDLE_TABLE_ENTRY HandleTableEntry
    )
{
    ULONG handleValue;

    handleValue = PhpDecodeHandle(Handle);

    if (!HandleTableEntry)
    {
        HandleTableEntry = PhpLookupHandleTableEntry(HandleTable, handleValue);

        if (!HandleTableEntry)
            return FALSE;

        if (!PhLockInUseHandleTableEntry(HandleTable, HandleTableEntry))
            return FALSE;
    }

    _InterlockedExchangePointer(
        (PVOID *)&HandleTableEntry->Value,
        (PVOID)(PH_HANDLE_TABLE_ENTRY_FREE | PH_HANDLE_TABLE_ENTRY_LOCKED)
        );

    // The handle table entry now has the lock bit cleared, so we 
    // should wake waiters.
    PhPulseAllCondition(&HandleTable->LockedCondition);

    PhpFreeHandleTableEntry(HandleTable, handleValue, HandleTableEntry);

    return TRUE;
}

PPH_HANDLE_TABLE_ENTRY PhGetHandleTableEntry(
    __in PPH_HANDLE_TABLE HandleTable,
    __in HANDLE Handle
    )
{
    PPH_HANDLE_TABLE_ENTRY entry;

    entry = PhpLookupHandleTableEntry(HandleTable, PhpDecodeHandle(Handle));

    if (!entry)
        return NULL;

    if (!PhLockInUseHandleTableEntry(HandleTable, entry))
        return NULL;

    return entry;
}

VOID PhEnumHandleTable(
    __in PPH_HANDLE_TABLE HandleTable,
    __in PPH_ENUM_HANDLE_TABLE_CALLBACK Callback,
    __in PVOID Context
    )
{
    ULONG handleValue;
    PPH_HANDLE_TABLE_ENTRY entry;
    BOOLEAN cont;

    handleValue = 0;

    while (entry = PhpLookupHandleTableEntry(HandleTable, handleValue))
    {
        if (PhLockInUseHandleTableEntry(HandleTable, entry))
        {
            cont = Callback(
                HandleTable,
                PhpEncodeHandle(handleValue),
                entry,
                Context
                );
            PhUnlockHandleTableEntry(HandleTable, entry);

            if (!cont)
                break;
        }

        handleValue++;
    }
}

VOID PhSweepHandleTable(
    __in PPH_HANDLE_TABLE HandleTable,
    __in PPH_ENUM_HANDLE_TABLE_CALLBACK Callback,
    __in PVOID Context
    )
{
    ULONG handleValue;
    PPH_HANDLE_TABLE_ENTRY entry;
    BOOLEAN cont;

    handleValue = 0;

    while (entry = PhpLookupHandleTableEntry(HandleTable, handleValue))
    {
        if (entry->TypeAndValue.Type == PH_HANDLE_TABLE_ENTRY_IN_USE)
        {
            cont = Callback(
                HandleTable,
                PhpEncodeHandle(handleValue),
                entry,
                Context
                );

            if (!cont)
                break;
        }

        handleValue++;
    }
}

PPH_HANDLE_TABLE_ENTRY PhpAllocateHandleTableEntry(
    __inout PPH_HANDLE_TABLE HandleTable,
    __out PULONG HandleValue
    )
{
    PPH_HANDLE_TABLE_ENTRY entry;
    ULONG freeValue;
    ULONG lockIndex;
    ULONG nextFreeValue;
    ULONG oldFreeValue;
    BOOLEAN result;

    while (TRUE)
    {
        freeValue = HandleTable->FreeValue;

        while (freeValue == PH_HANDLE_VALUE_INVALID)
        {
            PhAcquireQueuedLockExclusiveFast(&HandleTable->Lock);

            // Check again to see if we have free handles.

            freeValue = *(volatile ULONG *)&HandleTable->FreeValue;

            if (freeValue != PH_HANDLE_VALUE_INVALID)
            {
                PhReleaseQueuedLockExclusive(&HandleTable->Lock);
                break;
            }

            // Move handles from the alt. free list to the main free list, 
            // and check again.

            freeValue = PhpMoveFreeHandleTableEntries(HandleTable);

            if (freeValue != PH_HANDLE_VALUE_INVALID)
            {
                PhReleaseQueuedLockExclusive(&HandleTable->Lock);
                break;
            }

            result = PhpAllocateMoreHandleTableEntries(HandleTable, TRUE);

            PhReleaseQueuedLockExclusive(&HandleTable->Lock);

            freeValue = HandleTable->FreeValue;

            // Note that PhpAllocateMoreHandleTableEntries only 
            // returns FALSE if it failed to allocate memory. 
            // Success does not guarantee a free handle to be 
            // allocated, as they may have been all used up (however 
            // unlikely) when we reach this point. Success simply 
            // means to retry the allocation using the fast path.

            if (!result && freeValue == PH_HANDLE_VALUE_INVALID)
                return NULL;
        }

        entry = PhpLookupHandleTableEntry(HandleTable, freeValue);
        lockIndex = PH_HANDLE_TABLE_LOCK_INDEX(freeValue);

        // To avoid the ABA problem, we would ideally have one 
        // queued lock per handle table entry. That would make the 
        // overhead too large, so instead there is a fixed number of 
        // locks, indexed by the handle value (mod no. locks).

        // Possibilities at this point:
        // 1. freeValue != A (our copy), but the other thread has freed 
        // A, so FreeValue = A. No ABA problem since freeValue != A.
        // 2. freeValue != A, and FreeValue != A. No ABA problem.
        // 3. freeValue = A, and the other thread has freed A, so 
        // FreeValue = A. No ABA problem since we haven't read 
        // NextFreeValue yet.
        // 4. freeValue = A, and FreeValue != A. No problem if this 
        // stays the same later, as the CAS will take care of it.

        PhpLockHandleTableShared(HandleTable, lockIndex);

        if (*(volatile ULONG *)&HandleTable->FreeValue != freeValue)
        {
            PhpUnlockHandleTableShared(HandleTable, lockIndex);
            continue;
        }

        MemoryBarrier();

        nextFreeValue = *(volatile ULONG *)&entry->NextFreeValue;

        // Possibilities/non-possibilities at this point:
        // 1. freeValue != A (our copy), but the other thread has freed 
        // A, so FreeValue = A. This is actually impossible since we 
        // have acquired the lock on A and the free code checks that 
        // and uses the alt. free list instead.
        // 2. freeValue != A, and FreeValue != A. No ABA problem.
        // 3. freeValue = A, and the other thread has freed A, so 
        // FreeValue = A. Impossible like above. This is *the* ABA 
        // problem which we have now prevented.
        // 4. freeValue = A, and FreeValue != A. CAS will take care 
        // of it.

        oldFreeValue = _InterlockedCompareExchange(
            &HandleTable->FreeValue,
            nextFreeValue,
            freeValue
            );

        PhpUnlockHandleTableShared(HandleTable, lockIndex);

        if (oldFreeValue == freeValue)
            break;
    }

    _InterlockedIncrement((PLONG)&HandleTable->Count);

    *HandleValue = freeValue;

    return entry;
}

VOID PhpFreeHandleTableEntry(
    __inout PPH_HANDLE_TABLE HandleTable,
    __in ULONG HandleValue,
    __inout PPH_HANDLE_TABLE_ENTRY HandleTableEntry
    )
{
    PULONG freeList;
    ULONG flags;
    ULONG oldValue;

    _InterlockedDecrement((PLONG)&HandleTable->Count);

    flags = HandleTable->Flags;

    // Choose the free list to use depending on whether someone 
    // is popping from the main free list (see 
    // PhpAllocateHandleTableEntry for details). We always use 
    // the alt. free list if strict FIFO is enabled.
    if (!(flags & PH_HANDLE_TABLE_STRICT_FIFO) &&
        PhTryAcquireReleaseQueuedLockExclusive(
        &HandleTable->Locks[PH_HANDLE_TABLE_LOCK_INDEX(HandleValue)]))
    {
        freeList = &HandleTable->FreeValue;
    }
    else
    {
        freeList = &HandleTable->FreeValueAlt;
    }

    while (TRUE)
    {
        oldValue = *freeList;
        HandleTableEntry->NextFreeValue = oldValue;

        if (_InterlockedCompareExchange(
            freeList,
            HandleValue,
            oldValue
            ) == oldValue)
        {
            break;
        }
    }
}

BOOLEAN PhpAllocateMoreHandleTableEntries(
    __in __assumeLocked PPH_HANDLE_TABLE HandleTable,
    __in BOOLEAN Initialize
    )
{
    ULONG_PTR tableValue;
    ULONG tableLevel;
    PPH_HANDLE_TABLE_ENTRY table0;
    PPH_HANDLE_TABLE_ENTRY *table1;
    PPH_HANDLE_TABLE_ENTRY **table2;
    ULONG i;
    ULONG j;
    ULONG oldNextValue;
    ULONG freeValue;

    // Get a pointer to the table, and its level.

    tableValue = HandleTable->TableValue;
    tableLevel = tableValue & PH_HANDLE_TABLE_LEVEL_MASK;
    tableValue -= tableLevel;

    switch (tableLevel)
    {
    case 0:
        {
            // Create a level 1 table.

            table1 = PhpCreateHandleTableLevel1(HandleTable);

#ifdef PH_HANDLE_TABLE_SAFE
            if (!table1)
                return FALSE;
#endif

            // Create a new level 0 table and move the existing level into 
            // the new level 1 table.

            table0 = PhpCreateHandleTableLevel0(HandleTable, Initialize);

#ifdef PH_HANDLE_TABLE_SAFE
            if (!table0)
            {
                PhpFreeHandleTableLevel1(table1);
                return FALSE;
            }
#endif

            table1[0] = (PPH_HANDLE_TABLE_ENTRY)tableValue;
            table1[1] = table0;

            tableValue = (ULONG_PTR)table1 | 1;
            //_InterlockedExchangePointer((PVOID *)&HandleTable->TableValue, (PVOID)tableValue);
            HandleTable->TableValue = tableValue;
        }
        break;
    case 1:
        {
            table1 = (PPH_HANDLE_TABLE_ENTRY *)tableValue;

            // Determine whether we need to create a new level 0 table or 
            // create a level 2 table.

            i = HandleTable->NextValue / PH_HANDLE_TABLE_LEVEL_ENTRIES;

            if (i < PH_HANDLE_TABLE_LEVEL_ENTRIES)
            {
                table0 = PhpCreateHandleTableLevel0(HandleTable, TRUE);

#ifdef PH_HANDLE_TABLE_SAFE
                if (!table0)
                    return FALSE;
#endif

                //_InterlockedExchangePointer((PVOID *)&table1[i], table0);
                table1[i] = table0;
            }
            else
            {
                // Create a level 2 table.

                table2 = PhpCreateHandleTableLevel2(HandleTable);

#ifdef PH_HANDLE_TABLE_SAFE
                if (!table2)
                    return FALSE;
#endif

                // Create a new level 1 table and move the existing level into 
                // the new level 2 table.

                table1 = PhpCreateHandleTableLevel1(HandleTable);

#ifdef PH_HANDLE_TABLE_SAFE
                if (table1)
                {
                    PhpFreeHandleTableLevel2(table2);
                    return FALSE;
                }
#endif

                table0 = PhpCreateHandleTableLevel0(HandleTable, Initialize);

#ifdef PH_HANDLE_TABLE_SAFE
                if (!table0)
                {
                    PhpFreeHandleTableLevel1(table1);
                    PhpFreeHandleTableLevel2(table2);
                    return FALSE;
                }
#endif

                table1[0] = table0;

                table2[0] = (PPH_HANDLE_TABLE_ENTRY *)tableValue;
                table2[1] = table1;

                tableValue = (ULONG_PTR)table2 | 2;
                //_InterlockedExchangePointer((PVOID *)&HandleTable->TableValue, (PVOID)tableValue);
                HandleTable->TableValue = tableValue;
            }
        }
        break;
    case 2:
        {
            table2 = (PPH_HANDLE_TABLE_ENTRY **)tableValue;

            i = HandleTable->NextValue /
                (PH_HANDLE_TABLE_LEVEL_ENTRIES * PH_HANDLE_TABLE_LEVEL_ENTRIES);
            // i contains an index into the level 2 table, of the containing 
            // level 1 table.

            // Check if we have exceeded the maximum number of handles.

            if (i >= PH_HANDLE_TABLE_LEVEL_ENTRIES)
                return FALSE;

            // Check if we should create a new level 0 table or a new level 2 
            // table.
            if (table2[i])
            {
                table0 = PhpCreateHandleTableLevel0(HandleTable, Initialize);

#ifdef PH_HANDLE_TABLE_SAFE
                if (!table0)
                    return FALSE;
#endif

                // Same as j = HandleTable->NextValue % (no. entries * no. entries), 
                // but we already calculated i so just use it.
                j = HandleTable->NextValue - i * 
                    (PH_HANDLE_TABLE_LEVEL_ENTRIES * PH_HANDLE_TABLE_LEVEL_ENTRIES);
                j /= PH_HANDLE_TABLE_LEVEL_ENTRIES;
                // j now contains an index into the level 1 table, of the containing 
                // level 0 table (the one which was created).

                //_InterlockedExchangePointer((PVOID *)&table2[i][j], table0);
                table2[i][j] = table0;
            }
            else
            {
                table1 = PhpCreateHandleTableLevel1(HandleTable);

#ifdef PH_HANDLE_TABLE_SAFE
                if (!table1)
                    return FALSE;
#endif

                table0 = PhpCreateHandleTableLevel0(HandleTable, TRUE);

#ifdef PH_HANDLE_TABLE_SAFE
                if (!table0)
                {
                    PhpFreeHandleTableLevel1(table1);
                    return FALSE;
                }
#endif

                table1[0] = table0;

                //_InterlockedExchangePointer((PVOID *)&table2[i], table1);
                table2[i] = table1;
            }
        }
        break;
    }

    // In each of the cases above, we allocated one additional level 0 table.
    oldNextValue = _InterlockedExchangeAdd(
        (PLONG)&HandleTable->NextValue,
        PH_HANDLE_TABLE_LEVEL_ENTRIES
        );

    if (Initialize)
    {
        // No ABA problem since these are new handles being pushed.

        while (TRUE)
        {
            freeValue = HandleTable->FreeValue;
            table0[PH_HANDLE_TABLE_LEVEL_ENTRIES - 1].NextFreeValue = freeValue;

            if (_InterlockedCompareExchange(
                &HandleTable->FreeValue,
                oldNextValue,
                freeValue
                ) == freeValue)
            {
                break;
            }
        }
    }

    return TRUE;
}

PPH_HANDLE_TABLE_ENTRY PhpLookupHandleTableEntry(
    __in PPH_HANDLE_TABLE HandleTable,
    __in ULONG HandleValue
    )
{
    ULONG_PTR tableValue;
    ULONG tableLevel;
    PPH_HANDLE_TABLE_ENTRY table0;
    PPH_HANDLE_TABLE_ENTRY *table1;
    PPH_HANDLE_TABLE_ENTRY **table2;
    PPH_HANDLE_TABLE_ENTRY entry;

    if (HandleValue >= HandleTable->NextValue)
        return NULL;

    // Get a pointer to the table, and its level.

    tableValue = HandleTable->TableValue;
    tableLevel = tableValue & PH_HANDLE_TABLE_LEVEL_MASK;
    tableValue -= tableLevel;

    // No additional checking needed; aleady checked against 
    // NextValue.

    switch (tableLevel)
    {
    case 0:
        {
            table0 = (PPH_HANDLE_TABLE_ENTRY)tableValue;
            entry = &table0[HandleValue]; 
        }
        break;
    case 1:
        {
            table1 = (PPH_HANDLE_TABLE_ENTRY *)tableValue;
            table0 = table1[PH_HANDLE_VALUE_LEVEL1_U(HandleValue)];
            entry = &table0[PH_HANDLE_VALUE_LEVEL0(HandleValue)];
        }
        break;
    case 2:
        {
            table2 = (PPH_HANDLE_TABLE_ENTRY **)tableValue;
            table1 = table2[PH_HANDLE_VALUE_LEVEL2_U(HandleValue)];
            table0 = table1[PH_HANDLE_VALUE_LEVEL1(HandleValue)];
            entry = &table0[PH_HANDLE_VALUE_LEVEL0(HandleValue)];
        }
        break;
    }

    return entry;
}

ULONG PhpMoveFreeHandleTableEntries(
    __inout __assumeLocked PPH_HANDLE_TABLE HandleTable
    )
{
    ULONG freeValueAlt;
    ULONG flags;
    ULONG i;
    ULONG index;
    ULONG nextIndex;
    ULONG lastIndex;
    PPH_HANDLE_TABLE_ENTRY entry;
    PPH_HANDLE_TABLE_ENTRY firstEntry;
    ULONG count;
    ULONG freeValue;

    // Remove all entries from the alt. free list.
    freeValueAlt = _InterlockedExchange(&HandleTable->FreeValueAlt, PH_HANDLE_VALUE_INVALID);

    if (freeValueAlt == PH_HANDLE_VALUE_INVALID)
    {
        // No handles on the alt. free list.
        return PH_HANDLE_VALUE_INVALID;
    }

    // Avoid the ABA problem by testing all locks (see 
    // PhpAllocateHandleTableEntry for details). Unlike 
    // in PhpFreeHandleTableEntry we have no "alternative" 
    // list, so we must allow blocking.
    for (i = 0; i < PH_HANDLE_TABLE_LOCKS; i++)
        PhAcquireReleaseQueuedLockExclusive(&HandleTable->Locks[i]);

    flags = HandleTable->Flags;

    if (!(flags & PH_HANDLE_TABLE_STRICT_FIFO))
    {
        // Shortcut: if there are no entries in the main free list and 
        // we don't need to reverse the chain, just return.
        if (_InterlockedCompareExchange(
            &HandleTable->FreeValue,
            freeValueAlt,
            PH_HANDLE_VALUE_INVALID
            ) == PH_HANDLE_VALUE_INVALID)
            return freeValueAlt;
    }

    // Reverse the chain (even if strict FIFO is off; we have to 
    // traverse the list to find the last entry, so we might as well 
    // reverse it along the way).

    index = freeValueAlt;
    lastIndex = PH_HANDLE_VALUE_INVALID;
    count = 0;

    while (TRUE)
    {
        entry = PhpLookupHandleTableEntry(HandleTable, index);
        count++;

        if (lastIndex == PH_HANDLE_VALUE_INVALID)
            firstEntry = entry;

        nextIndex = entry->NextFreeValue;
        entry->NextFreeValue = lastIndex;
        lastIndex = index;

        if (nextIndex == PH_HANDLE_VALUE_INVALID)
            break;

        index = nextIndex;
    }

    // Note that firstEntry actually contains the last free 
    // entry, since we reversed the list. Similarly 
    // index/lastIndex both contain the index of the first 
    // free entry.

    // Push the entries onto the free list.
    while (TRUE)
    {
        freeValue = HandleTable->FreeValue;
        firstEntry->NextFreeValue = freeValue;

        if (_InterlockedCompareExchange(
            &HandleTable->FreeValue,
            index,
            freeValue
            ) == freeValue)
            break;
    }

    // Force expansion if we don't have enough free handles.
    if (
        (flags & PH_HANDLE_TABLE_STRICT_FIFO) &&
        count < PH_HANDLE_TABLE_FREE_COUNT
        )
    {
        freeValueAlt = PH_HANDLE_VALUE_INVALID;
    }

    return freeValueAlt;
}

PPH_HANDLE_TABLE_ENTRY PhpCreateHandleTableLevel0(
    __in PPH_HANDLE_TABLE HandleTable,
    __in BOOLEAN Initialize
    )
{
    PPH_HANDLE_TABLE_ENTRY table;
    PPH_HANDLE_TABLE_ENTRY entry;
    ULONG baseValue;
    ULONG i;

#ifdef PH_HANDLE_TABLE_SAFE
    __try
    {
        table = PhAllocateFromFreeList(PhHandleTableLevel0FreeList);
    }
    __except (SIMPLE_EXCEPTION_FILTER(GetExceptionCode() == STATUS_INSUFFICIENT_RESOURCES))
    {
        return NULL;
    }
#else
    table = PhAllocateFromFreeList(PhHandleTableLevel0FreeList);
#endif

    if (Initialize)
    {
        entry = &table[0];
        baseValue = HandleTable->NextValue;

        for (i = baseValue + 1; i < baseValue + PH_HANDLE_TABLE_LEVEL_ENTRIES; i++)
        {
            entry->Value = PH_HANDLE_TABLE_ENTRY_FREE | PH_HANDLE_TABLE_ENTRY_LOCKED;
            entry->NextFreeValue = i;
            entry++;
        }

        entry->Value = PH_HANDLE_TABLE_ENTRY_FREE | PH_HANDLE_TABLE_ENTRY_LOCKED;
        entry->NextFreeValue = PH_HANDLE_VALUE_INVALID;
    }

    return table;
}

VOID PhpFreeHandleTableLevel0(
    __in PPH_HANDLE_TABLE_ENTRY Table
    )
{
    PhFreeToFreeList(PhHandleTableLevel0FreeList, Table);
}

PPH_HANDLE_TABLE_ENTRY *PhpCreateHandleTableLevel1(
    __in PPH_HANDLE_TABLE HandleTable
    )
{
    PPH_HANDLE_TABLE_ENTRY *table;

#ifdef PH_HANDLE_TABLE_SAFE
    __try
    {
        table = PhAllocateFromFreeList(PhHandleTableLevel1FreeList);
    }
    __except (SIMPLE_EXCEPTION_FILTER(GetExceptionCode() == STATUS_INSUFFICIENT_RESOURCES))
    {
        return NULL;
    }
#else
    table = PhAllocateFromFreeList(PhHandleTableLevel1FreeList);
#endif

    memset(table, 0, sizeof(PPH_HANDLE_TABLE_ENTRY) * PH_HANDLE_TABLE_LEVEL_ENTRIES);

    return table;
}

VOID PhpFreeHandleTableLevel1(
    __in PPH_HANDLE_TABLE_ENTRY *Table
    )
{
    PhFreeToFreeList(PhHandleTableLevel1FreeList, Table);
}

PPH_HANDLE_TABLE_ENTRY **PhpCreateHandleTableLevel2(
    __in PPH_HANDLE_TABLE HandleTable
    )
{
    PPH_HANDLE_TABLE_ENTRY **table;

#ifdef PH_HANDLE_TABLE_SAFE
    __try
    {
        table = PhAllocate(sizeof(PPH_HANDLE_TABLE_ENTRY *) * PH_HANDLE_TABLE_LEVEL_ENTRIES);
    }
    __except (SIMPLE_EXCEPTION_FILTER(GetExceptionCode() == STATUS_INSUFFICIENT_RESOURCES))
    {
        return NULL;
    }
#else
    table = PhAllocate(sizeof(PPH_HANDLE_TABLE_ENTRY *) * PH_HANDLE_TABLE_LEVEL_ENTRIES);
#endif

    memset(table, 0, sizeof(PPH_HANDLE_TABLE_ENTRY *) * PH_HANDLE_TABLE_LEVEL_ENTRIES);

    return table;
}

VOID PhpFreeHandleTableLevel2(
    __in PPH_HANDLE_TABLE_ENTRY **Table
    )
{
    PhFree(Table);
}
