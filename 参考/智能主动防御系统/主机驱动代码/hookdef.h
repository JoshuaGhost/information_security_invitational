
/***********新增加的变量类型定义****************/
typedef unsigned int    UINT;
typedef char            CHAR;
typedef char *          PCHAR;
typedef PVOID           POBJECT;
typedef ULONG           DWORD;
typedef unsigned char   BYTE;
/***********************************************/



/*************字符串常量***********************/

//驱动设备名字和符号链接定义
#define DEVICE_NAME       L"\\Device\\protect"
#define DOS_DEVICE_NAME  L"\\DosDevices\\protect"

#define SYSNAME         "System"

/***********************************************/


#define MUTEX_INIT(v)      KeInitializeMutex( &v, 0 )
#define MUTEX_WAIT(v)      KeWaitForMutexObject( &v, Executive, KernelMode, FALSE, NULL )
#define MUTEX_RELEASE(v)   KeReleaseMutex( &v, FALSE )

/***********数据范围定义************************/
#ifndef PAGE_SIZE
#if defined(_ALPHA_)
#define PAGE_SIZE 0x2000  // 8K
#else
#define PAGE_SIZE 0x1000  // 4K
#endif
#endif


#define MAXMEM          1000000    
#define MAX_STORE	(PAGE_SIZE*16 - 8)

// The maximum length of Registry values that will be copied
#define MAXROOTLEN      128

// The maximum registry path length that will be copied
#define MAXPATHLEN     1024

#define NUMROOTKEYS     4
#define MAXVALLEN      256
#define MAXREGDATA     2048
#define FUNCTIONNAME   64
#define MAXPROCDATA    512
#define MAXPROCNAME    128

// Length of process name buffer. Process names are at most 16 characters so
// we take into account a trailing NULL.
#define MAXPROCNAMELEN  20


#define MAXFILEPATHLEN 1024

// Maximum length of data that will be copied to the "other" field in the display
#define MAXDATALEN     2048

// Maximum length of root keys that we will produce abbreviations for
#define MAXINFOLEN      512


///////////////////////////不同的windows版本下面的偏移值不同
#define  EPROCESS_SIZE       0x25C //EPROCESS结构大小

#define  PEB_OFFSET          0x1B0
#define  FILE_NAME_OFFSET    0x174
#define  PROCESS_LINK_OFFSET 0x088
#define  PROCESS_ID_OFFSET   0x084
#define  EXIT_TIME_OFFSET    0x078

#define  OBJECT_HEADER_SIZE  0x018
#define  OBJECT_TYPE_OFFSET  0x008

#define PDE_INVALID 2 
#define PTE_INVALID 1 
#define VALID 0 

/***********************************************/


/*************所用结构体定义*******************/

//
typedef struct _store {
	ULONG               Len;
	struct _store *     Next;
	char                Data[ MAX_STORE ];
} STORE_BUF, *PSTORE_BUF;

//受保护的注册表键/值
typedef struct _PROTECT_REGEDIT//应该是每个链表的节点
{
	char key[MAXPATHLEN];
	//char valuename[MAXVALUELEN];
	char info[MAXINFOLEN];//describe the key information,because this is the danger operate
	//int score;
	PVOID next;//指向下一个节点
}PROTECT_REGEDIT,*PPROTECT_REGEDIT;

//受保护的程序
typedef struct _WHITE_PROCESS_LIST
{
	char name[MAXPROCNAME];
	char fnName[FUNCTIONNAME];
	PVOID next;
}WHITE_PROCESS_LIST,*PWHITE_PROCESS_LIST;

#define DEFAULTNAMELEN  (9*sizeof(WCHAR))
WCHAR                   DefaultValueString[] = L"(Default)";
UNICODE_STRING          DefaultValue = {
	DEFAULTNAMELEN,
	DEFAULTNAMELEN,
	DefaultValueString
};


typedef struct _rootkey {
	CHAR                RootName[256];
	CHAR                RootShort[32];
	ULONG               RootNameLen;
} ROOTKEY, *PROOTKEY;

typedef struct _nameentry {
	POBJECT              Object;
	PCHAR                FullPathName;
	struct _nameentry    *Next;//指向下一个节点
} HASH_ENTRY, *PHASH_ENTRY;


#define NUMHASH         0x100
#define HASHOBJECT(_regobject)          (((ULONG)_regobject)>>2)%NUMHASH//建立hash表


#define ObjectNameInformation  1

#define SystemHandleInformation 0x10

typedef struct _SYSTEM_HANDLE_INFORMATION {
	ULONG ProcessId;
	UCHAR ObjectTypeNumber;
	UCHAR Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} _SYSTEM_HANDLE_INFORMATION, *P_SYSTEM_HANDLE_INFORMATION;


