#ifndef CLRSUP_H
#define CLRSUP_H

#define CINTERFACE
#define COBJMACROS
#include <clrdata.h>
#undef CINTERFACE
#undef COBJMACROS

// General interfaces

typedef struct _CLR_PROCESS_SUPPORT
{
    struct IXCLRDataProcess *DataProcess;
} CLR_PROCESS_SUPPORT, *PCLR_PROCESS_SUPPORT;

PCLR_PROCESS_SUPPORT CreateClrProcessSupport(
    __in HANDLE ProcessId
    );

VOID FreeClrProcessSupport(
    __in PCLR_PROCESS_SUPPORT Support
    );

PPH_STRING GetRuntimeNameByAddressClrProcess(
    __in PCLR_PROCESS_SUPPORT Support,
    __in ULONG64 Address,
    __out_opt PULONG64 Displacement
    );

PPH_STRING GetNameXClrDataAppDomain(
    __in PVOID AppDomain
    );

PVOID LoadMscordacwks(
    __in BOOLEAN IsClrV4
    );

HRESULT CreateXCLRDataProcess(
    __in HANDLE ProcessId,
    __in ICLRDataTarget *Target,
    __out struct IXCLRDataProcess **DataProcess
    );

// xclrdata

typedef ULONG64 CLRDATA_ENUM;

typedef struct IXCLRDataProcess IXCLRDataProcess;
typedef struct IXCLRDataAppDomain IXCLRDataAppDomain;
typedef struct IXCLRDataTask IXCLRDataTask;
typedef struct IXCLRDataStackWalk IXCLRDataStackWalk;
typedef struct IXCLRDataFrame IXCLRDataFrame;

typedef struct IXCLRDataProcessVtbl
{
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        __in IXCLRDataProcess *This,
        __in REFIID riid,
        __deref_out void **ppvObject
        );

    ULONG (STDMETHODCALLTYPE *AddRef)(
        __in IXCLRDataProcess *This
        );

    ULONG (STDMETHODCALLTYPE *Release)(
        __in IXCLRDataProcess *This
        );

    HRESULT (STDMETHODCALLTYPE *Flush)(
        __in IXCLRDataProcess *This
        );

    HRESULT (STDMETHODCALLTYPE *StartEnumTasks)(
        __in IXCLRDataProcess *This,
        __out CLRDATA_ENUM *handle
        );

    HRESULT (STDMETHODCALLTYPE *EnumTask)(
        __in IXCLRDataProcess *This,
        __inout CLRDATA_ENUM *handle,
        __out IXCLRDataTask **task
        );

    HRESULT (STDMETHODCALLTYPE *EndEnumTasks)(
        __in IXCLRDataProcess *This,
        __in CLRDATA_ENUM handle
        );

    HRESULT (STDMETHODCALLTYPE *GetTaskByOSThreadID)(
        __in IXCLRDataProcess *This,
        __in ULONG32 osThreadID,
        __out IXCLRDataTask **task
        );

    PVOID GetTaskByUniqueID;
    PVOID GetFlags;
    PVOID IsSameObject;
    PVOID GetManagedObject;
    PVOID GetDesiredExecutionState;
    PVOID SetDesiredExecutionState;
    PVOID GetAddressType;

    HRESULT (STDMETHODCALLTYPE *GetRuntimeNameByAddress)(
        __in IXCLRDataProcess *This,
        __in CLRDATA_ADDRESS address,
        __in ULONG32 flags,
        __in ULONG32 bufLen,
        __out ULONG32 *nameLen,
        __out WCHAR *nameBuf,
        __out CLRDATA_ADDRESS *displacement
        );

    // ...
} IXCLRDataProcessVtbl;

typedef struct IXCLRDataProcess
{
    struct IXCLRDataProcessVtbl *lpVtbl;
} IXCLRDataProcess;

#define IXCLRDataProcess_QueryInterface(This, riid, ppvObject) \
    ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))

#define IXCLRDataProcess_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IXCLRDataProcess_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IXCLRDataProcess_GetRuntimeNameByAddress(This, address, flags, bufLen, nameLen, nameBuf, displacement) \
    ((This)->lpVtbl->GetRuntimeNameByAddress(This, address, flags, bufLen, nameLen, nameBuf, displacement))

