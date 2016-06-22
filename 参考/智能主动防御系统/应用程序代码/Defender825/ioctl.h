#define FILE_DEVICE_REGMON      0x00008306

#define REGMON_hook      (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x00, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define REGMON_unhook      (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x01, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define REGMON_getstats  (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x03, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define REGMON_getprocess  (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x04, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define Begin_hook      (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x05, METHOD_BUFFERED, FILE_ANY_ACCESS )

#ifndef PAGE_SIZE
#if defined(_ALPHA_)
#define PAGE_SIZE 0x2000  // 8K
#else
#define PAGE_SIZE 0x1000  // 4K
#endif
#endif

#define MAX_STORE (PAGE_SIZE*16 - 8)
#define MAXPATHLEN     1024

typedef struct _store {
	ULONG               Len;
	struct _store *     Next;
	char                Data[ MAX_STORE ];
} STORE_BUF, *PSTORE_BUF;

typedef struct {
	ULONG	seq;
	char	text[1];
} ENTRY, *PENTRY;

typedef struct struct_process
{
	ULONG pEProcess;
  ULONG PID;
  char pFileName[20];
  char pPathName[MAXPATHLEN];
}struct_process;