typedef struct _SYSTEM_HANDLE_INformATION_EX {
	ULONG NumberOfHandles;
	_SYSTEM_HANDLE_INFORMATION Information[1];
} _SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;


//显示进程信息
typedef struct _myprocess {

  ULONG pEProcess;
  ULONG PID;
  char pFileName[20];
  char pPathName[MAXPATHLEN];
	struct _myprocess *     Next;
} PROCESS_BUF, *PPROCESS_BUF;

typedef struct struct_process
{
	ULONG pEProcess;
  ULONG PID;
  char pFileName[20];
  char pPathName[MAXPATHLEN];
}struct_process;


/***********************************************/


NTSTATUS ZwQuerySystemInformation( 
		IN ULONG SystemInformationClass, 
		IN PVOID SystemInformation, 
		IN ULONG SystemInformationLength, 
		OUT PULONG ReturnLength);

NTSTATUS KeAttachProcess(PEPROCESS pPeb);
NTSTATUS KeDetachProcess();

NTSTATUS ZwDuplicateObject(
                 IN HANDLE                 SourceProcessHandle,
                 IN PHANDLE                 SourceHandle,
                 IN HANDLE                 TargetProcessHandle,
                 OUT PHANDLE               TargetHandle,
                 IN ACCESS_MASK             DesiredAccess OPTIONAL,
                 IN BOOLEAN                 InheritHandle,
                 IN ULONG                   Options );

NTSTATUS ZwQueryObject(
				IN HANDLE                ObjectHandle,
				IN ULONG                 ObjectInformationClass,
				OUT PVOID                ObjectInformation,
				IN ULONG                 ObjectInformationLength,
				OUT PULONG               ReturnLength OPTIONAL);

PEPROCESS  IoThreadToProcess(IN PETHREAD  Thread); 

NTSTATUS  PsLookupThreadByThreadId(IN HANDLE ThreadId,    OUT PETHREAD *Thread);


/*********************功能函数声明***********************/

//钩住相关函数
VOID HookRegistry( void );
//解除钩子
VOID UnhookRegistry(void);

//IOCTL控制码处理例程
BOOLEAN  RegmonDeviceControl(IN PFILE_OBJECT FileObject, 
							 IN BOOLEAN Wait,
							 IN PVOID InputBuffer, 
							 IN ULONG InputBufferLength, 
							 OUT PVOID OutputBuffer, 
							 IN ULONG OutputBufferLength, 
							 IN ULONG IoControlCode,
							 OUT PIO_STATUS_BLOCK IoStatus, 
							 IN PDEVICE_OBJECT DeviceObject);
//卸载驱动
VOID HookUnload( IN PDRIVER_OBJECT DriverObject ); 

//显示进程所用到的函数
VOID EnumProcess();
ULONG  GetPebAddress();
VOID    ShowProcess(ULONG pEProcess);
ULONG VALIDpage(ULONG addr) ;
BOOLEAN IsaRealProcess(ULONG i) ;


/*******************************************************/


/************被钩住的导出函数声明***********************/

/////////进程相关函数：3个////////////
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSection(
               OUT PHANDLE SectionHandle,
               IN ACCESS_MASK DesiredAccess,
               IN POBJECT_ATTRIBUTES ObjectAttributes,
               IN PLARGE_INTEGER SectionSize OPTIONAL,
               IN ULONG Protect,
               IN ULONG Attributes,
               IN HANDLE FileHandle);

typedef 
NTSTATUS 
(*ZWCREATESECTION)(
			   OUT PHANDLE SectionHandle,
               IN ACCESS_MASK DesiredAccess,
               IN POBJECT_ATTRIBUTES ObjectAttributes,
               IN PLARGE_INTEGER SectionSize OPTIONAL,
               IN ULONG Protect,
               IN ULONG Attributes,
               IN HANDLE FileHandle);

//OpenProcess:获得进程句柄
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcess(
             OUT PHANDLE ProcessHandle,
             IN ACCESS_MASK DesiredAccess,
             IN POBJECT_ATTRIBUTES ObjectAttributes,
             IN PCLIENT_ID ClientId OPTIONAL);

typedef 
NTSTATUS  
(*ZWOPENPROCESS)(
			 OUT PHANDLE ProcessHandle,
             IN ACCESS_MASK DesiredAccess,
             IN POBJECT_ATTRIBUTES ObjectAttributes,
             IN PCLIENT_ID ClientId OPTIONAL);

//TerminateProcess:结束进程
NTSYSAPI
NTSTATUS
NTAPI
ZwTerminateProcess(
             IN HANDLE ProcessHandle OPTIONAL,
             IN NTSTATUS ExitStatus);

typedef 
NTSTATUS  
(*ZWTERMINATEPROCESS)(
			 IN HANDLE ProcessHandle OPTIONAL,
			 IN NTSTATUS ExitStatus);

/////////////////////////////////

