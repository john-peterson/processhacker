#ifndef PH_H
#define PH_H

#include <phbase.h>
#include <stdarg.h>
#include <dltmgr.h>

// native

/** The PID of the idle process. */
#define SYSTEM_IDLE_PROCESS_ID ((HANDLE)0)
/** The PID of the system process. */
#define SYSTEM_PROCESS_ID ((HANDLE)4)

/** Specifies a PEB string. */
typedef enum _PH_PEB_OFFSET
{
    PhpoCurrentDirectory,
    PhpoDllPath,
    PhpoImagePathName,
    PhpoCommandLine,
    PhpoWindowTitle,
    PhpoDesktopName,
    PhpoShellInfo,
    PhpoRuntimeData
} PH_PEB_OFFSET;

/** Specifies a token integrity level. */
typedef enum _PH_INTEGRITY
{
    PiUntrusted = 0,
    PiLow = 1,
    PiMedium = 2,
    PiHigh = 3,
    PiSystem = 4,
    PiInstaller = 5
} PH_INTEGRITY, *PPH_INTEGRITY;

typedef struct _PH_PROCESS_WS_COUNTERS
{
    ULONG NumberOfPages;
    ULONG NumberOfPrivatePages;
    ULONG NumberOfSharedPages;
    ULONG NumberOfShareablePages;
} PH_PROCESS_WS_COUNTERS, *PPH_PROCESS_WS_COUNTERS;

/** Contains information about an environment variable. */
typedef struct _PH_ENVIRONMENT_VARIABLE
{
    /** A string containing the variable name. */
    PPH_STRING Name;
    /** A string containing the variable value. */
    PPH_STRING Value;
} PH_ENVIRONMENT_VARIABLE, *PPH_ENVIRONMENT_VARIABLE;

/** Contains information about a thread stack frame. */
typedef struct _PH_THREAD_STACK_FRAME
{
    PVOID PcAddress;
    PVOID ReturnAddress;
    PVOID FrameAddress;
    PVOID StackAddress;
    PVOID BStoreAddress;
    PVOID Params[4];
} PH_THREAD_STACK_FRAME, *PPH_THREAD_STACK_FRAME;

NTSTATUS PhOpenProcess(
    __out PHANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in HANDLE ProcessId
    );

NTSTATUS PhOpenThread(
    __out PHANDLE ThreadHandle,
    __in ACCESS_MASK DesiredAccess,
    __in HANDLE ThreadId
    );

NTSTATUS PhOpenProcessToken(
    __out PHANDLE TokenHandle,
    __in ACCESS_MASK DesiredAccess,
    __in HANDLE ProcessHandle
    );

NTSTATUS PhOpenThreadToken(
    __out PHANDLE TokenHandle,
    __in ACCESS_MASK DesiredAccess,
    __in HANDLE ThreadHandle,
    __in BOOLEAN OpenAsSelf
    );