#define IXCLRDataProcess_Flush(This) \
    ((This)->lpVtbl->Flush(This))

#define IXCLRDataProcess_StartEnumTasks(This, handle) \
    ((This)->lpVtbl->StartEnumTasks(This, handle))

#define IXCLRDataProcess_EnumTask(This, handle, task) \
    ((This)->lpVtbl->EnumTask(This, handle, task))

#define IXCLRDataProcess_EndEnumTasks(This, handle) \
    ((This)->lpVtbl->EndEnumTasks(This, handle))

#define IXCLRDataProcess_GetTaskByOSThreadID(This, osThreadID, task) \
    ((This)->lpVtbl->GetTaskByOSThreadID(This, osThreadID, task))

typedef struct IXCLRDataAppDomainVtbl
{
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        __in IXCLRDataAppDomain *This,
        __in REFIID riid,
        __deref_out void **ppvObject
        );

    ULONG (STDMETHODCALLTYPE *AddRef)(
        __in IXCLRDataAppDomain *This
        );

    ULONG (STDMETHODCALLTYPE *Release)(
        __in IXCLRDataAppDomain *This
        );

    HRESULT (STDMETHODCALLTYPE *GetProcess)(
        __in IXCLRDataAppDomain *This,
        __out IXCLRDataProcess **process
        );

    HRESULT (STDMETHODCALLTYPE *GetName)(
        __in IXCLRDataAppDomain *This,
        __in ULONG32 bufLen,
        __out ULONG32 *nameLen,
        __out WCHAR *name
        );

    HRESULT (STDMETHODCALLTYPE *GetUniqueID)(
        __in IXCLRDataAppDomain *This,
        __out ULONG64 *id
        );

    // ...
} IXCLRDataAppDomainVtbl;

typedef struct IXCLRDataAppDomain
{
    struct IXCLRDataAppDomainVtbl *lpVtbl;
} IXCLRDataAppDomain;

#define IXCLRDataAppDomain_QueryInterface(This, riid, ppvObject) \
    ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))

#define IXCLRDataAppDomain_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IXCLRDataAppDomain_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IXCLRDataAppDomain_GetProcess(This, process) \
    ((This)->lpVtbl->GetProcess(This, process))

#define IXCLRDataAppDomain_GetName(This, bufLen, nameLen, name) \
    ((This)->lpVtbl->GetName(This, bufLen, nameLen, name))

#define IXCLRDataAppDomain_GetUniqueID(This, id) \
    ((This)->lpVtbl->GetUniqueID(This, id))

typedef struct IXCLRDataTaskVtbl
{
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        __in IXCLRDataTask *This,
        __in REFIID riid,
        __deref_out void **ppvObject
        );

    ULONG (STDMETHODCALLTYPE *AddRef)(
        __in IXCLRDataTask *This
        );

    ULONG (STDMETHODCALLTYPE *Release)(
        __in IXCLRDataTask *This
        );

    HRESULT (STDMETHODCALLTYPE *GetProcess)(
        __in IXCLRDataTask *This,
        __out IXCLRDataProcess **process
        );

    HRESULT (STDMETHODCALLTYPE *GetCurrentAppDomain)(
        __in IXCLRDataTask *This,
        __out IXCLRDataAppDomain **appDomain
        );

    HRESULT (STDMETHODCALLTYPE *GetUniqueID)(
        __in IXCLRDataTask *This,
        __out ULONG64 *id
        );

    HRESULT (STDMETHODCALLTYPE *GetFlags)(
        __in IXCLRDataTask *This,
        __out ULONG32 *flags
        );

    PVOID IsSameObject;
    PVOID GetManagedObject;
    PVOID GetDesiredExecutionState;
    PVOID SetDesiredExecutionState;

    HRESULT (STDMETHODCALLTYPE *CreateStackWalk)(
        __in IXCLRDataTask *This,
        __in ULONG32 flags,
        __out IXCLRDataStackWalk **stackWalk
        );

    HRESULT (STDMETHODCALLTYPE *GetOSThreadID)(
        __in IXCLRDataTask *This,
        __out ULONG32 *id
        );

    PVOID GetContext;
    PVOID SetContext;
    PVOID GetCurrentExceptionState;
    PVOID Request;

    HRESULT (STDMETHODCALLTYPE *GetName)(
        __in IXCLRDataTask *This,
        __in ULONG32 bufLen,
        __out ULONG32 *nameLen,
        __out WCHAR *name
        );

    PVOID GetLastExceptionState;
} IXCLRDataTaskVtbl;