///////注册表相关函数：7个////////////

//OpenKey:打开键
NTSYSAPI
NTSTATUS
NTAPI 
ZwOpenKey(
		 IN PHANDLE pKey, 
		 IN OUT ACCESS_MASK DesirdAccess, 
		 IN POBJECT_ATTRIBUTES ObjectAttributes );

typedef
NTSTATUS
(*ZWOPENKEY)(
		 IN PHANDLE pKey,
		 IN OUT ACCESS_MASK DesirdAccess, 
		 IN POBJECT_ATTRIBUTES ObjectAttributes);

//CreateKey:创建键
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateKey(
			OUT PHANDLE KeyHandle,
			IN ACCESS_MASK DesiredAccess,
			IN POBJECT_ATTRIBUTES ObjectAttributes,
			IN ULONG TitleIndex,
			IN PUNICODE_STRING Class OPTIONAL,
			IN ULONG CreateOptions,
			OUT PULONG Disposition OPTIONAL);
typedef 
NTSTATUS 
(*ZWCREATEKEY)(
			OUT PHANDLE KeyHandle,
			IN ACCESS_MASK DesiredAccess,
			IN POBJECT_ATTRIBUTES ObjectAttributes,
			IN ULONG TitleIndex,
			IN PUNICODE_STRING Class OPTIONAL,
			IN ULONG CreateOptions,
			OUT PULONG Disposition OPTIONAL);

//DeleteValueKey：删除键值
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteValueKey(
		         IN HANDLE KeyHandle,
				 IN PUNICODE_STRING ValueName);

typedef
NTSTATUS
(*ZWDELETEVALUEKEY)(
				 IN HANDLE KeyHandle,
				 IN PUNICODE_STRING ValueName);

//SetValueKey:设置键值
NTSYSAPI
NTSTATUS
NTAPI
ZwSetValueKey(
			  IN HANDLE KeyHandle,
			  IN PUNICODE_STRING ValueName,
			  IN ULONG TitleIndex,
			  IN ULONG Type,
			  IN PVOID Data,
			  IN ULONG DataSize);

typedef 
NTSTATUS
(*ZWSETVALUEKEY)(
			  IN HANDLE KeyHandle,
			  IN PUNICODE_STRING ValueName,
			  IN ULONG TitleIndex,
			  IN ULONG Type,
			  IN PVOID Data,
			  IN ULONG DataSize);

//CloseKey：关闭键
NTSYSAPI
NTSTATUS
NTAPI
ZwCloseKey(IN HANDLE );

typedef
NTSTATUS 
(*ZWCLOSEKEY)(IN HANDLE);

//DeleteKey：删除键
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteKey(HANDLE);

typedef 
NTSTATUS 
(*ZWDELETEKEY)(HANDLE);

//QueryKey：查询键值
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryKey(
		  IN HANDLE, 
		  IN KEY_INFORMATION_CLASS,
          OUT PVOID, 
		  IN ULONG, 
		  OUT PULONG );

typedef 
NTSTATUS  
(*ZWQUERYKEY)( 
		  IN HANDLE, 
		  IN KEY_INFORMATION_CLASS,
		  OUT PVOID, 
		  IN ULONG, 
		  OUT PULONG );

//////////////////////////////////////////


//////////////内存相关函数////////////////

//WriteVirtualMemory:写内存函数
NTSYSAPI
NTSTATUS
NTAPI
ZwWriteVirtualMemory(
					 IN HANDLE ProcessHandle,
					 IN PVOID BaseAddress,
					 IN PVOID Buffer,
					 IN ULONG BufferLength,
					 OUT PULONG ReturnLength OPTIONAL);

typedef 
NTSTATUS 
(*ZWWRITEVIRTUALMEMORY)(
	                 IN HANDLE ProcessHandle,
	                 IN PVOID BaseAddress,
                     IN PVOID Buffer,
                     IN ULONG BufferLength,
	                 OUT PULONG ReturnLength OPTIONAL);
/////////////////////////////////

///////////驱动相关函数//////////////////

//LoadDriver：加载驱动
NTSYSAPI
NTSTATUS
NTAPI
ZwLoadDriver(
			 IN PUNICODE_STRING DriverServiceName);

typedef 
NTSTATUS
(*ZWLOADDRIVER)(
			 IN PUNICODE_STRING DriverServiceName);
/////////////////////////////////////////////


////////////其他////////////////////////////

//SetWindowsHookEx:用户态钩子函数
typedef  
HHOOK  
(*NTUSERSETWINDOWSHOOKEX)(
						  HINSTANCE Mod,
						  PUNICODE_STRING UnsafeModuleName,
						  DWORD ThreadId,
						  int HookId,
						  FARPROC HookProc,
						  BOOL Ansi);
/////////////////////////////////////////////

/***********************************************/