NTSTATUS PhGetObjectSecurity(
    __in HANDLE Handle,
    __in SECURITY_INFORMATION SecurityInformation,
    __out PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

NTSTATUS PhSetObjectSecurity(
    __in HANDLE Handle,
    __in SECURITY_INFORMATION SecurityInformation,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS PhTerminateProcess(
    __in HANDLE ProcessHandle,
    __in NTSTATUS ExitStatus
    );

NTSTATUS PhSuspendProcess(
    __in HANDLE ProcessHandle
    );

NTSTATUS PhResumeProcess(
    __in HANDLE ProcessHandle
    );

NTSTATUS PhTerminateThread(
    __in HANDLE ThreadHandle,
    __in NTSTATUS ExitStatus
    );

NTSTATUS PhSuspendThread(
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    );

NTSTATUS PhResumeThread(
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    );

NTSTATUS PhGetThreadContext(
    __in HANDLE ThreadHandle,
    __inout PCONTEXT Context
    );

NTSTATUS PhSetThreadContext(
    __in HANDLE ThreadHandle,
    __in PCONTEXT Context
    );

NTSTATUS PhReadVirtualMemory(
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __out_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesRead
    );

NTSTATUS PhWriteVirtualMemory(
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __in_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesWritten
    );

NTSTATUS PhGetProcessBasicInformation(
    __in HANDLE ProcessHandle,
    __out PPROCESS_BASIC_INFORMATION BasicInformation
    );

NTSTATUS PhGetProcessImageFileName(
    __in HANDLE ProcessHandle,
    __out PPH_STRING *FileName
    );

/**
 * Gets a process' command line.
 *
 * \param ProcessHandle A handle to a process. The handle must 
 * have PROCESS_QUERY_LIMITED_INFORMATION and PROCESS_VM_READ 
 * access.
 * \param String A variable which receives a pointer to a 
 * string containing the command line. You must free the string 
 * using PhDereferenceObject() when you no longer need it.
 */
#define PhGetProcessCommandLine(ProcessHandle, String) \
    PhGetProcessPebString(ProcessHandle, PhpoCommandLine, String)

NTSTATUS PhGetProcessPebString(
    __in HANDLE ProcessHandle,
    __in PH_PEB_OFFSET Offset,
    __out PPH_STRING *String
    );

NTSTATUS PhGetProcessExecuteFlags(
    __in HANDLE ProcessHandle,
    __out PULONG ExecuteFlags
    );

NTSTATUS PhGetProcessIsWow64(
    __in HANDLE ProcessHandle,
    __out PBOOLEAN IsWow64
    );

NTSTATUS PhGetProcessIsBeingDebugged(
    __in HANDLE ProcessHandle,
    __out PBOOLEAN IsBeingDebugged
    );

NTSTATUS PhGetProcessDebugObject(
    __in HANDLE ProcessHandle,
    __out PHANDLE DebugObjectHandle
    );

NTSTATUS PhGetProcessIoPriority(
    __in HANDLE ProcessHandle,
    __out PULONG IoPriority
    );

NTSTATUS PhGetProcessPagePriority(
    __in HANDLE ProcessHandle,
    __out PULONG PagePriority
    );

NTSTATUS PhGetProcessIsPosix(
    __in HANDLE ProcessHandle,
    __out PBOOLEAN IsPosix
    );

NTSTATUS PhGetProcessCycleTime(
    __in HANDLE ProcessHandle,
    __out PULONG64 CycleTime
    );

NTSTATUS PhGetProcessPosixCommandLine(
    __in HANDLE ProcessHandle,
    __out PPH_STRING *CommandLine
    );

NTSTATUS PhGetProcessEnvironmentVariables(
    __in HANDLE ProcessHandle,
    __out PPH_ENVIRONMENT_VARIABLE *Variables,
    __out PULONG NumberOfVariables
    );

VOID PhFreeProcessEnvironmentVariables(
    __in PPH_ENVIRONMENT_VARIABLE Variables,
    __in ULONG NumberOfVariables
    );

NTSTATUS PhGetProcessMappedFileName(
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __out PPH_STRING *FileName
    );

NTSTATUS PhGetProcessWorkingSetInformation(
    __in HANDLE ProcessHandle,
    __out PMEMORY_WORKING_SET_INFORMATION *WorkingSetInformation
    );

NTSTATUS PhGetProcessWsCounters(
    __in HANDLE ProcessHandle,
    __out PPH_PROCESS_WS_COUNTERS WsCounters
    );

NTSTATUS PhSetProcessIoPriority(
    __in HANDLE ProcessHandle,
    __in ULONG IoPriority
    );

NTSTATUS PhSetProcessExecuteFlags(
    __in HANDLE ProcessHandle,
    __in ULONG ExecuteFlags
    );

NTSTATUS PhInjectDllProcess(
    __in HANDLE ProcessHandle,
    __in PWSTR FileName,
    __in ULONG Timeout
    );

NTSTATUS PhUnloadDllProcess(
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __in ULONG Timeout
    );

NTSTATUS PhGetThreadBasicInformation(
    __in HANDLE ThreadHandle,
    __out PTHREAD_BASIC_INFORMATION BasicInformation
    );

NTSTATUS PhGetThreadIoPriority(
    __in HANDLE ThreadHandle,
    __out PULONG IoPriority
    );

NTSTATUS PhGetThreadPagePriority(
    __in HANDLE ThreadHandle,
    __out PULONG PagePriority
    );

NTSTATUS PhGetThreadCycleTime(
    __in HANDLE ThreadHandle,
    __out PULONG64 CycleTime
    );

NTSTATUS PhSetThreadIoPriority(
    __in HANDLE ThreadHandle,
    __in ULONG IoPriority
    );

#define PH_WALK_I386_STACK 0x1
#define PH_WALK_AMD64_STACK 0x2
#define PH_WALK_KERNEL_STACK 0x10

/**
 * A callback function passed to PhWalkThreadStack() 
 * and called for each stack frame.
 *
 * \param StackFrame A structure providing information about 
 * the stack frame.
 * \param Context A user-defined value passed to 
 * PhWalkThreadStack().
 *
 * \return TRUE to continue the stack walk, FALSE to 
 * stop.
 */
typedef BOOLEAN (NTAPI *PPH_WALK_THREAD_STACK_CALLBACK)(
    __in PPH_THREAD_STACK_FRAME StackFrame,
    __in PVOID Context
    );

NTSTATUS PhWalkThreadStack(
    __in HANDLE ThreadHandle,
    __in_opt HANDLE ProcessHandle,
    __in ULONG Flags,
    __in PPH_WALK_THREAD_STACK_CALLBACK Callback,
    __in PVOID Context
    );

NTSTATUS PhGetTokenUser(
    __in HANDLE TokenHandle,
    __out PTOKEN_USER *User
    );

NTSTATUS PhGetTokenSessionId(
    __in HANDLE TokenHandle,
    __out PULONG SessionId
    );

NTSTATUS PhGetTokenElevationType(
    __in HANDLE TokenHandle,
    __out PTOKEN_ELEVATION_TYPE ElevationType
    );

NTSTATUS PhGetTokenIsElevated(
    __in HANDLE TokenHandle,
    __out PBOOLEAN Elevated
    );

NTSTATUS PhGetTokenStatistics(
    __in HANDLE TokenHandle,
    __out PTOKEN_STATISTICS Statistics
    );

NTSTATUS PhGetTokenGroups(
    __in HANDLE TokenHandle,
    __out PTOKEN_GROUPS *Groups
    );

NTSTATUS PhGetTokenPrivileges(
    __in HANDLE TokenHandle,
    __out PTOKEN_PRIVILEGES *Privileges
    );

NTSTATUS PhGetTokenIsVirtualizationAllowed(
    __in HANDLE TokenHandle,
    __out PBOOLEAN IsVirtualizationAllowed
    );

NTSTATUS PhGetTokenIsVirtualizationEnabled(
    __in HANDLE TokenHandle,
    __out PBOOLEAN IsVirtualizationEnabled
    );

NTSTATUS PhSetTokenSessionId(
    __in HANDLE TokenHandle,
    __in ULONG SessionId
    );

BOOLEAN PhSetTokenPrivilege(
    __in HANDLE TokenHandle,
    __in_opt PWSTR PrivilegeName,
    __in_opt PLUID PrivilegeLuid,
    __in ULONG Attributes
    );

NTSTATUS PhSetTokenIsVirtualizationEnabled(
    __in HANDLE TokenHandle,
    __in BOOLEAN IsVirtualizationEnabled
    );

NTSTATUS PhGetTokenIntegrityLevel(
    __in HANDLE TokenHandle,
    __out_opt PPH_INTEGRITY IntegrityLevel, 
    __out_opt PPH_STRING *IntegrityString
    );

NTSTATUS PhGetTransactionManagerBasicInformation(
    __in HANDLE TransactionManagerHandle,
    __out PTRANSACTIONMANAGER_BASIC_INFORMATION BasicInformation
    );

NTSTATUS PhGetTransactionManagerLogFileName(
    __in HANDLE TransactionManagerHandle,
    __out PPH_STRING *LogFileName
    );

NTSTATUS PhGetTransactionBasicInformation(
    __in HANDLE TransactionHandle,
    __out PTRANSACTION_BASIC_INFORMATION BasicInformation
    );

NTSTATUS PhGetTransactionPropertiesInformation(
    __in HANDLE TransactionHandle,
    __out_opt PLARGE_INTEGER Timeout,
    __out_opt TRANSACTION_OUTCOME *Outcome,
    __out_opt PPH_STRING *Description
    );

NTSTATUS PhGetResourceManagerBasicInformation(
    __in HANDLE ResourceManagerHandle,
    __out_opt PGUID Guid,
    __out_opt PPH_STRING *Description
    );

NTSTATUS PhGetEnlistmentBasicInformation(
    __in HANDLE EnlistmentHandle,
    __out PENLISTMENT_BASIC_INFORMATION BasicInformation
    );

NTSTATUS PhOpenDriverByBaseAddress(
    __out PHANDLE DriverHandle,
    __in PVOID BaseAddress
    );

NTSTATUS PhGetDriverServiceKeyName(
    __in HANDLE DriverHandle,
    __out PPH_STRING *ServiceKeyName
    );

NTSTATUS PhUnloadDriver(
    __in_opt PVOID BaseAddress,
    __in_opt PWSTR Name
    );

NTSTATUS PhDuplicateObject(
    __in HANDLE SourceProcessHandle,
    __in HANDLE SourceHandle,
    __in_opt HANDLE TargetProcessHandle,
    __out_opt PHANDLE TargetHandle,
    __in ACCESS_MASK DesiredAccess,
    __in ULONG HandleAttributes,
    __in ULONG Options
    );

#define PH_ENUM_PROCESS_MODULES_ITERS 0x800

/**
 * A callback function passed to PhEnumProcessModules() 
 * and called for each process module.
 *
 * \param Module A structure providing information about 
 * the module.
 * \param Context A user-defined value passed to 
 * PhEnumProcessModules().
 *
 * \return TRUE to continue the enumeration, FALSE to 
 * stop.
 */
typedef BOOLEAN (NTAPI *PPH_ENUM_PROCESS_MODULES_CALLBACK)(
    __in PLDR_DATA_TABLE_ENTRY Module,
    __in PVOID Context
    );

NTSTATUS PhEnumProcessModules(
    __in HANDLE ProcessHandle,
    __in PPH_ENUM_PROCESS_MODULES_CALLBACK Callback,
    __in PVOID Context
    );

NTSTATUS PhSetProcessModuleLoadCount(
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __in USHORT LoadCount
    );

PPH_STRING PhGetKernelFileName();

NTSTATUS PhEnumKernelModules(
    __out PRTL_PROCESS_MODULES *Modules
    );

/**
 * Gets a pointer to the first process information 
 * structure in a buffer returned by PhEnumProcesses().
 *
 * \param Processes A pointer to a buffer returned 
 * by PhEnumProcesses().
 */
#define PH_FIRST_PROCESS(Processes) ((PSYSTEM_PROCESS_INFORMATION)(Processes))

/**
 * Gets a pointer to the process information structure 
 * after a given structure.
 *
 * \param Process A pointer to a process information 
 * structure.
 *
 * \return A pointer to the next process information 
 * structure, or NULL if there are no more.
 */
#define PH_NEXT_PROCESS(Process) ( \
    ((PSYSTEM_PROCESS_INFORMATION)(Process))->NextEntryOffset ? \
    (PSYSTEM_PROCESS_INFORMATION)((PCHAR)(Process) + \
    ((PSYSTEM_PROCESS_INFORMATION)(Process))->NextEntryOffset) : \
    NULL \
    )

NTSTATUS PhEnumProcesses(
    __out PPVOID Processes
    );

PSYSTEM_PROCESS_INFORMATION PhFindProcessInformation(
    __in PVOID Processes,
    __in HANDLE ProcessId
    );

NTSTATUS PhEnumHandles(
    __out PSYSTEM_HANDLE_INFORMATION *Handles
    );

/**
 * A callback function passed to PhEnumDirectoryObjects() 
 * and called for each directory object.
 *
 * \param Name The name of the object.
 * \param TypeName The name of the object's type.
 * \param Context A user-defined value passed to 
 * PhEnumDirectoryObjects().
 *
 * \return TRUE to continue the enumeration, FALSE to 
 * stop.
 */
typedef BOOLEAN (NTAPI *PPH_ENUM_DIRECTORY_OBJECTS)(
    __in PPH_STRING Name,
    __in PPH_STRING TypeName,
    __in PVOID Context
    );

NTSTATUS PhEnumDirectoryObjects(
    __in HANDLE DirectoryHandle,
    __in PPH_ENUM_DIRECTORY_OBJECTS Callback,
    __in PVOID Context
    );

VOID PhInitializeDosDeviceNames();

VOID PhRefreshDosDeviceNames();

PPH_STRING PhResolveDevicePrefix(
    __in PPH_STRING Name
    );

PPH_STRING PhGetFileName(
    __in PPH_STRING FileName
    );

typedef struct _PH_MODULE_INFO
{
    PVOID BaseAddress;
    ULONG Size;
    PVOID EntryPoint;
    ULONG Flags;
    PPH_STRING Name;
    PPH_STRING FileName;
} PH_MODULE_INFO, *PPH_MODULE_INFO;

/**
 * A callback function passed to PhEnumGenericModules() 
 * and called for each process module.
 *
 * \param Module A structure providing information about 
 * the module.
 * \param Context A user-defined value passed to 
 * PhEnumGenericModules().
 *
 * \return TRUE to continue the enumeration, FALSE to 
 * stop.
 */
typedef BOOLEAN (NTAPI *PPH_ENUM_GENERIC_MODULES_CALLBACK)(
    __in PPH_MODULE_INFO Module,
    __in PVOID Context
    );

#define PH_ENUM_GENERIC_MAPPED_FILES 0x1

NTSTATUS PhEnumGenericModules(
    __in HANDLE ProcessId,
    __in_opt HANDLE ProcessHandle,
    __in ULONG Flags,
    __in PPH_ENUM_GENERIC_MODULES_CALLBACK Callback,
    __in PVOID Context
    );

// lsa

NTSTATUS PhOpenLsaPolicy(
    __out PLSA_HANDLE PolicyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt PUNICODE_STRING SystemName
    );

BOOLEAN PhLookupPrivilegeName(
    __in PLUID PrivilegeValue,
    __out PPH_STRING *PrivilegeName
    );

BOOLEAN PhLookupPrivilegeDisplayName(
    __in PWSTR PrivilegeName,
    __out PPH_STRING *PrivilegeDisplayName
    );

BOOLEAN PhLookupPrivilegeValue(
    __in PWSTR PrivilegeName,
    __out PLUID PrivilegeValue
    );

NTSTATUS PhLookupSid(
    __in PSID Sid,
    __out_opt PPH_STRING *Name,
    __out_opt PPH_STRING *DomainName,
    __out_opt PSID_NAME_USE NameUse
    );

PPH_STRING PhGetSidFullName(
    __in PSID Sid,
    __in BOOLEAN IncludeDomain,
    __out_opt PSID_NAME_USE NameUse
    );

PPH_STRING PhSidToStringSid(
    __in PSID Sid
    );

typedef BOOLEAN (NTAPI *PPH_ENUM_ACCOUNTS_CALLBACK)(
    __in PSID Sid,
    __in PVOID Context
    );

NTSTATUS PhEnumAccounts(
    __in LSA_HANDLE PolicyHandle,
    __in PPH_ENUM_ACCOUNTS_CALLBACK Callback,
    __in PVOID Context
    );

// hndlinfo

VOID PhHandleInfoInitialization();

PPH_STRING PhFormatNativeKeyName(
    __in PPH_STRING Name
    );

PPH_STRING PhGetClientIdName(
    __in PCLIENT_ID ClientId
    );

NTSTATUS PhGetHandleInformation(
    __in HANDLE ProcessHandle,
    __in HANDLE Handle,
    __in ULONG ObjectTypeNumber,
    __out_opt POBJECT_BASIC_INFORMATION BasicInformation,
    __out_opt PPH_STRING *TypeName,
    __out_opt PPH_STRING *ObjectName,
    __out_opt PPH_STRING *BestObjectName
    );

NTSTATUS PhQueryObjectNameHack(
    __in HANDLE Handle,
    __out_bcount(ObjectNameInformationLength) POBJECT_NAME_INFORMATION ObjectNameInformation,
    __in ULONG ObjectNameInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSTATUS PhQueryObjectSecurityHack(
    __in HANDLE Handle,
    __in SECURITY_INFORMATION SecurityInformation,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __out_opt PULONG ReturnLength
    );

NTSTATUS PhSetObjectSecurityHack(
    __in HANDLE Handle,
    __in SECURITY_INFORMATION SecurityInformation,
    __in PVOID Buffer
    );

// verify

typedef enum _VERIFY_RESULT
{
    VrUnknown = 0,
    VrNoSignature,
    VrTrusted,
    VrExpired,
    VrRevoked,
    VrDistrust,
    VrSecuritySettings
} VERIFY_RESULT, *PVERIFY_RESULT;

VOID PhVerifyInitialization();

VERIFY_RESULT PhVerifyFile(
    __in PWSTR FileName,
    __out_opt PPH_STRING *SignerName
    );

// provider

#ifndef PROVIDER_PRIVATE
extern LIST_ENTRY PhDbgProviderListHead;
extern PH_FAST_LOCK PhDbgProviderListLock;
#endif

typedef enum _PH_PROVIDER_THREAD_STATE
{
    ProviderThreadRunning,
    ProviderThreadStopped,
    ProviderThreadStopping
} PH_PROVIDER_THREAD_STATE;

typedef VOID (NTAPI *PPH_PROVIDER_FUNCTION)(
    __in PVOID Object
    );

typedef struct _PH_PROVIDER_REGISTRATION
{
    LIST_ENTRY ListEntry;
    PPH_PROVIDER_FUNCTION Function;
    PVOID Object;
    BOOLEAN Enabled;
    BOOLEAN Unregistering;
    BOOLEAN Boosting;
} PH_PROVIDER_REGISTRATION, *PPH_PROVIDER_REGISTRATION;

typedef struct _PH_PROVIDER_THREAD
{
#ifdef DEBUG
    LIST_ENTRY DbgListEntry;
#endif
    HANDLE ThreadHandle;
    HANDLE TimerHandle;
    ULONG Interval;
    PH_PROVIDER_THREAD_STATE State;

    PH_QUEUED_LOCK Lock;
    LIST_ENTRY ListHead;
    ULONG BoostCount;
} PH_PROVIDER_THREAD, *PPH_PROVIDER_THREAD;

VOID PhInitializeProviderThread(
    __out PPH_PROVIDER_THREAD ProviderThread,
    __in ULONG Interval
    );

VOID PhDeleteProviderThread(
    __inout PPH_PROVIDER_THREAD ProviderThread
    );

VOID PhStartProviderThread(
    __inout PPH_PROVIDER_THREAD ProviderThread
    );

VOID PhStopProviderThread(
    __inout PPH_PROVIDER_THREAD ProviderThread
    );

VOID PhSetProviderThreadInterval(
    __inout PPH_PROVIDER_THREAD ProviderThread,
    __in ULONG Interval
    );

VOID PhBoostProvider(
    __inout PPH_PROVIDER_THREAD ProviderThread,
    __inout PPH_PROVIDER_REGISTRATION Registration
    );

VOID PhSetProviderEnabled(
    __in PPH_PROVIDER_REGISTRATION Registration,
    __in BOOLEAN Enabled
    );

VOID PhRegisterProvider(
    __inout PPH_PROVIDER_THREAD ProviderThread,
    __in PPH_PROVIDER_FUNCTION Function,
    __in PVOID Object,
    __out PPH_PROVIDER_REGISTRATION Registration
    );

VOID PhUnregisterProvider(
    __inout PPH_PROVIDER_THREAD ProviderThread,
    __inout PPH_PROVIDER_REGISTRATION Registration
    );

// symprv

#ifndef SYMPRV_PRIVATE
extern PPH_OBJECT_TYPE PhSymbolProviderType;
#endif

#define PH_MAX_SYMBOL_NAME_LEN 128

typedef struct _PH_SYMBOL_PROVIDER
{
    PPH_LIST ModulesList;
    PH_QUEUED_LOCK ModulesListLock;
    HANDLE ProcessHandle;
    BOOLEAN IsRealHandle;
} PH_SYMBOL_PROVIDER, *PPH_SYMBOL_PROVIDER;

typedef enum _PH_SYMBOL_RESOLVE_LEVEL
{
    PhsrlFunction,
    PhsrlModule,
    PhsrlAddress,
    PhsrlInvalid
} PH_SYMBOL_RESOLVE_LEVEL, *PPH_SYMBOL_RESOLVE_LEVEL;

BOOLEAN PhSymbolProviderInitialization();

VOID PhSymbolProviderDynamicImport();

PPH_SYMBOL_PROVIDER PhCreateSymbolProvider(
    __in_opt HANDLE ProcessId
    );

ULONG64 PhGetModuleFromAddress(
    __in PPH_SYMBOL_PROVIDER SymbolProvider,
    __in ULONG64 Address,
    __out_opt PPH_STRING *FileName
    );

PPH_STRING PhGetSymbolFromAddress(
    __in PPH_SYMBOL_PROVIDER SymbolProvider,
    __in ULONG64 Address,
    __out_opt PPH_SYMBOL_RESOLVE_LEVEL ResolveLevel,
    __out_opt PPH_STRING *FileName,
    __out_opt PPH_STRING *SymbolName,
    __out_opt PULONG64 Displacement
    );

BOOLEAN PhSymbolProviderLoadModule(
    __in PPH_SYMBOL_PROVIDER SymbolProvider,
    __in PWSTR FileName,
    __in ULONG64 BaseAddress,
    __in ULONG Size
    );

VOID PhSymbolProviderSetSearchPath(
    __in PPH_SYMBOL_PROVIDER SymbolProvider,
    __in PWSTR SearchPath
    );

// procprv

#ifndef PROCPRV_PRIVATE
extern PPH_OBJECT_TYPE PhProcessItemType;

extern PH_CALLBACK PhProcessAddedEvent;
extern PH_CALLBACK PhProcessModifiedEvent;
extern PH_CALLBACK PhProcessRemovedEvent;
#endif

typedef struct _PH_IMAGE_VERSION_INFO
{
    PPH_STRING CompanyName;
    PPH_STRING FileDescription;
    PPH_STRING FileVersion;
    PPH_STRING ProductName;
} PH_IMAGE_VERSION_INFO, *PPH_IMAGE_VERSION_INFO;

#define DPCS_PROCESS_ID ((HANDLE)-2)
#define INTERRUPTS_PROCESS_ID ((HANDLE)-3)

#define PH_INTEGRITY_STR_LEN 10
#define PH_INTEGRITY_STR_LEN_1 (PH_INTEGRITY_STR_LEN + 1)

typedef struct _PH_PROCESS_ITEM
{
    ULONG RunId;
    HANDLE ProcessId;
    HANDLE ParentProcessId;
    PPH_STRING ProcessName;
    ULONG SessionId;

    PPH_STRING FileName;
    PPH_STRING CommandLine;

    HICON SmallIcon;
    HICON LargeIcon;
    PH_IMAGE_VERSION_INFO VersionInfo;

    LARGE_INTEGER CreateTime;

    PPH_STRING UserName;
    PH_INTEGRITY IntegrityLevel;

    PPH_STRING JobName;

    VERIFY_RESULT VerifyResult;
    PPH_STRING VerifySignerName;

    ULONG ImportFunctions;
    ULONG ImportModules;

    BOOLEAN HasParent : 1;
    BOOLEAN IsBeingDebugged : 1;
    BOOLEAN IsDotNet : 1;
    BOOLEAN IsElevated : 1;
    BOOLEAN IsInJob : 1;
    BOOLEAN IsInSignificantJob : 1;
    BOOLEAN IsPacked : 1;
    BOOLEAN IsPosix : 1;
    BOOLEAN IsWow64 : 1;

    BOOLEAN JustProcessed;
    PH_EVENT Stage1Event;

    PPH_POINTER_LIST ServiceList;
    PH_QUEUED_LOCK ServiceListLock;

    WCHAR ProcessIdString[PH_INT32_STR_LEN_1];
    WCHAR ParentProcessIdString[PH_INT32_STR_LEN_1];
    WCHAR SessionIdString[PH_INT32_STR_LEN_1];
    WCHAR IntegrityString[PH_INTEGRITY_STR_LEN_1];
    WCHAR CpuUsageString[PH_INT32_STR_LEN_1];

    FLOAT CpuUsage; // from 0 to 1

    PH_UINT64_DELTA CpuKernelDelta;
    PH_UINT64_DELTA CpuUserDelta;
    PH_UINT64_DELTA IoReadDelta;
    PH_UINT64_DELTA IoWriteDelta;
    PH_UINT64_DELTA IoOtherDelta;
} PH_PROCESS_ITEM, *PPH_PROCESS_ITEM;

BOOLEAN PhInitializeProcessProvider();

BOOLEAN PhInitializeImageVersionInfo(
    __out PPH_IMAGE_VERSION_INFO ImageVersionInfo,
    __in PWSTR FileName
    );

VOID PhDeleteImageVersionInfo(
    __inout PPH_IMAGE_VERSION_INFO ImageVersionInfo
    );

PPH_PROCESS_ITEM PhCreateProcessItem(
    __in HANDLE ProcessId
    );

PPH_PROCESS_ITEM PhReferenceProcessItem(
    __in HANDLE ProcessId
    );

VOID PhProcessProviderUpdate(
    __in PVOID Object
    );

// srvprv

#ifndef SRVPRV_PRIVATE
extern PPH_OBJECT_TYPE PhServiceItemType;

extern PH_CALLBACK PhServiceAddedEvent;
extern PH_CALLBACK PhServiceModifiedEvent;
extern PH_CALLBACK PhServiceRemovedEvent;
extern PH_CALLBACK PhServicesUpdatedEvent;
#endif

typedef struct _PH_SERVICE_ITEM
{
    ULONG RunId;
    PH_STRINGREF Key; // points to Name
    PPH_STRING Name;
    PPH_STRING DisplayName;

    // State
    ULONG Type;
    ULONG State;
    ULONG ControlsAccepted;
    ULONG ProcessId;

    // Config
    ULONG StartType;
    ULONG ErrorControl;

    BOOLEAN PendingProcess;
    BOOLEAN NeedsConfigUpdate;

    WCHAR ProcessIdString[PH_INT32_STR_LEN_1];
} PH_SERVICE_ITEM, *PPH_SERVICE_ITEM;

typedef struct _PH_SERVICE_MODIFIED_DATA
{
    PPH_SERVICE_ITEM Service;
    PH_SERVICE_ITEM OldService;
} PH_SERVICE_MODIFIED_DATA, *PPH_SERVICE_MODIFIED_DATA;

typedef enum _PH_SERVICE_CHANGE
{
    ServiceStarted,
    ServiceContinued,
    ServicePaused,
    ServiceStopped
} PH_SERVICE_CHANGE, *PPH_SERVICE_CHANGE;

BOOLEAN PhInitializeServiceProvider();

PPH_SERVICE_ITEM PhCreateServiceItem(
    __in_opt LPENUM_SERVICE_STATUS_PROCESS Information
    );

PPH_SERVICE_ITEM PhReferenceServiceItem(
    __in PWSTR Name
    );

VOID PhMarkNeedsConfigUpdateServiceItem(
    __in PPH_SERVICE_ITEM ServiceItem
    );

PVOID PhEnumServices(
    __in SC_HANDLE ScManagerHandle,
    __in_opt ULONG Type,
    __in_opt ULONG State,
    __out PULONG Count
    );

SC_HANDLE PhOpenService(
    __in PWSTR ServiceName,
    __in ACCESS_MASK DesiredAccess
    );

PVOID PhGetServiceConfig(
    __in SC_HANDLE ServiceHandle
    );

PPH_STRING PhGetServiceDescription(
    __in SC_HANDLE ServiceHandle
    );

BOOLEAN PhFindIntegerSiKeyValuePairs(
    __in PPH_KEY_VALUE_PAIR KeyValuePairs,
    __in ULONG SizeOfKeyValuePairs,
    __in PWSTR String,
    __out PULONG Integer
    );

BOOLEAN PhFindStringSiKeyValuePairs(
    __in PPH_KEY_VALUE_PAIR KeyValuePairs,
    __in ULONG SizeOfKeyValuePairs,
    __in ULONG Integer,
    __out PWSTR *String
    );

PWSTR PhGetServiceStateString(
    __in ULONG ServiceState
    );

PWSTR PhGetServiceTypeString(
    __in ULONG ServiceType
    );

ULONG PhGetServiceTypeInteger(
    __in PWSTR ServiceType
    );

PWSTR PhGetServiceStartTypeString(
    __in ULONG ServiceStartType
    );

ULONG PhGetServiceStartTypeInteger(
    __in PWSTR ServiceStartType
    );

PWSTR PhGetServiceErrorControlString(
    __in ULONG ServiceErrorControl
    );

ULONG PhGetServiceErrorControlInteger(
    __in PWSTR ServiceErrorControl
    );

PH_SERVICE_CHANGE PhGetServiceChange(
    __in PPH_SERVICE_MODIFIED_DATA Data
    );

VOID PhUpdateProcessItemServices(
    __in PPH_PROCESS_ITEM ProcessItem
    );

VOID PhServiceProviderUpdate(
    __in PVOID Object
    );

// modprv

#ifndef MODPRV_PRIVATE
extern PPH_OBJECT_TYPE PhModuleProviderType;
extern PPH_OBJECT_TYPE PhModuleItemType;
#endif

typedef struct _PH_MODULE_ITEM
{
    ULONG RunId;
    PVOID BaseAddress;
    ULONG Size;
    ULONG Flags;
    PPH_STRING Name;
    PPH_STRING FileName;
    PH_IMAGE_VERSION_INFO VersionInfo;

    PPH_STRING SizeString;

    WCHAR BaseAddressString[PH_PTR_STR_LEN_1];

    BOOLEAN IsFirst;
} PH_MODULE_ITEM, *PPH_MODULE_ITEM;

typedef struct _PH_MODULE_PROVIDER
{
    PPH_HASHTABLE ModuleHashtable;
    PH_FAST_LOCK ModuleHashtableLock;
    PH_CALLBACK ModuleAddedEvent;
    PH_CALLBACK ModuleRemovedEvent;
    PH_CALLBACK UpdatedEvent;
    ULONG RunCount;

    HANDLE ProcessId;
    HANDLE ProcessHandle;
} PH_MODULE_PROVIDER, *PPH_MODULE_PROVIDER;

BOOLEAN PhInitializeModuleProvider();

PPH_MODULE_PROVIDER PhCreateModuleProvider(
    __in HANDLE ProcessId
    );

PPH_MODULE_ITEM PhCreateModuleItem();

PPH_MODULE_ITEM PhReferenceModuleItem(
    __in PPH_MODULE_PROVIDER ModuleProvider,
    __in PVOID BaseAddress
    );

VOID PhDereferenceAllModuleItems(
    __in PPH_MODULE_PROVIDER ModuleProvider
    );

VOID PhModuleProviderUpdate(
    __in PVOID Object
    );

// thrdprv

#ifndef THRDPRV_PRIVATE
extern PPH_OBJECT_TYPE PhThreadProviderType;
extern PPH_OBJECT_TYPE PhThreadItemType;
#endif

typedef struct _PH_THREAD_ITEM
{
    ULONG RunId;
    HANDLE ThreadId;

    PH_UINT32_DELTA ContextSwitchesDelta;
    PH_UINT64_DELTA CyclesDelta;
    LONG Priority;
    ULONG64 StartAddress;
    PPH_STRING StartAddressString;
    PH_SYMBOL_RESOLVE_LEVEL StartAddressResolveLevel;
    KWAIT_REASON WaitReason;
    LONG PriorityWin32;
    PPH_STRING PriorityWin32String;

    HANDLE ThreadHandle;

    BOOLEAN IsGuiThread;
    BOOLEAN JustResolved;

    PPH_STRING ContextSwitchesDeltaString;
    PPH_STRING CyclesDeltaString;

    WCHAR ThreadIdString[PH_INT32_STR_LEN_1];
} PH_THREAD_ITEM, *PPH_THREAD_ITEM;

typedef struct _PH_THREAD_PROVIDER
{
    PPH_HASHTABLE ThreadHashtable;
    PH_FAST_LOCK ThreadHashtableLock;
    PH_CALLBACK ThreadAddedEvent;
    PH_CALLBACK ThreadModifiedEvent;
    PH_CALLBACK ThreadRemovedEvent;
    PH_CALLBACK UpdatedEvent;
    PH_CALLBACK LoadingStateChangedEvent;
    ULONG RunCount;

    HANDLE ProcessId;
    HANDLE ProcessHandle;
    PPH_SYMBOL_PROVIDER SymbolProvider;
    PH_EVENT SymbolsLoadedEvent;
    LONG SymbolsLoading;
    PPH_QUEUE QueryQueue;
    PH_MUTEX QueryQueueLock;
} PH_THREAD_PROVIDER, *PPH_THREAD_PROVIDER;

BOOLEAN PhInitializeThreadProvider();

PPH_THREAD_PROVIDER PhCreateThreadProvider(
    __in HANDLE ProcessId
    );

PPH_THREAD_ITEM PhCreateThreadItem(
    __in HANDLE ThreadId
    );

PPH_THREAD_ITEM PhReferenceThreadItem(
    __in PPH_THREAD_PROVIDER ThreadProvider,
    __in HANDLE ThreadId
    );

VOID PhDereferenceAllThreadItems(
    __in PPH_THREAD_PROVIDER ThreadProvider
    );

VOID PhThreadProviderUpdate(
    __in PVOID Object
    );

// hndlprv

#ifndef HNDLPRV_PRIVATE
extern PPH_OBJECT_TYPE PhHandleProviderType;
extern PPH_OBJECT_TYPE PhHandleItemType;
#endif

typedef struct _PH_HANDLE_ITEM
{
    ULONG RunId;
    HANDLE Handle;
    PVOID Object;
    ULONG Attributes;
    ACCESS_MASK GrantedAccess;

    PPH_STRING TypeName;
    PPH_STRING ObjectName;
    PPH_STRING BestObjectName;

    WCHAR HandleString[PH_PTR_STR_LEN_1];
    WCHAR ObjectString[PH_PTR_STR_LEN_1];
    WCHAR GrantedAccessString[PH_PTR_STR_LEN_1];
} PH_HANDLE_ITEM, *PPH_HANDLE_ITEM;

typedef struct _PH_HANDLE_PROVIDER
{
    PPH_HASHTABLE HandleHashtable;
    PH_QUEUED_LOCK HandleHashtableLock;
    PH_CALLBACK HandleAddedEvent;
    PH_CALLBACK HandleModifiedEvent;
    PH_CALLBACK HandleRemovedEvent;
    PH_CALLBACK UpdatedEvent;
    ULONG RunCount;

    HANDLE ProcessId;
    HANDLE ProcessHandle;
} PH_HANDLE_PROVIDER, *PPH_HANDLE_PROVIDER;

BOOLEAN PhInitializeHandleProvider();

PPH_HANDLE_PROVIDER PhCreateHandleProvider(
    __in HANDLE ProcessId
    );

PPH_HANDLE_ITEM PhCreateHandleItem(
    __in_opt PSYSTEM_HANDLE_TABLE_ENTRY_INFO Handle
    );

PPH_HANDLE_ITEM PhReferenceHandleItem(
    __in PPH_HANDLE_PROVIDER HandleProvider,
    __in HANDLE Handle
    );

VOID PhDereferenceAllHandleItems(
    __in PPH_HANDLE_PROVIDER HandleProvider
    );

VOID PhHandleProviderUpdate(
    __in PVOID Object
    );

// support

#ifndef SUPPORT_PRIVATE
extern ULONG PhMaxSizeUnit;
#endif

typedef struct _PH_INTEGER_PAIR
{
    LONG X;
    LONG Y;
} PH_INTEGER_PAIR, *PPH_INTEGER_PAIR;

typedef struct _PH_RECTANGLE
{
    union
    {
        PH_INTEGER_PAIR Position;
        struct
        {
            LONG Left;
            LONG Top;
        };
    };
    union
    {
        PH_INTEGER_PAIR Size;
        struct
        {
            LONG Width;
            LONG Height;
        };
    };
} PH_RECTANGLE, *PPH_RECTANGLE;

FORCEINLINE PH_RECTANGLE PhRectToRectangle(
    __in RECT Rect
    )
{
    PH_RECTANGLE rectangle;

    rectangle.Left = Rect.left;
    rectangle.Top = Rect.top;
    rectangle.Width = Rect.right - Rect.left;
    rectangle.Height = Rect.bottom - Rect.top;

    return rectangle;
}

FORCEINLINE RECT PhRectangleToRect(
    __in PH_RECTANGLE Rectangle
    )
{
    RECT rect;

    rect.left = Rectangle.Left;
    rect.top = Rectangle.Top;
    rect.right = Rectangle.Left + Rectangle.Width;
    rect.top = Rectangle.Top + Rectangle.Height;

    return rect;
}

FORCEINLINE VOID PhConvertRect(
    __inout PRECT Rect,
    __in PRECT ParentRect
    )
{
    Rect->right = ParentRect->right - ParentRect->left - Rect->right;
    Rect->bottom = ParentRect->bottom - ParentRect->top - Rect->bottom;
}

FORCEINLINE RECT PhMapRect(
    __in RECT InnerRect,
    __in RECT OuterRect
    )
{
    RECT rect;

    rect.left = InnerRect.left - OuterRect.left;
    rect.top = InnerRect.top - OuterRect.top;
    rect.right = InnerRect.right - OuterRect.left;
    rect.bottom = InnerRect.bottom - OuterRect.top;

    return rect;
}

VOID PhAdjustRectangleToBounds(
    __inout PPH_RECTANGLE Rectangle,
    __in PPH_RECTANGLE Bounds
    );

VOID PhCenterRectangle(
    __inout PPH_RECTANGLE Rectangle,
    __in PPH_RECTANGLE Bounds
    );

VOID PhAdjustRectangleToWorkingArea(
    __in HWND hWnd,
    __inout PPH_RECTANGLE Rectangle
    );

FORCEINLINE VOID PhCenterWindow(
    __in HWND WindowHandle,
    __in HWND ParentWindowHandle
    )
{
    RECT rect, parentRect;
    PH_RECTANGLE rectangle, parentRectangle;

    GetWindowRect(WindowHandle, &rect);
    GetWindowRect(ParentWindowHandle, &parentRect);
    rectangle = PhRectToRectangle(rect);
    parentRectangle = PhRectToRectangle(parentRect);

    PhCenterRectangle(&rectangle, &parentRectangle);
    PhAdjustRectangleToWorkingArea(WindowHandle, &rectangle);
    MoveWindow(WindowHandle, rectangle.Left, rectangle.Top,
        rectangle.Width, rectangle.Height, FALSE);
}

FORCEINLINE VOID PhLargeIntegerToSystemTime(
    __out PSYSTEMTIME SystemTime,
    __in PLARGE_INTEGER LargeInteger
    )
{
    FILETIME fileTime;

    fileTime.dwLowDateTime = LargeInteger->LowPart;
    fileTime.dwHighDateTime = LargeInteger->HighPart;
    FileTimeToSystemTime(&fileTime, SystemTime);
}

FORCEINLINE VOID PhLargeIntegerToLocalSystemTime(
    __out PSYSTEMTIME SystemTime,
    __in PLARGE_INTEGER LargeInteger
    )
{
    FILETIME fileTime;
    FILETIME newFileTime;

    fileTime.dwLowDateTime = LargeInteger->LowPart;
    fileTime.dwHighDateTime = LargeInteger->HighPart;
    FileTimeToLocalFileTime(&fileTime, &newFileTime);
    FileTimeToSystemTime(&newFileTime, SystemTime);
}

FORCEINLINE FILETIME PhSubtractFileTime(
    __inout FILETIME Value1,
    __in FILETIME Value2
    )
{
    ULARGE_INTEGER value1;
    ULARGE_INTEGER value2;
    ULARGE_INTEGER value3;
    FILETIME result;

    value1.LowPart = Value1.dwLowDateTime;
    value1.HighPart = Value1.dwHighDateTime;
    value2.LowPart = Value2.dwLowDateTime;
    value2.HighPart = Value2.dwHighDateTime;

    value3.QuadPart = value1.QuadPart - value2.QuadPart;
    result.dwLowDateTime = value3.LowPart;
    result.dwHighDateTime = value3.HighPart;

    return result;
}

VOID PhReferenceObjects(
    __in PPVOID Objects,
    __in ULONG NumberOfObjects
    );

VOID PhDereferenceObjects(
    __in PPVOID Objects,
    __in ULONG NumberOfObjects
    );

PPH_STRING PhGetMessage(
    __in PVOID DllHandle,
    __in ULONG MessageTableId,
    __in ULONG MessageLanguageId,
    __in ULONG MessageId
    );

PPH_STRING PhGetNtMessage(
    __in NTSTATUS Status
    );

PPH_STRING PhGetWin32Message(
    __in ULONG Result
    );

#define PH_MAX_MESSAGE_SIZE 400

INT PhShowMessage(
    __in HWND hWnd,
    __in ULONG Type,
    __in PWSTR Format,
    ...
    );

INT PhShowMessage_V(
    __in HWND hWnd,
    __in ULONG Type,
    __in PWSTR Format,
    __in va_list ArgPtr
    );

#define PhShowError(hWnd, Format, ...) PhShowMessage(hWnd, MB_OK | MB_ICONERROR, Format, __VA_ARGS__)
#define PhShowWarning(hWnd, Format, ...) PhShowMessage(hWnd, MB_OK | MB_ICONWARNING, Format, __VA_ARGS__)
#define PhShowInformation(hWnd, Format, ...) PhShowMessage(hWnd, MB_OK | MB_ICONINFORMATION, Format, __VA_ARGS__)

VOID PhShowStatus(
    __in HWND hWnd,
    __in_opt PWSTR Message,
    __in NTSTATUS Status,
    __in_opt ULONG Win32Result
    );

BOOLEAN PhShowContinueStatus(
    __in HWND hWnd,
    __in_opt PWSTR Message,
    __in NTSTATUS Status,
    __in_opt ULONG Win32Result
    );

BOOLEAN PhShowConfirmMessage(
    __in HWND hWnd,
    __in PWSTR Verb,
    __in PWSTR Object,
    __in_opt PWSTR Message,
    __in BOOLEAN Warning
    );

PPH_STRING PhFormatDate(
    __in_opt PSYSTEMTIME Date,
    __in_opt PWSTR Format
    );

PPH_STRING PhFormatTime(
    __in_opt PSYSTEMTIME Time,
    __in_opt PWSTR Format
    );

PPH_STRING PhFormatUInt64(
    __in ULONG64 Value,
    __in BOOLEAN GroupDigits
    );

PPH_STRING PhFormatDecimal(
    __in PWSTR Value,
    __in ULONG FractionalDigits,
    __in BOOLEAN GroupDigits
    );

PPH_STRING PhFormatSize(
    __in ULONG64 Size,
    __in ULONG MaxSizeUnit
    );

PPH_STRING PhFormatGuid(
    __in PGUID Guid
    );

PVOID PhGetFileVersionInfo(
    __in PWSTR FileName
    );

ULONG PhGetFileVersionInfoLangCodePage(
    __in PVOID VersionInfo
    );

PPH_STRING PhGetFileVersionInfoString(
    __in PVOID VersionInfo,
    __in PWSTR SubBlock
    );

PPH_STRING PhGetFileVersionInfoString2(
    __in PVOID VersionInfo,
    __in ULONG LangCodePage,
    __in PWSTR StringName
    );

PPH_STRING PhGetFullPath(
    __in PWSTR FileName,
    __out_opt PULONG IndexOfFileName
    );

PPH_STRING PhExpandEnvironmentStrings(
    __in PWSTR String
    );

PPH_STRING PhGetBaseName(
    __in PPH_STRING FileName
    );

PPH_STRING PhGetSystemDirectory();

PPH_STRING PhGetSystemRoot();

PPH_STRING PhGetApplicationModuleFileName(
    __in HMODULE ModuleHandle,
    __out_opt PULONG IndexOfFileName
    );

PPH_STRING PhGetApplicationFileName();

PPH_STRING PhGetApplicationDirectory();

PPH_STRING PhGetKnownLocation(
    __in ULONG Folder,
    __in_opt PWSTR AppendPath
    );

FORCEINLINE BOOLEAN PhFileExists(
    __in PWSTR FileName
    )
{
    return GetFileAttributes(FileName) != INVALID_FILE_ATTRIBUTES;
}

VOID PhShellExecute(
    __in HWND hWnd,
    __in PWSTR FileName,
    __in PWSTR Parameters
    );

BOOLEAN PhShellExecuteEx(
    __in HWND hWnd,
    __in PWSTR FileName,
    __in PWSTR Parameters,
    __in ULONG ShowWindowType,
    __in BOOLEAN StartAsAdmin,
    __in_opt ULONG Timeout
    );

VOID PhShellExploreFile(
    __in HWND hWnd,
    __in PWSTR FileName
    );

VOID PhShellProperties(
    __in HWND hWnd,
    __in PWSTR FileName
    );

VOID PhShellOpenKey(
    __in HWND hWnd,
    __in PPH_STRING KeyName
    );

PPH_STRING PhQueryRegistryString(
    __in HKEY KeyHandle,
    __in_opt PWSTR ValueName
    );

PVOID PhCreateOpenFileDialog();

PVOID PhCreateSaveFileDialog();

VOID PhFreeFileDialog(
    __in PVOID FileDialog
    );

BOOLEAN PhShowFileDialog(
    __in HWND hWnd,
    __in PVOID FileDialog
    );

typedef struct _PH_FILETYPE_FILTER
{
    PWSTR Name;
    PWSTR Filter;
} PH_FILETYPE_FILTER, *PPH_FILETYPE_FILTER;

VOID PhSetFileDialogFilter(
    __in PVOID FileDialog,
    __in PPH_FILETYPE_FILTER Filters,
    __in ULONG NumberOfFilters
    );

PPH_STRING PhGetFileDialogFileName(
    __in PVOID FileDialog
    );

VOID PhSetFileDialogFileName(
    __in PVOID FileDialog,
    __in PWSTR FileName
    );

typedef struct _PH_COMMAND_LINE_OPTION
{
    ULONG Id;
    PWSTR LongName;
    PWSTR ShortName;
    BOOLEAN AcceptArgument;

    PWSTR Description;
} PH_COMMAND_LINE_OPTION, *PPH_COMMAND_LINE_OPTION;

typedef VOID (NTAPI *PPH_COMMAND_LINE_CALLBACK)(
    __in PPH_COMMAND_LINE_OPTION Option,
    __in PPH_STRING Value,
    __in PVOID Context
    );

VOID PhParseCommandLine(
    __in PPH_STRINGREF CommandLine,
    __in PPH_COMMAND_LINE_OPTION Options,
    __in ULONG NumberOfOptions,
    __in PPH_COMMAND_LINE_CALLBACK Callback,
    __in PVOID Context
    );

FORCEINLINE PVOID PhAllocateCopy(
    __in PVOID Data,
    __in ULONG Size
    )
{
    PVOID copy;

    copy = PhAllocate(Size);
    memcpy(copy, Data, Size);

    return copy;
}

// dbgcon

VOID PhShowDebugConsole();

#endif