typedef struct IXCLRDataTask
{
    struct IXCLRDataTaskVtbl *lpVtbl;
} IXCLRDataTask;

#define IXCLRDataTask_QueryInterface(This, riid, ppvObject) \
    ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))

#define IXCLRDataTask_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IXCLRDataTask_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IXCLRDataTask_GetProcess(This, process) \
    ((This)->lpVtbl->GetProcess(This, process))

#define IXCLRDataTask_GetCurrentAppDomain(This, appDomain) \
    ((This)->lpVtbl->GetCurrentAppDomain(This, appDomain))

#define IXCLRDataTask_GetUniqueID(This, id) \
    ((This)->lpVtbl->GetUniqueID(This, id))

#define IXCLRDataTask_GetFlags(This, flags) \
    ((This)->lpVtbl->GetFlags(This, flags))

#define IXCLRDataTask_CreateStackWalk(This, flags, stackWalk) \
    ((This)->lpVtbl->CreateStackWalk(This, flags, stackWalk))

#define IXCLRDataTask_GetOSThreadID(This, id) \
    ((This)->lpVtbl->GetOSThreadID(This, id))

#define IXCLRDataTask_GetName(This, bufLen, nameLen, name) \
    ((This)->lpVtbl->GetName(This, bufLen, nameLen, name))

typedef enum
{
    CLRDATA_SIMPFRAME_UNRECOGNIZED = 0x1,
    CLRDATA_SIMPFRAME_MANAGED_METHOD = 0x2,
    CLRDATA_SIMPFRAME_RUNTIME_MANAGED_CODE = 0x4,
    CLRDATA_SIMPFRAME_RUNTIME_UNMANAGED_CODE = 0x8
} CLRDataSimpleFrameType;

typedef enum
{
    CLRDATA_DETFRAME_UNRECOGNIZED,
    CLRDATA_DETFRAME_UNKNOWN_STUB,
    CLRDATA_DETFRAME_CLASS_INIT,
    CLRDATA_DETFRAME_EXCEPTION_FILTER,
    CLRDATA_DETFRAME_SECURITY,
    CLRDATA_DETFRAME_CONTEXT_POLICY,
    CLRDATA_DETFRAME_INTERCEPTION, 
    CLRDATA_DETFRAME_PROCESS_START,
    CLRDATA_DETFRAME_THREAD_START,
    CLRDATA_DETFRAME_TRANSITION_TO_MANAGED,
    CLRDATA_DETFRAME_TRANSITION_TO_UNMANAGED,
    CLRDATA_DETFRAME_COM_INTEROP_STUB,
    CLRDATA_DETFRAME_DEBUGGER_EVAL,
    CLRDATA_DETFRAME_CONTEXT_SWITCH,
    CLRDATA_DETFRAME_FUNC_EVAL,
    CLRDATA_DETFRAME_FINALLY
} CLRDataDetailedFrameType;

typedef enum
{
    CLRDATA_STACK_SET_UNWIND_CONTEXT  = 0x00000000,
    CLRDATA_STACK_SET_CURRENT_CONTEXT = 0x00000001
} CLRDataStackSetContextFlag;

typedef struct IXCLRDataStackWalkVtbl
{
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        __in IXCLRDataStackWalk *This,
        __in REFIID riid,
        __deref_out void **ppvObject
        );

    ULONG (STDMETHODCALLTYPE *AddRef)(
        __in IXCLRDataStackWalk *This
        );

    ULONG (STDMETHODCALLTYPE *Release)(
        __in IXCLRDataStackWalk *This
        );

    HRESULT (STDMETHODCALLTYPE *GetContext)(
        __in IXCLRDataStackWalk *This,
        __in ULONG32 contextFlags,
        __in ULONG32 contextBufSize,
        __out ULONG32 *contextSize,
        __out BYTE *contextBuf
        );

    PVOID SetContext;

    HRESULT (STDMETHODCALLTYPE *Next)(
        __in IXCLRDataStackWalk *This
        );

    HRESULT (STDMETHODCALLTYPE *GetStackSizeSkipped)(
        __in IXCLRDataStackWalk *This,
        __out ULONG64 *stackSizeSkipped
        );

    HRESULT (STDMETHODCALLTYPE *GetFrameType)(
        __in IXCLRDataStackWalk *This,
        __out CLRDataSimpleFrameType *simpleType,
        __out CLRDataDetailedFrameType *detailedType
        );

    HRESULT (STDMETHODCALLTYPE *GetFrame)(
        __in IXCLRDataStackWalk *This,
        __out PVOID *frame
        );

    HRESULT (STDMETHODCALLTYPE *Request)(
        __in IXCLRDataStackWalk *This,
        __in ULONG32 reqCode,
        __in ULONG32 inBufferSize,
        __in BYTE *inBuffer,
        __in ULONG32 outBufferSize,
        __out BYTE *outBuffer
        );

    HRESULT (STDMETHODCALLTYPE *SetContext2)(
        __in IXCLRDataStackWalk *This,
        __in ULONG32 flags,
        __in ULONG32 contextSize,
        __in BYTE *context
        );
} IXCLRDataStackWalkVtbl;

typedef struct IXCLRDataStackWalk
{
    struct IXCLRDataStackWalkVtbl *lpVtbl;
} IXCLRDataStackWalk;

#define IXCLRDataStackWalk_QueryInterface(This, riid, ppvObject) \
    ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))

#define IXCLRDataStackWalk_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IXCLRDataStackWalk_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IXCLRDataStackWalk_GetContext(This, contextFlags, contextBufSize, contextSize, contextBuf) \
    ((This)->lpVtbl->GetContext(This, contextFlags, contextBufSize, contextSize, contextBuf))

#define IXCLRDataStackWalk_Next(This) \
    ((This)->lpVtbl->Next(This))

#define IXCLRDataStackWalk_GetStackSizeSkipped(This, stackSizeSkipped) \
    ((This)->lpVtbl->GetStackSizeSkipped(This, stackSizeSkipped))

#define IXCLRDataStackWalk_GetFrameType(This, simpleType, detailedType) \
    ((This)->lpVtbl->GetFrameType(This, simpleType, detailedType))

#define IXCLRDataStackWalk_GetFrame(This, frame) \
    ((This)->lpVtbl->GetFrame(This, frame))

#define IXCLRDataStackWalk_Request(This, reqCode, inBufferSize, inBuffer, outBufferSize, outBuffer) \
    ((This)->lpVtbl->SetContext2(This, reqCode, inBufferSize, inBuffer, outBufferSize, outBuffer))

#define IXCLRDataStackWalk_SetContext2(This, flags, contextSize, context) \
    ((This)->lpVtbl->SetContext2(This, flags, contextSize, context))

typedef struct IXCLRDataFrameVtbl
{
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        __in IXCLRDataFrame *This,
        __in REFIID riid,
        __deref_out void **ppvObject
        );

    ULONG (STDMETHODCALLTYPE *AddRef)(
        __in IXCLRDataFrame *This
        );

    ULONG (STDMETHODCALLTYPE *Release)(
        __in IXCLRDataFrame *This
        );

    HRESULT (STDMETHODCALLTYPE *GetFrameType)(
        __in IXCLRDataFrame *This,
        __out CLRDataSimpleFrameType *simpleType,
        __out CLRDataDetailedFrameType *detailedType
        );

    HRESULT (STDMETHODCALLTYPE *GetContext)(
        __in IXCLRDataFrame *This,
        __in ULONG32 contextFlags,
        __in ULONG32 contextBufSize,
        __out ULONG32 *contextSize,
        __out BYTE *contextBuf
        );

    PVOID GetAppDomain;
    PVOID GetNumArguments;
    PVOID GetArgumentByIndex;
    PVOID GetNumLocalVariables;
    PVOID GetLocalVariableByIndex;

    HRESULT (STDMETHODCALLTYPE *GetCodeName)(
        __in IXCLRDataFrame *This,
        __in ULONG32 *flags,
        __in ULONG32 *bufLen,
        __out ULONG32 *nameLen,
        __out WCHAR *nameBuf
        );
} IXCLRDataFrameVtbl;

typedef struct IXCLRDataFrame
{
    IXCLRDataFrameVtbl *lpVtbl;
} IXCLRDataFrame;

#define IXCLRDataFrame_QueryInterface(This, riid, ppvObject) \
    ((This)->lpVtbl->QueryInterface(This, riid, ppvObject))

#define IXCLRDataFrame_AddRef(This) \
    ((This)->lpVtbl->AddRef(This))

#define IXCLRDataFrame_Release(This) \
    ((This)->lpVtbl->Release(This))

#define IXCLRDataFrame_GetFrameType(This, simpleType, detailedType) \
    ((This)->lpVtbl->GetFrameType(This, simpleType, detailedType))

#define IXCLRDataFrame_GetContext(This, contextFlags, contextBufSize, contextSize, contextBuf) \
    ((This)->lpVtbl->GetContext(This, contextFlags, contextBufSize, contextSize, contextBuf))

#define IXCLRDataFrame_GetCodeName(This, flags, bufLen, nameLen, nameBuf) \
    ((This)->lpVtbl->GetCodeName(This, flags, bufLen, nameLen, nameBuf))

// DnCLRDataTarget

typedef struct
{
    ICLRDataTargetVtbl *VTable;

    ULONG RefCount;

    HANDLE ProcessId;
    HANDLE ProcessHandle;
    BOOLEAN IsWow64;
} DnCLRDataTarget;

ICLRDataTarget *DnCLRDataTarget_Create(
    __in HANDLE ProcessId
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_QueryInterface(
    __in ICLRDataTarget *This,
    __in REFIID Riid,
    __out PVOID *Object
    );

ULONG STDMETHODCALLTYPE DnCLRDataTarget_AddRef(
    __in ICLRDataTarget *This
    );

ULONG STDMETHODCALLTYPE DnCLRDataTarget_Release(
    __in ICLRDataTarget *This
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_GetMachineType(
    __in ICLRDataTarget *This,
    __out ULONG32 *machineType
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_GetPointerSize(
    __in ICLRDataTarget *This,
    __out ULONG32 *pointerSize
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_GetImageBase(
    __in ICLRDataTarget *This,
    __in LPCWSTR imagePath,
    __out CLRDATA_ADDRESS *baseAddress
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_ReadVirtual(
    __in ICLRDataTarget *This,
    __in CLRDATA_ADDRESS address,
    __out BYTE *buffer,
    __in ULONG32 bytesRequested,
    __out ULONG32 *bytesRead
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_WriteVirtual(
    __in ICLRDataTarget *This,
    __in CLRDATA_ADDRESS address,
    __in BYTE *buffer,
    __in ULONG32 bytesRequested,
    __out ULONG32 *bytesWritten
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_GetTLSValue(
    __in ICLRDataTarget *This,
    __in ULONG32 threadID,
    __in ULONG32 index,
    __out CLRDATA_ADDRESS *value
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_SetTLSValue(
    __in ICLRDataTarget *This,
    __in ULONG32 threadID,
    __in ULONG32 index,
    __in CLRDATA_ADDRESS value
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_GetCurrentThreadID(
    __in ICLRDataTarget *This,
    __out ULONG32 *threadID
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_GetThreadContext(
    __in ICLRDataTarget *This,
    __in ULONG32 threadID,
    __in ULONG32 contextFlags,
    __in ULONG32 contextSize,
    __out BYTE *context
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_SetThreadContext(
    __in ICLRDataTarget *This,
    __in ULONG32 threadID,
    __in ULONG32 contextSize,
    __in BYTE *context
    );

HRESULT STDMETHODCALLTYPE DnCLRDataTarget_Request(
    __in ICLRDataTarget *This,
    __in ULONG32 reqCode,
    __in ULONG32 inBufferSize,
    __in BYTE *inBuffer,
    __in ULONG32 outBufferSize,
    __out BYTE *outBuffer
    );

typedef struct _PHP_GET_IMAGE_BASE_CONTEXT
{
    UNICODE_STRING ImagePath;
    PVOID BaseAddress;
} PHP_GET_IMAGE_BASE_CONTEXT, *PPHP_GET_IMAGE_BASE_CONTEXT;

#endif
