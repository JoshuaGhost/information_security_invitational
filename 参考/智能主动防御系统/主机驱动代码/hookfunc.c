#include <ntddk.h>
#include<stdio.h>
#include <windef.h>
#include "hookdef.h"
#include "hookioctl.h"

int TOTALCOUNT=0;  //Debug���������

DWORD  protectedPid=0;  //GUI����PID

//ͨ���¼�
PVOID	gpEventObjectReceive = NULL;  //�ȴ��û�ѡ��
PVOID	gpEventObjectSend    = NULL;  //���û��߳����У�����Σ����ʾ�Ի���
KMUTEX  StoreMutex                 ;  //������ʴ洢�ռ䣺�����ͷţ�
KEVENT  event                      ;  //����waituser������


PSTORE_BUF              Store           = NULL;


ULONG                   NumStore        = 0;
ULONG                   MaxStore        = MAXMEM/MAX_STORE;

ULONG                   ProcessNameOffset;
BOOLEAN                 RegHooked = FALSE;
char                    *output;

ULONG                   Sequence        = 0;

PPROTECT_REGEDIT  gpRegeditProtect=NULL;//ָ���ܱ�����ע��������ͷָ��.
PWHITE_PROCESS_LIST gpWhiteList=NULL;   //������

ULONG reg_protect_num=0;
ULONG process_white_list=0;
ULONG   shadow_ssdt_hook=1;


PHASH_ENTRY             HashTable[NUMHASH];
KMUTEX                  HashMutex;
PHASH_ENTRY             FreeHashList=NULL;
DWORD                   indexMem;

PEPROCESS               crsEProc;
ULONG                    OldCr0;
ULONG NtUserSetWindowsHookEx_callnumber=0;


//��ʾ�������õ���ȫ�ֱ���
ULONG     pebAddress;         //PEB��ַ��ǰ�벿��
PEPROCESS pSystem;            //system����
ULONG     pObjectTypeProcess; //���̶�������
UCHAR totalProcess;
PPROCESS_BUF PROCESS_HEAD=NULL;

ROOTKEY CurrentUser[2] = {
	{ "\\\\REGISTRY\\USER\\S", "HKCU", 0 },
	{ "HKU\\S", "HKCU", 0 }
};

ROOTKEY RootKey[NUMROOTKEYS] = {
	{ "\\\\REGISTRY\\USER", "HKU", 0 },
	{ "\\\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\HARDWARE PROFILES\\CURRENT", 
	"HKCC", 0 },
	{ "\\\\REGISTRY\\MACHINE\\SOFTWARE\\CLASSES", "HKCR", 0 },
	{ "\\\\REGISTRY\\MACHINE", "HKLM", 0 }
};


#pragma pack(1)
typedef struct ServiceDescriptorEntry {
	unsigned int *ServiceTableBase;
	unsigned int *ServiceCounterTableBase; //Used only in checked build
	unsigned int NumberOfServices;
	unsigned char *ParamTableBase;
} ServiceDescriptorTableEntry_t, *PServiceDescriptorTableEntry_t;
#pragma pack()

PServiceDescriptorTableEntry_t  KeServiceDescriptorTableShadow;

__declspec(dllimport)  ServiceDescriptorTableEntry_t KeServiceDescriptorTable;
__declspec(dllimport) _stdcall KeAddSystemServiceTable(PVOID, PVOID, PVOID, PVOID, PVOID);
#define SYSTEMSERVICE(_function)  KeServiceDescriptorTable.ServiceTableBase[ *(PULONG)((PUCHAR)_function+1)]//�õ�NT�����ĵ�ַ


PMDL  g_pmdlSystemCall;
PVOID *MappedSystemCallTable;//��ssdt����ͬһ����ַ
#define SYSCALL_INDEX(_Function) *(PULONG)((PUCHAR)_Function+1)//�õ���ssdt���е�����
#define HOOK_SYSCALL(_Function, _Hook, _Orig )  \
	_Orig = (PVOID) InterlockedExchange( (PLONG) &MappedSystemCallTable[SYSCALL_INDEX(_Function)], (LONG) _Hook)

#define UNHOOK_SYSCALL(_Function, _Hook, _Orig )  \
	InterlockedExchange( (PLONG) &MappedSystemCallTable[SYSCALL_INDEX(_Function)], (LONG) _Hook)

#define  MYHOOK_SYSCALL(_Index,_Hook,_Orig) \
	_Orig = (PVOID) InterlockedExchange( (PLONG) &MappedSystemCallTable[_Index],(LONG)_Hook)

#define MYUNHOOK_SYSCALL(_Index, _Hook, _Orig )  \
	InterlockedExchange( (PLONG) &MappedSystemCallTable[_Index], (LONG) _Hook)



extern NTSTATUS
ObQueryNameString(
				  IN  PVOID Object,
				  OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
				  IN  ULONG Length,
				  OUT PULONG ReturnLength
				  );

NTSTATUS PsLookupProcessByProcessId(
									IN ULONG ulProcId, 
									OUT PEPROCESS * pEProcess
									);

VOID ConvertToUpper( PCHAR Dest, PCHAR Source, ULONG Len );


 typedef NTSTATUS (*QUERY_INFO_PROCESS) (
										IN HANDLE ProcessHandle,
										IN PROCESSINFOCLASS ProcessInformationClass,
										OUT PVOID ProcessInformation,
										IN ULONG ProcessInformationLength,
										OUT PULONG ReturnLength );

QUERY_INFO_PROCESS ZwQueryInformationProcess;
#define ProcessImageFileName 27

void InsertList(char *data);




//������غ���
ZWCREATESECTION  RealZwCreateSection;

ZWOPENPROCESS  RealZwOpenProcess;           //�򿪽��̣���ý��̾��

ZWTERMINATEPROCESS RealZwTerminateProcess;  //��������


//ע�����غ���
ZWOPENKEY RealZwOpenKey;                    //�򿪼�

ZWCREATEKEY RealZwCreateKey;                //������

ZWDELETEVALUEKEY   RealZwDeleteValueKey;    //ɾ����ֵ

ZWSETVALUEKEY  RealZwSetValueKey;           //���ü�ֵ

ZWCLOSEKEY RealZwCloseKey;                  //�رռ�

ZWDELETEKEY  RealZwDeleteKey;               //ɾ����

ZWQUERYKEY  RealZwQueryKey;                 //��ѯ��ֵ


//�ڴ���غ���
ZWWRITEVIRTUALMEMORY  RealZwWriteVirtualMemory;  //д�ڴ�


//������غ���
ZWLOADDRIVER  RealZwLoadDriver;                  //��������


//����
NTUSERSETWINDOWSHOOKEX  RealNtUserSetWindowsHookEx;//SetWindowsHookEx:�û�̬���Ӻ���





//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//����hash����߲���Ч��
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
VOID RegmonStoreHash( POBJECT object, PCHAR fullname )
{
	PHASH_ENTRY     newEntry;

	MUTEX_WAIT( HashMutex );

	if( FreeHashList ) {

		newEntry = FreeHashList;
		FreeHashList = newEntry->Next;//FreeHashListָ���

	} else {

		newEntry = ExAllocatePool( PagedPool, sizeof(HASH_ENTRY));
	}

	newEntry->Object                = object;
	newEntry->FullPathName          = ExAllocatePool( PagedPool, strlen(fullname)+1 );
	newEntry->Next                  = HashTable[ HASHOBJECT( object) ];//�����Ҫ����ĵ�ַ
	HashTable[ HASHOBJECT(object) ] = newEntry;     
	strcpy( newEntry->FullPathName, fullname );

	MUTEX_RELEASE( HashMutex );
}


VOID RegmonHashCleanup()
{
	PHASH_ENTRY             hashEntry, nextEntry;
	ULONG                   i;

	MUTEX_WAIT( HashMutex );    

	//
	// First free the hash table entries
	//       
	for( i = 0; i < NUMHASH; i++ ) {
		hashEntry = HashTable[i];
		while( hashEntry ) {
			nextEntry = hashEntry->Next;
			ExFreePool( hashEntry->FullPathName );
			ExFreePool( hashEntry);
			hashEntry = nextEntry;
		}
	}

	hashEntry = FreeHashList;
	while( hashEntry ) {
		nextEntry = hashEntry->Next;
		ExFreePool( hashEntry );
		hashEntry = nextEntry;
	}
	MUTEX_RELEASE( HashMutex );
}


VOID RegmonFreeHashEntry( POBJECT object )
{
	PHASH_ENTRY             hashEntry, prevEntry;

	MUTEX_WAIT( HashMutex );

	//
	// look-up the entry
	//
	hashEntry = HashTable[ HASHOBJECT( object ) ];
	prevEntry = NULL;
	while( hashEntry && hashEntry->Object != object ) {
		prevEntry = hashEntry;
		hashEntry = hashEntry->Next;
	}


	if( !hashEntry ) {
		MUTEX_RELEASE( HashMutex );
		return;
	}

	
	if( prevEntry ) 
		prevEntry->Next = hashEntry->Next;
	else 
		HashTable[ HASHOBJECT( object )] = hashEntry->Next;//


	ExFreePool( hashEntry->FullPathName );//
	hashEntry->Next = FreeHashList;
	FreeHashList = hashEntry;//

	MUTEX_RELEASE( HashMutex );
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//����洢�ڵ㲿��
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-------------------------------------------------------------------------------------------
VOID RegmonFreeStore()
{
	PSTORE_BUF      next;

	while( Store ) {
		next = Store->Next;
		ExFreePool( Store );
		Store = next;
		DbgPrint("store is exist\n");
	}
//	DbgPrint("free store ok");
}       



void RegmonNewStore( void )
{
	PSTORE_BUF prev = Store, newstore;//

	if( MaxStore == NumStore ) {//allow to 
		Store->Len = 0;
		return; 
	}


	if( !Store->Len ) {

		return ;
	}

	newstore = ExAllocatePool( PagedPool, sizeof(*Store) );
	if( newstore ) { 

		Store   = newstore;
		Store->Len  = 0;
		Store->Next = prev;//ָ����һ����ͷ�鷨�½ڵ㶼����ͷ��
		NumStore++;
//		DbgPrint("allocate new store ok");

	} 
	else {

		Store->Len = 0;
		return ;

	}
	
}



PSTORE_BUF RegmonOldestStore( void )
{
	PSTORE_BUF  ptr = Store, prev = NULL;

	while ( ptr->Next ) {

		ptr = (prev = ptr)->Next;
	}
	if ( prev ) {

		prev->Next = NULL;  //prev�����ڶ����ڵ�  
	}
	NumStore--;
//	DbgPrint("oldest store emove ok");
	return ptr;
}



VOID RegmonResetStore()
{
	PSTORE_BUF  current, next;

	MUTEX_WAIT( StoreMutex );

	//
	// Traverse the list of output buffers
	//
	current = Store->Next;
	while( current ) {

		//
		// Free the buffer
		//
		next = current->Next;
		ExFreePool( current );//�ͷ���Щ�ڵ�
		current = next;
	}

	// 
	// Move the output pointer in the buffer that's being kept
	// the start of the buffer.
	// 
	Store->Len = 0;
	Store->Next = NULL;


	MUTEX_RELEASE( StoreMutex );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Init the protect list and white_list
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
NTSTATUS InitProtectList(PPROTECT_REGEDIT *phead)//��ʼ������.
{
	
	PPROTECT_REGEDIT temp1=NULL,temp2=NULL;
	char  *buffer=ExAllocatePool(PagedPool,MAXREGDATA);
	char  buffer2[4]={-1};
	HANDLE file_handle=NULL ;//���ļ�ȫ�־��
	ULONG length,n=0,j=0,i=0,number=0;
	NTSTATUS status;
	OBJECT_ATTRIBUTES object_attributes;
	IO_STATUS_BLOCK io_status={0};
	LARGE_INTEGER offset={0};
	UNICODE_STRING filepath = RTL_CONSTANT_STRING(L"\\??\\c:\\protect.txt");
	char *tempbuf2=ExAllocatePool(PagedPool,MAXINFOLEN);
	char *tempbuf=ExAllocatePool(PagedPool,MAXINFOLEN);
	RtlZeroMemory(tempbuf,MAXINFOLEN);
	RtlZeroMemory(tempbuf2,MAXINFOLEN);
	RtlZeroMemory(buffer,MAXREGDATA);

	length = MAXREGDATA;
	//��ʼ��OBJECT_ATTRIBUTES
	InitializeObjectAttributes(&object_attributes,&filepath,OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,NULL,NULL);
	//���ļ�
	status=ZwCreateFile(&file_handle,FILE_READ_DATA|SYNCHRONIZE,&object_attributes,&io_status,NULL,FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,FILE_OPEN_IF,FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,0);
	if (!NT_SUCCESS(status))
	{
//		DbgPrint("create file error %x\n",(ULONG)status);
		return status;
	}
	//offset.HighPart=-1;
	//offset.LowPart=FILE_USE_FILE_POINTER_POSITION;
	status = ZwReadFile(file_handle,NULL,NULL,NULL,&io_status,buffer,length,&offset,NULL);//���ļ��еĶ������뻺��
//	DbgPrint("the read length %d\n",io_status.Information);
	offset.QuadPart+=io_status.Information;
	buffer[io_status.Information]='\0';
	if(NT_SUCCESS(status))
	{
		//len = io_status.Information;

		status = ZwReadFile(file_handle,NULL,NULL,NULL,&io_status,buffer2,sizeof(buffer2),&offset,NULL);

//		DbgPrint("the read length %d\n",io_status.Information);
	}

	if (!NT_SUCCESS(status))
	{
		if (status==STATUS_END_OF_FILE)//ע�⣬��������ļ�β�Ļ�����ô����ֵ��һ������.
		{
			DbgPrint("read all files\n");
			ZwClose(file_handle);
			status=STATUS_SUCCESS;
		}
		else
		{
//			DbgPrint("read file error\n");
			ZwClose(file_handle);
			return status;
		}
	}
	// buffer[len]='\0';
	for(n=0;buffer[n]!='\0';)
	{
		i=0;
		RtlZeroMemory(tempbuf,MAXINFOLEN);
		while (buffer[n]!='\n' && buffer[n]!='\0')//����ÿһ������
		{
			tempbuf[i]=buffer[n];
			i=i+1;
			n=n+1;
		}
		//��õİ취��ÿ��ʹ��ǰ��������飬��������̫�˷�ʱ��
		if(buffer[n]=='\n')
		{
		   tempbuf[i-1]='\0';//use '\0' to change '\n' at the last of line
        }
		else
			tempbuf[i]='\0';
		temp1=(PPROTECT_REGEDIT)ExAllocatePool(PagedPool,sizeof(PROTECT_REGEDIT));//��������ռ�
		if(temp1==NULL)
		{
//			DbgPrint("allocate memory error");
			return status;
		}
		j=0;//propare to copy the first data
		while (tempbuf[j]!='\t')//copy the first text
		{
			tempbuf2[j]=tempbuf[j];
			j=j+1;
		}
		tempbuf2[j]='\0';//use '\0' to change ''
		strcpy(temp1->key,tempbuf2);//copy key
		strcpy(temp1->info,&tempbuf[++j]);//copy the next text
//		DbgPrint("key:%s\n",temp1->key);
//		DbgPrint("info:%s\n",temp1->info);
		if (*phead==NULL)
		{
			*phead = temp1;
		}
		else
			temp2->next=temp1;
		temp2=temp1;
		number++;//��¼����Ľڵ���Ŀ
		if (buffer[n]!='\0')//������һ���ַ�����'\0'��������һλ��ָ����һ��.
			n++;
	}
	if(temp2!=NULL)
	{
		temp2->next=NULL;
	}
	reg_protect_num=number;
//	DbgPrint("number %d\n",number);
	ExFreePool(buffer);
	ExFreePool(tempbuf);
	ExFreePool(tempbuf2);

	return STATUS_SUCCESS;
}

NTSTATUS InitWhiteProcessList(PWHITE_PROCESS_LIST *phead)
{
	PWHITE_PROCESS_LIST temp1=NULL,temp2=NULL;
	HANDLE file_handle=NULL ;//���ļ�ȫ�־��
	NTSTATUS status;
	OBJECT_ATTRIBUTES object_attributes;
	IO_STATUS_BLOCK io_status={0};
	LARGE_INTEGER offset={0};
	UNICODE_STRING filepath = RTL_CONSTANT_STRING(L"\\??\\c:\\whitelist.txt");
	ULONG length,n,i,j,number=0;
	char  buffer2[2];
	char *tempbuf2=ExAllocatePool(PagedPool,MAXINFOLEN);
	char  *buffer=ExAllocatePool(PagedPool,MAXDATALEN);
	char  *tempbuf=ExAllocatePool(PagedPool,MAXINFOLEN);

	RtlZeroMemory(tempbuf,MAXINFOLEN);
	RtlZeroMemory(buffer,MAXDATALEN);
	RtlZeroMemory(tempbuf2,MAXINFOLEN);
    
	length = MAXDATALEN;


	InitializeObjectAttributes(&object_attributes,&filepath,OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,NULL,NULL);
	//���ļ�
	status=ZwCreateFile(&file_handle,FILE_READ_DATA|SYNCHRONIZE,&object_attributes,&io_status,NULL,FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,FILE_OPEN_IF,FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,0);
	if (!NT_SUCCESS(status))
	{
//		DbgPrint("create file error %x\n",(ULONG)status);
		return status;
	}
	//offset.HighPart=-1;
	//offset.LowPart=FILE_USE_FILE_POINTER_POSITION;
	status = ZwReadFile(file_handle,NULL,NULL,NULL,&io_status,buffer,length,&offset,NULL);//���ļ��еĶ������뻺��
//	DbgPrint("the read length %d\n",io_status.Information);
	offset.QuadPart+=io_status.Information;
	buffer[io_status.Information]='\0';
	if(NT_SUCCESS(status))
	{
		//len = io_status.Information;

		status = ZwReadFile(file_handle,NULL,NULL,NULL,&io_status,buffer2,sizeof(buffer2),&offset,NULL);

//		DbgPrint("the read length %d\n",io_status.Information);
	}

	if (!NT_SUCCESS(status))
	{
		if (status==STATUS_END_OF_FILE)//ע�⣬��������ļ�β�Ļ�����ô����ֵ��һ������.
		{
//			DbgPrint("read all files\n");
			ZwClose(file_handle);
			status=STATUS_SUCCESS;
		}
		else
		{
//			DbgPrint("read file error\n");
			ZwClose(file_handle);
			return status;
		}
	}
   for(n=0;buffer[n]!='\0';)
	{
		i=0;
		RtlZeroMemory(tempbuf,MAXINFOLEN);
		while (buffer[n]!='\n' && buffer[n]!='\0')//����ÿһ������
		{
			tempbuf[i]=buffer[n];
			i=i+1;
			n=n+1;
		}
		
		if(buffer[n]=='\n')
		{
		   tempbuf[i-1]='\0';//use '\0' to change '\n' at the last of line
        }
		else
			tempbuf[i]='\0';
		temp1=(PWHITE_PROCESS_LIST)ExAllocatePool(PagedPool,sizeof(WHITE_PROCESS_LIST));//��������ռ�
		if(temp1==NULL)
		{
//			DbgPrint("allocate memory error");
			return status;
		}
		j=0;//propare to copy the first data
		while (tempbuf[j]!='\t')//copy the first text
		{
			tempbuf2[j]=tempbuf[j];
			j=j+1;
		}
		tempbuf2[j]='\0';//use '\0' to change ''
		strcpy(temp1->name,tempbuf2);//copy key
		strcpy(temp1->fnName,&tempbuf[++j]);//copy the next text
//		DbgPrint("name:%s\n",temp1->name);
//		DbgPrint("fnname:%s\n",temp1->fnName);
		if (*phead==NULL)
		{
			*phead = temp1;
		}
		else
			temp2->next=temp1;
		temp2=temp1;
		number++;//��¼����Ľڵ���Ŀ
		if (buffer[n]!='\0')//������һ���ַ�����'\0'��������һλ��ָ����һ��.
			n++;
	}
	if(temp2!=NULL)
	{
		temp2->next=NULL;
	}
	process_white_list=number;
//	DbgPrint("number %d\n",number);
	ExFreePool(buffer);
	ExFreePool(tempbuf);
	ExFreePool(tempbuf2);
	return STATUS_SUCCESS;
}




void FreeProtectList(PPROTECT_REGEDIT phead)
{
	
	PPROTECT_REGEDIT temp=phead;
        PPROTECT_REGEDIT before=temp;

	MUTEX_WAIT( StoreMutex );
	while(temp)
	{
		
		temp=(PPROTECT_REGEDIT)temp->next;
                ExFreePool(before);
                before=temp;
	}
	phead=NULL;
	MUTEX_RELEASE( StoreMutex );

//	DbgPrint("protect.SYS:���ͷ�regedit list\n");
}


void FreeProcessWhiteList(PWHITE_PROCESS_LIST phead)
{
	PWHITE_PROCESS_LIST temp=phead;
        PWHITE_PROCESS_LIST before=phead;
	
	MUTEX_WAIT( StoreMutex );
	while(temp)
	{
		temp=(PWHITE_PROCESS_LIST)temp->next;
                ExFreePool(before);
                before=temp;
	}
	phead=NULL;
	MUTEX_RELEASE( StoreMutex );

//	DbgPrint("protect.SYS:���ͷ�whitelist\n");

}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//��Ϣ������
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-----------------------------------------------------------------------------------------
PCHAR strncatZ( PCHAR dest, PCHAR source, int length )//�����Ӽ�
{       
	int     origlen = strlen(dest);

	strncpy( dest+origlen, source, length );
	dest[ origlen+length ] = 0;
	return(dest);
}


POBJECT GetPointer( HANDLE handle )
{
	POBJECT         pKey;

	//
	// Ignore null handles
	//
	if( !handle ) return NULL;

	//
	// Get the pointer the handle refers to
	//
	if( ObReferenceObjectByHandle( handle, 0, NULL, KernelMode, &pKey, NULL ) !=
		STATUS_SUCCESS ) 
	{

//			DbgPrint(("Error %x getting key pointer\n"));
			pKey = NULL;
	} 
	return pKey;
}


//----------------------------------------------------------------------
//
// ReleasePointer
//
// Dereferences the object.
//
//----------------------------------------------------------------------
VOID ReleasePointer( POBJECT object )
{
	if( object ) ObDereferenceObject( object );
}



//--------------------------------------------------------------------------------------
BOOLEAN GetProcessImageName(char *pImageName,HANDLE hProcess)
{
	KIRQL        CurIRQL ;
	NTSTATUS     status;
	ULONG        returnedLength;
	ULONG        bufferLength;
	PVOID        buffer;
	PUNICODE_STRING imageName;
	ANSI_STRING    tmpAnsiName;
	BOOLEAN      bRet = FALSE ;

	PAGED_CODE(); // this eliminates the possibility of the IDLE Thread/Process

	CurIRQL = KeGetCurrentIrql() ;
//	DbgPrint ("Current IRQL is %d\r\n", CurIRQL) ;
	if (PASSIVE_LEVEL != CurIRQL)
	{   
		return FALSE ;
	}

	try
	{
		if ( ! MmIsAddressValid (&tmpAnsiName))
		{
			return FALSE ;
		}

    }
	except (EXCEPTION_EXECUTE_HANDLER)
	{
		return FALSE ;
	}

	if (NULL == ZwQueryInformationProcess) {

		UNICODE_STRING routineName;

		RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");

		ZwQueryInformationProcess = 
			(QUERY_INFO_PROCESS) MmGetSystemRoutineAddress(&routineName);

		if (NULL == ZwQueryInformationProcess) {
//			DbgPrint("Cannot resolve ZwQueryInformationProcess\n");
			return FALSE ;
		}
	}
	//
	// Step one - get the size we need
	//
	status = ZwQueryInformationProcess( hProcess, 
		ProcessImageFileName,
		NULL, // buffer
		0, // buffer size
		&returnedLength);

	if (STATUS_INFO_LENGTH_MISMATCH != status) {

		return FALSE;

	}

	//
	// Is the passed-in buffer going to be big enough for us?  
	// This function returns a single contguous buffer model...
	//
	bufferLength = returnedLength - sizeof(UNICODE_STRING);

	//
	// If we get here, the buffer IS going to be big enough for us, so 
	// let's allocate some storage.
	//
	buffer = ExAllocatePoolWithTag(PagedPool, returnedLength, 'ipgD');

	if (NULL == buffer) {
		return FALSE ;        
	}

	try
	{
		//
		// Now lets go get the data
		//
		status = ZwQueryInformationProcess( hProcess, 
			ProcessImageFileName,
			buffer,
			returnedLength,
			&returnedLength);

		if (NT_SUCCESS(status)) {
			//
			// Ah, we got what we needed
			//
			imageName = (PUNICODE_STRING) buffer;

			//       RtlCopyUnicodeString(ProcessImageName, imageName);
			//RtlZeroMemory (pBuffer, cbBuffer) ;
			//if (((size_t)-1) != wcstombs (pBuffer, imageName->Buffer, cbBuffer))
			//{
			// bRet = TRUE ;
			//}

			//KdPrint (("Current ProcessImageFileName: \"%s\"\r\n", pBuffer)) ;

			if (STATUS_SUCCESS != RtlUnicodeStringToAnsiString (&tmpAnsiName, imageName, TRUE))
			{
				bRet = FALSE ;
				
//				KdPrint (("Current ProcessImageFileName: Unknow\r\n")) ;
			}
			else
			{
				strncpy(pImageName,tmpAnsiName.Buffer,tmpAnsiName.Length);
				strcat(pImageName,"\0"); 
				bRet = TRUE ;
				//KdPrint (("Current ProcessImageFileName: \"%s\"\r\n", pImageName->Buffer)) ;
			}
		}
	}
	except (EXCEPTION_EXECUTE_HANDLER)
	{
		bRet = FALSE ;
	}
	//
	// free our buffer
	//
	RtlFreeAnsiString( &tmpAnsiName );
	ExFreePool(buffer);

	//
	// And tell the caller what happened.
	//    
	return bRet ;

}


//--------------------------------------------------------------------------------------





void GetFullName( HANDLE hKey, PUNICODE_STRING lpszSubKeyVal, 
				 PCHAR fullname )
{
	NTSTATUS ns;
    PHASH_ENTRY             hashEntry;
	POBJECT                 pKey = NULL;
	CHAR                    tmpkey[16];
	ANSI_STRING             keyname;
	PCHAR                   tmpname;
	PCHAR                   nameptr;
	PUNICODE_STRING         fullUniName;
	ULONG                   actualLen;
	int                     i;
	char                   cmpname[MAXROOTLEN];


	// Allocate a temporary buffer
	//
	tmpname = ExAllocatePool( PagedPool, MAXPATHLEN );//ΪʲôҪ����ռ䣬��ջ�ռ䲻����?

	//
	// Translate the hkey into a pointer
	//
	fullname[0] = 0;
	tmpname[0] = 0;

	//
	// Is it a valid handle?
	if( pKey = GetPointer( hKey )) 
	{
		ReleasePointer( pKey );
		MUTEX_WAIT( HashMutex );
		hashEntry = HashTable[ HASHOBJECT( pKey ) ];
		while( hashEntry && hashEntry->Object != pKey )
			hashEntry = hashEntry->Next;
		MUTEX_RELEASE( HashMutex );

		if( hashEntry ) 
		{
			strcpy( tmpname, hashEntry->FullPathName );
		}  
		else 
		{

		// See if we find the key in the hash table
	
			if( pKey )
			{

				fullUniName = ExAllocatePool( PagedPool, MAXPATHLEN*2+2*sizeof(ULONG));
				fullUniName->MaximumLength = MAXPATHLEN*2;
				ns=ObQueryNameString( pKey, (POBJECT_NAME_INFORMATION)fullUniName, MAXPATHLEN, &actualLen ); 

				if( NT_SUCCESS(ns))
				{

					RtlUnicodeStringToAnsiString( &keyname, fullUniName, TRUE ); 
					if( keyname.Buffer[0] )
					{         
						strcpy( tmpname, "\\" );
						strncatZ( tmpname, keyname.Buffer, MAXPATHLEN -2 );
					}
					RtlFreeAnsiString( &keyname );
				}
				ExFreePool( fullUniName );
			}
		}
	}


	//
	// Append subkey and value, if they are there
	//
	if( lpszSubKeyVal ) {
		RtlUnicodeStringToAnsiString( &keyname, lpszSubKeyVal, TRUE );
		if( keyname.Buffer[0] ) {
			strcat( tmpname, "\\" );
			strncatZ( tmpname, keyname.Buffer, MAXPATHLEN - 1 - strlen(tmpname) );
		}
		RtlFreeAnsiString( &keyname );
	}


	for( i = 0; i < 2; i++ )
	{
		ConvertToUpper( cmpname, tmpname, CurrentUser[i].RootNameLen );//��β��û��0���ƹ�ȥ
		if( !strncmp( cmpname, CurrentUser[i].RootName,
			CurrentUser[i].RootNameLen ))
		{

//				DbgPrint(( " CurrentUser(%d) %s ==> %s\n", i, tmpname, CurrentUser[i].RootName ));

				//
				// Its current user. Process to next slash
				//
				nameptr = tmpname + CurrentUser[i].RootNameLen;
				while( *nameptr && *nameptr != '\\' ) nameptr++;//nameptrָ�����������Ӽ�
				strcpy( fullname, CurrentUser[i].RootShort );//��������д���ƹ�ȥ
				strcat( fullname, nameptr );
				ExFreePool( tmpname );
				return;
		}
	}     

	//
	// Now, see if we can translate a root key name
	//
	for( i = 0; i < NUMROOTKEYS; i++ ) {
		ConvertToUpper( cmpname, tmpname, RootKey[i].RootNameLen );
		if( !strncmp( cmpname, RootKey[i].RootName, 
			RootKey[i].RootNameLen )) {
				nameptr = tmpname + RootKey[i].RootNameLen;
				strcpy( fullname, RootKey[i].RootShort );
				strcat( fullname, nameptr );
				ExFreePool( tmpname );
				return;
		}
	}



	
	strcpy( fullname, tmpname );
	ExFreePool( tmpname ); 
}


ULONG GetProcessNameOffset()
{
	PEPROCESS       curproc;
	int             i;

	curproc = PsGetCurrentProcess();

	//
	// Scan for 12KB, hopping the KPEB never grows that big!
	//
	for( i = 0; i < 3*PAGE_SIZE; i++ ) {

		if( !strncmp( SYSNAME, (PCHAR) curproc + i, strlen(SYSNAME) )) {

			return i;
		}
	}

	//
	// Name not found - oh, well
	//
	return 0;
}

DWORD   GetCurrentProcessId()
{
	PEPROCESS       curproc;
	DWORD           pid=0;
	curproc = PsGetCurrentProcess();
	pid = *(DWORD *)((DWORD)curproc + 0x084);
	return pid;
}



PCHAR GetCurrentProcessName( PCHAR Name )
{
	PEPROCESS       curproc;
	char            *nameptr;
	ULONG           i;

	//
	// We only try and get the name if we located the name offset
	//
	if( ProcessNameOffset ) {

		curproc = PsGetCurrentProcess();
		nameptr   = (PCHAR) curproc + ProcessNameOffset;
		strncpy( Name, nameptr, 16 );

	} else {

		strcpy( Name, "???");
	}

	 return NULL;

}


void  GetFilePathName(HANDLE FileHandle,char *name)
{
	NTSTATUS                      ns;
	ANSI_STRING                   ObjectName;
	POBJECT                          pObject;
	POBJECT_ATTRIBUTES            pFileObj;
	ULONG                         ret;
	PUNICODE_STRING               pFullPath=NULL;
	ns=ObReferenceObjectByHandle(FileHandle,GENERIC_READ,NULL,KernelMode,&pObject,0);
	if(!NT_SUCCESS(ns))
	{
//		DbgPrint("get fileobject error\n");
		return;
	}
	
	pFileObj=(POBJECT_ATTRIBUTES)pObject;
	pFullPath = (PUNICODE_STRING)ExAllocatePool(NonPagedPool,MAXFILEPATHLEN);
	RtlZeroMemory(pFullPath,MAXFILEPATHLEN);
	pFullPath->MaximumLength = MAXFILEPATHLEN;
	ns=ObQueryNameString(pFileObj,(POBJECT_NAME_INFORMATION)pFullPath,MAXFILEPATHLEN,&ret);
	ObDereferenceObject(pObject);
    RtlUnicodeStringToAnsiString(&ObjectName,pFullPath,TRUE);


	//RtlUnicodeStringToAnsiString(&ObjectName,(PUNICODE_STRING)pFileObj->ObjectName,TRUE);
    if(ObjectName.Length==0)
	{
//		DbgPrint("objectName error\n");
		return;
	}
            
	
	strncpy(name,ObjectName.Buffer,ObjectName.Length);
	name[ObjectName.Length]='\0';
//	DbgPrint("the file name is %s\n",name);
	RtlFreeAnsiString(&ObjectName);
}

	

NTSTATUS   GetFullProcessName(HANDLE     KeyHandle,char   *fullname)   
{   
	NTSTATUS   ns;   
	PVOID   pKey=NULL,pFile=NULL;   
	UNICODE_STRING                   fullUniName;   
	ANSI_STRING                           akeyname;   
	ULONG   actualLen;   
	UNICODE_STRING   dosName;   

	fullUniName.Buffer=NULL;   
	fullUniName.Length=0;   
	fullname[0]=0x00;   
	ns=   ObReferenceObjectByHandle(KeyHandle,   0,   NULL,   KernelMode,   &pKey,   NULL);   
	if(   !NT_SUCCESS(ns))   return   ns;   

	fullUniName.Buffer   =   ExAllocatePool(   PagedPool,   MAXPATHLEN*2);//1024*2   
	fullUniName.MaximumLength   =   MAXPATHLEN*2;   

	__try   
	{   

		pFile=(PVOID)*(ULONG   *)((char   *)pKey+20);   
		pFile=(PVOID)*(ULONG   *)((char   *)pFile);   
		pFile=(PVOID)*(ULONG   *)((char   *)pFile+36);//���յõ�����һ��file_object�ṹ   


		ObReferenceObjectByPointer(pFile,   0,   NULL,   KernelMode);   
		RtlVolumeDeviceToDosName(((PFILE_OBJECT)pFile)->DeviceObject,&dosName);   
		//ns=ObQueryNameString(   pFile,   fullUniName,   MAXPATHLEN,   &actualLen   );   
		RtlCopyUnicodeString(&fullUniName,   &dosName);   
		RtlAppendUnicodeStringToString(&fullUniName,&((PFILE_OBJECT)pFile)->FileName);   

		ObDereferenceObject(pFile);   
		ObDereferenceObject(pKey   );   

		RtlUnicodeStringToAnsiString(   &akeyname,   &fullUniName,   TRUE   );   
		if(akeyname.Length<MAXPATHLEN)     
		{   
			memcpy(fullname,akeyname.Buffer,akeyname.Length);   
			fullname[akeyname.Length]=0x00;   
		}   
		else   
		{   
			memcpy(fullname,akeyname.Buffer,MAXPATHLEN);   
			fullname[MAXPATHLEN-1]=0x00;   
		}   

		RtlFreeAnsiString(   &akeyname   );   
		ExFreePool(dosName.Buffer);   
		ExFreePool(   fullUniName.Buffer   );   

		return   STATUS_SUCCESS;   

	}   

	__except(1)   
	{   
		if(fullUniName.Buffer)   ExFreePool(   fullUniName.Buffer     );   
		if(pKey)   ObDereferenceObject(pKey  );   
		return   STATUS_SUCCESS;   

	}   

} 


VOID ConvertToUpper( PCHAR Dest, PCHAR Source, ULONG Len )
{
	ULONG   i;

	for( i = 0; i < Len; i++ ) {
		if( Source[i] >= 'a' && Source[i] <= 'z' ) {

			Dest[i] = Source[i] - 'a' + 'A';

		} else {

			Dest[i] = Source[i];
		}
	}
}

//��Σ�����ݴ����û������ȴ��û�ѡ��
DWORD waituser(char *userData)	
{

	DWORD choice;

	KeWaitForSingleObject(&event,Executive,KernelMode,0,0);
	
	//output���û�����Ļ��壬����Ϣ������û�
	strncpy(&output[0],userData,strlen(userData)+1);

	//���û��߳�����
	if(gpEventObjectSend)
	{
		KeSetEvent(gpEventObjectSend,0,FALSE);
//		DbgPrint("���û��߳�����!\n");
	}
	//�ȴ��û�ѡ��
	if(gpEventObjectReceive)
	{
//		DbgPrint("��ʼ�ȴ�!\n");
		KeWaitForSingleObject(gpEventObjectReceive,Executive,KernelMode,0,0);
//		DbgPrint("��ɵȴ�!\n");
	}

	memmove(&choice,&output[0],4);//������û��Ĳ���
//	DbgPrint("the choice is:%d\n",choice);
	InsertList(userData);  //��ʱ������
	KeSetEvent(&event,0,FALSE);
	
	return choice;
}

void InsertList(char *data)
{

	PENTRY          Entry;
	ULONG           len ;

	MUTEX_WAIT( StoreMutex );
	InterlockedIncrement( &Sequence );
	len = strlen(data);
	len += 1; 


	if ( Store->Len + len + sizeof(ENTRY) + 1 >= MAX_STORE ) 
	{

		RegmonNewStore();//
	}
	Entry = (void *)(Store->Data+Store->Len);//
	Entry->seq = Sequence;
	memcpy( Entry->text, data, len);
	Store->Len += sizeof(Entry->seq)+len;

//	DbgPrint("insert ok\n");

	MUTEX_RELEASE( StoreMutex );
}
//--------------------------------------------------------------------------------
DWORD DoLoadDriver(char *pdata)//Ҳ�����и�������.
{
	DWORD ret;
	ret = waituser(pdata);
	return ret;

}

DWORD DoMemory(char *pdata)
{

	DWORD ret;
	int num=0,i=0,len,begin=0;
	char tmp[MAXPROCNAMELEN],dest[MAXPROCNAMELEN];
	char functionName[FUNCTIONNAME];

	PWHITE_PROCESS_LIST current_whiteps =gpWhiteList;
	RtlZeroMemory(dest,MAXPROCNAMELEN);
	RtlZeroMemory(tmp,MAXPROCNAMELEN);

	RtlZeroMemory(functionName,FUNCTIONNAME);
	while ((pdata[i]!='!')) i++;

	strncpy(tmp,pdata,i);//i��ֻ�ܸ���i-1�������Բ��ܼ�����Ϊ�Ǵ�0��ʼ��
	tmp[i]='\0';
	len = strlen(tmp);
	ConvertToUpper(dest,tmp,len);//dest is the create process name
	dest[len]='\0';
	//find the function name
    i+=3;
//	DbgPrint("i is:%d\n",i);
	while(pdata[i]!='\t')
	{
		functionName[begin++]=pdata[i++];
	}
	functionName[begin]='\0';
//	DbgPrint("fnname:%s\n",functionName);
	
	



//	DbgPrint("manage the process\n");


	while (current_whiteps!=NULL)
	{
		
		if(strstr(dest,current_whiteps->name))//�ҵ�,˵���Ǵ��ڰ�������.
		{
//			DbgPrint("the cyrrent whitelist name is:%s\n",current_whiteps->name);
//			DbgPrint("the current function name is:%s\n",current_whiteps->fnName);
			//�ж��Ƿ���Ҫ���еĶ�����
			if((_stricmp(current_whiteps->fnName,"all")==0) || (_stricmp(functionName,current_whiteps->fnName)==0))
			{
//				DbgPrint("the white protect process\n");
			    break;
			}
		}

		num++;
		current_whiteps=current_whiteps->next;
	}
	if (num==process_white_list)
	{
		ret =waituser(pdata);
		//˵�����Ǳ��������г��������Ľ��̣��ȴ��û�����
	}
	else
	{
		ret =1;
		InsertList(pdata);
	}
//	DbgPrint("do process\n");
	return ret;

}



//---------------------------------------------------------------------------------
DWORD DoProcess(char *pdata)
{
	DWORD ret;
	int num=0,i=0,begin=0,end =0;
	ULONG  length;
	char tmp[MAXPROCNAMELEN],dest[MAXPROCNAMELEN];
	char functionName[FUNCTIONNAME];
	char pathname[MAXPROCNAMELEN];
	ULONG  flag=0;

	PWHITE_PROCESS_LIST current_whiteps =gpWhiteList;
	RtlZeroMemory(dest,MAXPROCNAMELEN);
	RtlZeroMemory(tmp,MAXPROCNAMELEN);
	RtlZeroMemory(pathname,MAXPROCNAMELEN);

	RtlZeroMemory(functionName,FUNCTIONNAME);
	while (pdata[i]!='#') i++;

	strncpy(tmp,pdata,i);//i��ֻ�ܸ���i-1�������Բ��ܼ�����Ϊ�Ǵ�0��ʼ��
	tmp[i]='\0';
	length = strlen(tmp);
	
	
    
	ConvertToUpper(dest,tmp,length);//dest is the create process name
	dest[length]='\0';
	//find the function name
    i+=3;
//	DbgPrint("i is:%d\n",i);
	while(pdata[i]!='\t')
	{
		functionName[begin++]=pdata[i++];
	}
	functionName[begin]='\0';
	if(_stricmp(functionName,"NtUserSetWindowsHookEx")!=0)//����Ĳ���ֻ��Դ�������
	{
		flag=1;

	while(pdata[i]!='\0')  i++;
	i--;
	end=i;
	DbgPrint("the last length is:%d\n",i);

	while(pdata[i]!='\\')  i--;
	num=end-i;
	DbgPrint("the length is:%d\n",num);
	i++;
	RtlZeroMemory(tmp,MAXPROCNAMELEN);
	strncpy(tmp,(char *)&pdata[i],num);//ȡ���ĳ�����
    ConvertToUpper(pathname,tmp,num);
	}
	num=0;
	DbgPrint("pathname:%s\n",pathname);

    DbgPrint("dest:%s\n",dest);

	while (current_whiteps!=NULL)
	{
		
		if(strstr(dest,current_whiteps->name))//�ҵ�,˵���Ǵ��ڰ�������.
		{
//			DbgPrint("the cyrrent whitelist name is:%s\n",current_whiteps->name);
//			DbgPrint("the current function name is:%s\n",current_whiteps->fnName);
			//�ж��Ƿ���Ҫ���еĶ�����
			if((_stricmp(current_whiteps->fnName,"all")==0)|| (_stricmp(functionName,current_whiteps->fnName)==0))
			{
				if(flag==1)
				{

				    if((_stricmp(dest,pathname)==0)||(strstr(dest,"EXPLORER")))
						break;
				}
				else if(flag==0)
//				DbgPrint("the white protect process\n");
			    break;
			}

		}
		

		num++;
		current_whiteps=current_whiteps->next;
	}
	DbgPrint("the num is:%d\n",num);
	DbgPrint("the white num is:%d\n",process_white_list);
	if (num==process_white_list)
	{
		ret =waituser(pdata);
		//˵�����Ǳ��������г��������Ľ��̣��ȴ��û�����
	}
	else
	{
		ret =1;
		InsertList(pdata);
	}
	return ret;

}

//����ע����������
DWORD  DoRegedit(char *pdata)//�ȽϺ͹���
{
	DWORD ret,len=0;
	int num=0,i=0;
	char tmp[MAXPATHLEN],dest[MAXPATHLEN];

	
	PPROTECT_REGEDIT current_regprotect =gpRegeditProtect;//����������
	RtlZeroMemory(dest,MAXPATHLEN);
	RtlZeroMemory(tmp,MAXPATHLEN);
	

	while ((pdata[i]!='\t')&&pdata[i]!='\0') i++;
	i++;
	while ((pdata[i]!='\t')&&pdata[i]!='\0') i++;
	i++;
	strcpy(tmp,&pdata[i]);//��β��0����
	len = strlen(tmp);
	ConvertToUpper(dest,tmp,len);
	dest[len]='\0';
	len--;
	while(dest[len]!='\\') len--;
	dest[len]='\0';


	while (current_regprotect!=NULL)
	{
		if(!strcmp(dest,current_regprotect->key))
		{
			break;
		}
		num++;
		current_regprotect=current_regprotect->next;
	}
	if (num==reg_protect_num)
	{
		InsertList(pdata);//�����ݲ��뵽��ʾ������//���ܱ����Ĳ���ʾ,Ҳ������
		ret = 1;
	}
	else
		ret =waituser(pdata);//���û������Ƿ������޸��ܱ����ļ�
	
	return ret;
}

//---------------------------------------------------------------------------------
DWORD manage(char *pdata,int belong)
{
	
	DWORD psresult=-1;
	// KEVENT  copyevent;
	//KeInitializeEvent(&copyevent,SynchronizationEvent,TRUE);//����ͬ��
	//char filter1[]="$$";//operate the regedit
	//char filter2[]="##";//operate  the process
	//char filter3[]="**";//operate  the driver
	//char filter4[]="!!";//operate  the memory
//	DbgPrint("protect.SYS:data%d:%s\n",TOTALCOUNT++,pdata);

	switch(belong)
	{
	case 0:
		psresult=DoRegedit(pdata);
		break;
	case 1:
		psresult =DoProcess(pdata);
		break;
	case 2:
		psresult = DoLoadDriver(pdata);
		break;
	case 3:
		psresult = DoMemory(pdata);
		break;
	default:
		break;
	}

	

	return psresult;
}

void  GetProcessPath(HANDLE thread,char *Name)
{
	PETHREAD     ethread;
	PEPROCESS    pProcess;
	char         *nameptr;
	ULONG           i;
	PsLookupThreadByThreadId(thread,&ethread);
	pProcess=IoThreadToProcess(ethread);//�õ�PEP
	if(pProcess==NULL)
	{
//		DbgPrint("get pep error\n");
		strcpy(Name,"???");
	}


	//
	// We only try and get the name if we located the name offset
	//
	if( ProcessNameOffset ) {

		
		nameptr   = (PCHAR) pProcess + ProcessNameOffset;
		strncpy( Name, nameptr, 16 );

	} 
	else {

		strcpy( Name, "???");
	}
}






//��סZwTerminateProcess��ֹ��������GUI����
NTSTATUS  FakedZwTerminateProcess(IN HANDLE ProcessHandle OPTIONAL,
								  IN NTSTATUS ExitStatus)
{
	NTSTATUS ntstatus;
	DWORD eproc=0x00000000;

    ntstatus = ObReferenceObjectByHandle(ProcessHandle,0x0001,NULL,KernelMode,(PVOID*)&eproc,NULL);
    if(NT_SUCCESS(ntstatus))
	{
//		DbgPrint("*********************************************\n");
//		DbgPrint("%d",*((DWORD*)(eproc+0x084)));
//		DbgPrint("\n*********************************************\n");
		//if (_stricmp((char*)((char*)Process+0x174), ProtectName) == 0 )
		//�ж��ǲ�������Ҫ�����Ľ���
		if(protectedPid==*((DWORD*)(eproc+0x084)))
		{
			ObDereferenceObject((PVOID)eproc); 
			return STATUS_UNSUCCESSFUL;
		}
		ObDereferenceObject((PVOID)eproc);
	}

	return  RealZwTerminateProcess(ProcessHandle,ExitStatus);

}



//OpenProcess:��ý��̾��
NTSTATUS  FakedZwOpenProcess(OUT PHANDLE ProcessHandle,
							 IN  ACCESS_MASK DesiredAccess,
							 IN  POBJECT_ATTRIBUTES ObjectAttributes,//define the target object tobe open
							 IN  PCLIENT_ID  ClientID   OPTIONAL)
{
	NTSTATUS           ntstatus;
	PEPROCESS process_to_kill,EProcess;
	DWORD host,guest;

    host=GetCurrentProcessId();
	guest=*(DWORD *) ClientID;
//	DbgPrint("����ID:%d\t%d\n",host,guest);

	if(host!=guest)
	{
		//�ܱ�����������̾�����������������
		if(guest==protectedPid)
		{
//			DbgPrint("�ܱ�����������̾�����������������\n");
			return STATUS_ACCESS_DENIED;
		}
	}


	/*PsLookupProcessByProcessId( *(ULONG *) ClientID, &EProcess);//�õ�Ҫ�򿪽��̵�PEPROCESS
		if ((DesiredAccess!= 0x401 ) && (DesiredAccess != 0x400 ) &&
			PsGetCurrentProcess() != EProcess)//�򿪵Ĳ��ǵ�ǰ���� 
			return STATUS_ACCESS_DENIED;*/

	return RealZwOpenProcess(ProcessHandle,DesiredAccess,ObjectAttributes,ClientID);
	
}


//���ݽ���ID��������
NTSTATUS TerminateProcessByID( DWORD pid)
{
	NTSTATUS status;

	CLIENT_ID cid1 =
	{
		(HANDLE)pid,  
		(HANDLE)0
	};

	OBJECT_ATTRIBUTES attr =  
	{ 
		sizeof ( OBJECT_ATTRIBUTES), 
		0, 
		NULL, 
		0,
		NULL,
		NULL
	};

	HANDLE hProcess = 0;
	status = ZwOpenProcess(&hProcess, 1, &attr, &cid1);
	if (status == STATUS_SUCCESS)
	{
		status = RealZwTerminateProcess(hProcess, 0); 
		ZwClose(hProcess);
	}
//	if(status == STATUS_SUCCESS)
//		DbgPrint("protect.SYS:�ɹ���������%d\n", pid);
//	else
//		DbgPrint("protect.SYS:�޷���������%d\n", pid);
	return status;
}	


//���ӽ��̴���/�ļ���
NTSTATUS   FakedZwCreateSection(
								OUT PHANDLE SectionHandle,
                                IN ACCESS_MASK DesiredAccess,
                                IN POBJECT_ATTRIBUTES ObjectAttributes,
                                IN PLARGE_INTEGER SectionSize OPTIONAL,
                                IN ULONG Protect,
                                IN ULONG Attributes,
                                IN HANDLE FileHandle)
{
	NTSTATUS    ntstatus;
	DWORD  mark;
	DWORD  pid=-1;

	HANDLE hHandle;
    PFILE_OBJECT pFileObject;
    OBJECT_HANDLE_INFORMATION HandleInformationObject;

	UNICODE_STRING   dosName, fullUniName; 
    ANSI_STRING asFileName;
    char PathName[MAXPATHLEN];
    char ProcessName[MAXPROCNAMELEN];
	char data[MAXDATALEN];
		
    char* findpointer=NULL;


	RtlZeroMemory(ProcessName,MAXPROCNAMELEN);
	RtlZeroMemory(PathName,MAXPATHLEN);
	RtlZeroMemory(data,MAXDATALEN);


	GetCurrentProcessName(ProcessName);
	pid  = GetCurrentProcessId();
	
//	DbgPrint("current pid:%d\n",pid);
//	DbgPrint("\n\n\n");
//	DbgPrint(ProcessName);
//	DbgPrint("\n\n\n");


	//�ܱ�������������������	
	if(pid==protectedPid)
	{
//		DbgPrint("���ü������!\n");
		return RealZwCreateSection(SectionHandle, 
                                 DesiredAccess, 
                                 ObjectAttributes, 
                                 SectionSize, 
                                 Protect, 
                                 Attributes, 
                                 FileHandle);
	}



	dosName.Buffer=(PWSTR)ExAllocatePool(PagedPool, 16);
    dosName.MaximumLength=8;

	fullUniName.Buffer = (PWSTR)ExAllocatePool(PagedPool, MAXPATHLEN*2);//1024*2   
    fullUniName.MaximumLength   =   MAXPATHLEN*2; 

	


	hHandle=(HANDLE)FileHandle;
    ObReferenceObjectByHandle(hHandle,0,0,KernelMode,&pFileObject,&HandleInformationObject);
    if(pFileObject == NULL)
	{
//		DbgPrint("error fileobject\n");
		return RealZwCreateSection(SectionHandle, 
                                 DesiredAccess, 
                                 ObjectAttributes, 
                                 SectionSize, 
                                 Protect, 
                                 Attributes, 
                                 FileHandle);
	}

//	GetProcessImageName(PathName,hHandle);

	RtlVolumeDeviceToDosName(pFileObject->DeviceObject, &dosName); 
	RtlCopyUnicodeString(&fullUniName, &dosName);  
	RtlAppendUnicodeStringToString(&fullUniName,&((PFILE_OBJECT)pFileObject)->FileName);
	RtlUnicodeStringToAnsiString(&asFileName,&(fullUniName),TRUE); 

//	DbgPrint("the path is:%s\n",asFileName.Buffer);
	
	if((findpointer=strstr(asFileName.Buffer, "."))==NULL)
//	if((findpointer=strstr(PathName, "."))==NULL)
	{
		return   RealZwCreateSection(SectionHandle, 
                                     DesiredAccess, 
                                     ObjectAttributes, 
                                     SectionSize, 
                                     Protect, 
                                     Attributes, 
                                     FileHandle);
	}
	
	if (_stricmp(findpointer, ".exe") != 0) 
	{
//		DbgPrint("not a exe file\n");
		return  RealZwCreateSection(SectionHandle, 
                                    DesiredAccess, 
                                    ObjectAttributes, 
                                    SectionSize, 
                                    Protect, 
                                    Attributes, 
                                    FileHandle);
	}
	
	strcpy(PathName,asFileName.Buffer);
	RtlFreeAnsiString(&asFileName);
	
	sprintf(data,"%s##\tZwCreateSection\t%s",ProcessName,PathName);
	mark = manage(data,1);
	
	if (mark==1)//ִ��һ��
	{
		ntstatus=RealZwCreateSection(SectionHandle, 
                                     DesiredAccess, 
                                     ObjectAttributes, 
                                     SectionSize, 
                                     Protect, 
                                     Attributes, 
                                     FileHandle);
	}
	else if ((mark==2)||(mark==3))
	{
//		DbgPrint("process mark==2\n");
		ntstatus= STATUS_ACCESS_DENIED;//�û�ѡ���˾ܾ�ִ��		
	}
	else if(mark==4)//���������
	{
		//if(!addWhiteList(gpDeviceContext->HookCalls[i]))
		//	DbgPrint("write file error");
		ntstatus =RealZwCreateSection(SectionHandle, 
                                      DesiredAccess, 
                                      ObjectAttributes, 
                                      SectionSize, 
                                      Protect, 
                                      Attributes, 
                                      FileHandle);

	}

	return  ntstatus;
}




NTSTATUS FakedZwCreateKey( OUT PHANDLE pHandle, IN ACCESS_MASK ReqAccess,
						  IN POBJECT_ATTRIBUTES pOpenInfo, IN ULONG TitleIndex,
						  IN PUNICODE_STRING Class, IN ULONG CreateOptions, OUT PULONG Disposition )
{
	NTSTATUS                ntstatus;
	DWORD                   retvalue;
	POBJECT                 regobj;
	CHAR                    fullname[MAXPATHLEN], data[MAXDATALEN], name[MAXPROCNAMELEN];
	ULONG                    pID;


	GetCurrentProcessName(name);
	if (strstr(name,"anti"))//�����������
	{
		ntstatus = RealZwCreateKey(pHandle, ReqAccess, pOpenInfo,TitleIndex, Class,CreateOptions,Disposition);
		return ntstatus;

	}
	GetFullName( pOpenInfo->RootDirectory, pOpenInfo->ObjectName, fullname );
	pID  = GetCurrentProcessId();

	//DbgPrint(("RegCreateKey: %s => %x, %x\n", fullname, *pHandle, ntstatus ));  
	regobj = GetPointer( *pHandle );
	RegmonFreeHashEntry( regobj );
	RegmonStoreHash( regobj, fullname );
	ReleasePointer( regobj );
	
	sprintf(data,"%s$$\tZwCreateKey\t%s", name,fullname);
	retvalue=manage(data,0);
	if ((retvalue==1)||(retvalue==4))
	{
		ntstatus = RealZwCreateKey(pHandle, ReqAccess, pOpenInfo,TitleIndex, Class,CreateOptions,Disposition);
	}
	else if (retvalue==2)
	{
		ntstatus=STATUS_ACCESS_DENIED;
	}
	else  if(retvalue==3)
	{
		ntstatus=TerminateProcessByID(pID);
	}



	return ntstatus;

}








NTSTATUS FakedZwDeleteKey( IN HANDLE Handle )
{
	NTSTATUS                ntstatus;
	POBJECT                 regobj;
	DWORD                   retvalue;
	CHAR                    fullname[MAXPATHLEN], name[MAXPROCNAMELEN],data[MAXDATALEN];
    ULONG                   pid;
	
	GetCurrentProcessName(name);
	pid  = GetCurrentProcessId();

	//�ܱ�������������������
	if(pid==protectedPid)
	{
		return RealZwDeleteKey( Handle );
	}


	GetFullName( Handle, NULL, fullname );//��������fullname����.
	
	regobj = GetPointer( Handle );
	ReleasePointer( regobj );
	if( regobj ) RegmonFreeHashEntry( regobj );

	sprintf(data,"%s$$\tZwDeleteKey\t%s", name,fullname);
	retvalue=manage(data,0);
	if ((retvalue==1)||(retvalue==4))
	{
		ntstatus = RealZwDeleteKey( Handle );
	}
	else if (retvalue==2)
	{
		ntstatus=STATUS_ACCESS_DENIED;
	}
	else  if(retvalue==3)
	{
		ntstatus=TerminateProcessByID(pid);
	}

	return  ntstatus;


}



NTSTATUS FakedZwDeleteValueKey( IN HANDLE Handle, PUNICODE_STRING Name )
{
	NTSTATUS                ntstatus;
	DWORD                   retvalue;
	CHAR                    fullname[MAXPATHLEN], name[MAXPROCNAMELEN],data[MAXDATALEN];
	ULONG                   pid;

	GetCurrentProcessName(name);
	pid  = GetCurrentProcessId();

	//�ܱ�������������������
	if(pid==protectedPid)
	{
		return RealZwDeleteKey( Handle );
	}

	
	GetFullName( Handle, Name, fullname );

	
	sprintf(data,"%s$$\tZwDeleteValueKey\t%s", name,fullname);
	retvalue=manage(data,0);
	if ((retvalue==1)||(retvalue==4))
	{
		ntstatus = RealZwDeleteValueKey( Handle,Name );
	}
	else if (retvalue==2)
	{
		ntstatus=STATUS_ACCESS_DENIED;
	}
    else  if(retvalue==3)
	{
		ntstatus=TerminateProcessByID(pid);
	}

	return  ntstatus;

}


//SetValueKey:���ü�ֵ
NTSTATUS FakedZwSetValueKey( IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName,
							IN ULONG TitleIndex, IN ULONG Type, IN PVOID Data, IN ULONG DataSize )
{
	NTSTATUS                ntstatus;
	PUNICODE_STRING         valueName;
	DWORD                   retvalue;
	CHAR                    fullname[MAXPATHLEN], data[MAXDATALEN], name[MAXPROCNAMELEN ];
	ULONG                   pid;

	GetCurrentProcessName(name);
	pid  = GetCurrentProcessId();

	//�ܱ�������������������
	if(pid==protectedPid)
	{
		return RealZwSetValueKey( KeyHandle, ValueName, TitleIndex,Type, Data, DataSize );
	}


	if( !ValueName || !ValueName->Length )
		valueName = &DefaultValue;
	else                           
        valueName = ValueName;

	GetFullName( KeyHandle,valueName, fullname );

	sprintf(data,"%s$$\tZwSetValueKey\t%s", name,fullname);
	retvalue=manage(data,0);
	if ((retvalue==1)||(retvalue==4))
	{
		ntstatus = RealZwSetValueKey( KeyHandle, ValueName, TitleIndex,Type, Data, DataSize );
	}
	else if (retvalue==2)
	{
		ntstatus=STATUS_ACCESS_DENIED;
	}
	else  if(retvalue==3)
	{
		ntstatus=TerminateProcessByID(pid);
	}

	return ntstatus;
}


//WriteVirtualMemory:д�ڴ溯��
NTSTATUS FakedZwWriteVirtualMemory(
					 IN HANDLE ProcessHandle,
					 IN PVOID BaseAddress,
					 IN PVOID Buffer,
					 IN ULONG BufferLength,
					 OUT PULONG ReturnLength OPTIONAL)
{
	DWORD                   mark;
	DWORD pid=-1;
	char            aProcessName[MAXPROCNAMELEN];

    char            aPathName[MAXPROCNAME]={0};
	char            data[MAXPROCDATA];
	
	NTSTATUS                ntstatus=STATUS_SUCCESS;

	GetCurrentProcessName(aProcessName);
    pid  = GetCurrentProcessId();

	//�ܱ�������������������
	if(pid==protectedPid)
	{
		return RealZwWriteVirtualMemory(ProcessHandle,
			                            BaseAddress,
		                            	Buffer,
	                            		BufferLength,
	                              		ReturnLength );
	}


	


	GetProcessImageName(aPathName,ProcessHandle);

//	DbgPrint("ZwWriteVirtualMemory is called by %s\n",aProcessName);
	sprintf(data,"%s!!\tZwWriteVirtualMemory\t%s", aProcessName,aPathName);
    mark=manage(data,3);
	
	if ((mark==1)||(mark==4))
	{
		ntstatus = RealZwWriteVirtualMemory(
			                          ProcessHandle,
		                              BaseAddress,
			                           Buffer,
			                          BufferLength,
			                          ReturnLength );
	}
	else if (mark==2)
	{
		ntstatus= STATUS_ACCESS_DENIED;//�û�ѡ���˾ܾ�ִ��
	}
	else if (mark==3)
	{
		ntstatus=TerminateProcessByID(pid);
	}
//	DbgPrint("protect.SYS:Error WriteVirtualMemory %d\n",mark);


	return ntstatus;

}


//LoadDriver����������
NTSTATUS FakedZwLoadDriver(IN PUNICODE_STRING DriverServiceName )
{
	NTSTATUS                ntstatus;	

	char                    aProcessName[ MAXPROCNAMELEN];
	char                    aDrvname[MAXPROCNAME];
    ANSI_STRING              ansi ;
	char                       data[MAXPROCDATA];
	
	DWORD                   mark;
	DWORD                    pid;

	pid=GetCurrentProcessId();

	//���Լ���Ӧ�ó����������
	if(pid==protectedPid)
	{
		return RealZwLoadDriver(DriverServiceName);
	}

	GetCurrentProcessName(aProcessName);
/*	if (strstr(aProcessName,"services.exe"))//�����������
	{
		return RealZwLoadDriver(DriverServiceName);
	}*/

	RtlUnicodeStringToAnsiString(&ansi,DriverServiceName,TRUE);
	if(ansi.Length<MAXPROCNAME)     
	{   
		memcpy(aDrvname,ansi.Buffer,ansi.Length);   
		aDrvname[ansi.Length]=0x00;   
	}   
	else   
	{   
		memcpy(aDrvname,ansi.Buffer,MAXPATHLEN);   
		aDrvname[MAXPATHLEN-1]='\0';   
	}   

	RtlFreeAnsiString(   &ansi   );  
//	DbgPrint("ZwLoadDriver is called by %s\n",aProcessName);
//	DbgPrint("Driver name is %s\n",aDrvname);
	
    sprintf(data,"%s**\tZwLoadDriver\t%s", aProcessName,aDrvname);
	mark=manage(data,2);
//	DbgPrint("the driver is :%d\n",mark);
	if ((mark==1)||(mark==4))
	{
		ntstatus =  RealZwLoadDriver(DriverServiceName);
	}
	else if (mark==2)
	{
		ntstatus= STATUS_ACCESS_DENIED;//�û�ѡ���˾ܾ�ִ��
	}
	else if(mark==3)
	{
		ntstatus=TerminateProcessByID(pid);
	}

	//DbgPrint("�Ƿ�ѡ��ֵ! %d\n",mark);
	return ntstatus;

}





HHOOK   FakedNtUserSetWindowsHookEx(
									HINSTANCE Mod,
									PUNICODE_STRING UnsafeModuleName,
									DWORD ThreadId,
									int HookId,
									FARPROC HookProc,
									BOOL Ansi)
{

	char                       procname[MAXPROCNAMELEN];
	char                       data[MAXPROCDATA];
    char                       targetPath[128];
	DWORD                      retvalue;
	ULONG                      pID;
	RtlZeroMemory(procname,MAXPROCNAMELEN);
	RtlZeroMemory(data,MAXPROCDATA);
	RtlZeroMemory(targetPath,128);
	GetCurrentProcessName(procname);

//	DbgPrint("the called proc name is:%s\n",procname);

	pID  = GetCurrentProcessId();
	if(pID==protectedPid || shadow_ssdt_hook==1)
	{
		return RealNtUserSetWindowsHookEx(
			Mod,
			UnsafeModuleName,
			ThreadId,
			HookId,
			HookProc,
			Ansi);
	}
	if(ThreadId==0)
	{
		strcpy(targetPath,"ȫ�ֹ���");
	}
	else
	{
		GetProcessPath((HANDLE)ThreadId,targetPath);
	}
	sprintf(data,"%s##\tNtUserSetWindowsHookEx\t%s", procname,targetPath);
//	DbgPrint("the data is:%s",data);
	retvalue=manage(data,1);
//	DbgPrint("retvalue is:%d",retvalue);
	if((retvalue==1)||(retvalue==4))
	{
		return RealNtUserSetWindowsHookEx(
		 Mod,
		 UnsafeModuleName,
		 ThreadId,
		 HookId,
		 HookProc,
	     Ansi);
	}
	else if(retvalue==2)
	{
		return NULL;
	}
	else if(retvalue==3)
	{
		TerminateProcessByID(pID);
	}
	return   NULL;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//����hook shadow ssdt ����  ������ȡӲ���뷽ʽ
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------------------
unsigned int getAddressOfShadowTable()
{
    unsigned int i;
    unsigned char *p;
    unsigned int dwordatbyte;

    p = (unsigned char*) KeAddSystemServiceTable;
	DbgPrint("ths addsystemservicetable is :%x\n",p);

    for(i = 0; i < 4096; i++, p++)
    {
        __try
        {
            dwordatbyte = *(unsigned int*)p;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return 0;
        }

        if(MmIsAddressValid((PVOID)dwordatbyte))
        {
            if(memcmp((PVOID)dwordatbyte, &KeServiceDescriptorTable, 16) == 0)
            {
                if((PVOID)dwordatbyte == &KeServiceDescriptorTable)
                {
                    continue;
                }
				DbgPrint("the ssdt address is:%x\n",dwordatbyte);

                return dwordatbyte;
            }
        }
    }

    return 0;
}

ULONG getShadowTable()
{
    KeServiceDescriptorTableShadow = (PServiceDescriptorTableEntry_t) getAddressOfShadowTable();

    if(KeServiceDescriptorTableShadow == NULL)
    {
//        DbgPrint("hooker.sys: Couldnt find shadowtable!\n");
        
        return FALSE;
    }
    else
    {
  //      DbgPrint("hooker.sys: Shadowtable has been found!\n");
        
        DbgPrint("hooker.sys: Shadowtable entries: %d\n", KeServiceDescriptorTableShadow[1].NumberOfServices);
        return TRUE;
    }
} 

//���ݲ���ϵͳ��ȷ�����庯���ķ���� 
VOID InitCallNumber()
{
	ULONG majorVersion, minorVersion;
	PsGetVersion( &majorVersion, &minorVersion, NULL, NULL );
    if ( majorVersion == 5 && minorVersion == 2 )
    {
	  DbgPrint("comint32: Running on Windows 2003");
     

	}
	else if ( majorVersion == 5 && minorVersion == 1 )
	{
//	  DbgPrint("comint32: Running on Windows XP\n");

	  NtUserSetWindowsHookEx_callnumber=0x225;
	}
	else if ( majorVersion == 5 && minorVersion == 0 )
	{
	  DbgPrint("comint32: Running on Windows 2000");
	  
	}
}

PVOID GetInfoTable(ULONG ATableType)
{
  ULONG mSize = 0x4000;
  PVOID mPtr = NULL;
  NTSTATUS St;
  do
  {
     mPtr = ExAllocatePool(PagedPool, mSize);
     memset(mPtr, 0, mSize);
     if (mPtr)
     {
        St = ZwQuerySystemInformation(ATableType, mPtr, mSize, NULL);//�õ�Ҫ��ѯ��Ϣ�����С
     } else return NULL;
     if (St == STATUS_INFO_LENGTH_MISMATCH)
     {
        ExFreePool(mPtr);
        mSize = mSize * 2;
     }
  } while (St == STATUS_INFO_LENGTH_MISMATCH);
  if (St == STATUS_SUCCESS) return mPtr;
  ExFreePool(mPtr);
  return NULL;
}

HANDLE GetCsrPid()
{
	HANDLE Process, hObject;
	HANDLE CsrId = (HANDLE)0;
	OBJECT_ATTRIBUTES obj;
	CLIENT_ID cid;
	UCHAR Buff[0x100];
	POBJECT_NAME_INFORMATION ObjName = (PVOID)&Buff;
	PSYSTEM_HANDLE_INFORMATION_EX Handles;
	ULONG r;

	Handles = GetInfoTable(SystemHandleInformation);

	if (!Handles) return CsrId;

	for (r = 0; r < Handles->NumberOfHandles; r++)
	{
		if (Handles->Information[r].ObjectTypeNumber == 21) //Port object
		{
			InitializeObjectAttributes(&obj, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

			cid.UniqueProcess = (HANDLE)Handles->Information[r].ProcessId;
			cid.UniqueThread = 0;

			if (NT_SUCCESS(NtOpenProcess(&Process, PROCESS_DUP_HANDLE, &obj, &cid)))
			{
				if (NT_SUCCESS(ZwDuplicateObject(Process, (HANDLE)Handles->Information[r].Handle,NtCurrentProcess(), &hObject, 0, 0, DUPLICATE_SAME_ACCESS)))//������������ʲô��
				{
					if (NT_SUCCESS(ZwQueryObject(hObject, ObjectNameInformation, ObjName, 0x100, NULL)))//�ڶ�������ȷ�����ҵ�����.
					{
						if (ObjName->Name.Buffer && !wcsncmp(L"\\Windows\\ApiPort", ObjName->Name.Buffer, 20))
						{
						  CsrId = (HANDLE)Handles->Information[r].ProcessId;
						} 
					}

					ZwClose(hObject);
				}

				ZwClose(Process);
			}
		}
	}

	ExFreePool(Handles);
	return CsrId;
}

BOOLEAN Sleep(ULONG MillionSecond)
{
	NTSTATUS st;
	LARGE_INTEGER DelayTime;
	DelayTime = RtlConvertLongToLargeInteger(-10000*MillionSecond);
	st=KeDelayExecutionThread( KernelMode, FALSE, &DelayTime );
	return (NT_SUCCESS(st));
}

NTSTATUS InitShadowSSdtEntry()
{
  NTSTATUS status;
  
 
  //���shadow�ĵ�ַ
  getShadowTable();
  //���ݲ�ͬ��ϵͳ��ò�ͬ�ĺ��������
  InitCallNumber();

 
  

  status = PsLookupProcessByProcessId((ULONG)GetCsrPid(), &crsEProc);
  if (!NT_SUCCESS( status ))
  {
//	DbgPrint("PsLookupProcessByProcessId() error\n");
	return status;
  }
  KeAttachProcess(crsEProc);

  __try
  {
	  if ((KeServiceDescriptorTableShadow!=NULL) && (NtUserSetWindowsHookEx_callnumber!=0)) 
	  {

		RealNtUserSetWindowsHookEx      =(NTUSERSETWINDOWSHOOKEX)KeServiceDescriptorTableShadow[1].ServiceTableBase[NtUserSetWindowsHookEx_callnumber];
//		DbgPrint("the real setwindowshook address is:%x\n",RealNtUserSetWindowsHookEx);
		
	  }
	  else
		KeServiceDescriptorTableShadow=NULL;
		

	  _asm
	  {
		CLI                    //dissable interrupt
		MOV    EAX, CR0        //move CR0 register into EAX
		and    EAX, NOT 0x10000 //disable WP bit 
		MOV    CR0, EAX        //write register back
	  }
	  if ((KeServiceDescriptorTableShadow!=NULL)  && (NtUserSetWindowsHookEx_callnumber!=0))
	  {
		
		 (NTUSERSETWINDOWSHOOKEX)KeServiceDescriptorTableShadow[1].ServiceTableBase[NtUserSetWindowsHookEx_callnumber]=FakedNtUserSetWindowsHookEx;
	  }

	  _asm 
	  {
		MOV    EAX, CR0        //move CR0 register into EAX
		OR     EAX, 0x10000        //enable WP bit     
		MOV    CR0, EAX        //write register back        
		STI                    //enable interrupt
	  }
  }
  __finally
  {
      KeDetachProcess(); 
  }

  //KdPrint(("Hook NtUserSetWindowsHookEx status is Succeessfully "));


  return status ;
}






VOID ShadowSSDTUnloadDriver()
{
    
	NTSTATUS status;

	status = PsLookupProcessByProcessId((ULONG)GetCsrPid(), &crsEProc);
	if (!NT_SUCCESS( status ))
	{
	  DbgPrint("PsLookupProcessByProcessId() error\n");
	  return ;
	}
	KeAttachProcess(crsEProc);


//////////////////////UnHook ZwQuerySystemInformation/////////////////////////////////////////////////
 
	__try
	{
	  _asm
	 {
		CLI                    //dissable interrupt
		MOV    EAX, CR0        //move CR0 register into EAX
		AND EAX, NOT 10000H    //disable WP bit 
		MOV    CR0, EAX        //write register back
	  }

	  if ((KeServiceDescriptorTableShadow!=NULL)  && (NtUserSetWindowsHookEx_callnumber!=0)) 
	  {
         (NTUSERSETWINDOWSHOOKEX)KeServiceDescriptorTableShadow[1].ServiceTableBase[NtUserSetWindowsHookEx_callnumber]=RealNtUserSetWindowsHookEx;
	  }

	  _asm 
	  {
		MOV    EAX, CR0        //move CR0 register into EAX
		OR     EAX, 10000H     //enable WP bit     
		MOV    CR0, EAX        //write register back        
		STI                    //enable interrupt
	  }
    }
	__finally
   {
	 KeDetachProcess();
	 Sleep(50);
   }   
}





//��ס��غ���
VOID HookRegistry( void )
{
	if( !RegHooked ) {
//		DbgPrint("begin to hook");
		HOOK_SYSCALL(ZwCreateKey,FakedZwCreateKey,RealZwCreateKey);
		
//		DbgPrint("the address of ZwCreateKey is:%s\n",RealZwCreateKey);
//		DbgPrint("the address of FakedZwCreateKey is:%s\n",FakedZwCreateKey);
		
		HOOK_SYSCALL(ZwDeleteKey,FakedZwDeleteKey,RealZwDeleteKey);
		HOOK_SYSCALL(ZwDeleteValueKey,FakedZwDeleteValueKey,RealZwDeleteValueKey);
		HOOK_SYSCALL(ZwSetValueKey,FakedZwSetValueKey,RealZwSetValueKey);
		HOOK_SYSCALL(ZwLoadDriver,FakedZwLoadDriver,RealZwLoadDriver);
		HOOK_SYSCALL(ZwOpenProcess,FakedZwOpenProcess,RealZwOpenProcess);
        HOOK_SYSCALL(ZwTerminateProcess,FakedZwTerminateProcess,RealZwTerminateProcess);
		HOOK_SYSCALL(ZwCreateSection,FakedZwCreateSection,RealZwCreateSection);

		MYHOOK_SYSCALL(indexMem,FakedZwWriteVirtualMemory,RealZwWriteVirtualMemory);
		shadow_ssdt_hook=0;//�����������Ҫͬ��

//		InitShadowSSdtEntry();���������hook shadow ssdt����Ϊ��ַ�ռ䲻��ϵͳ��.
       
//		DbgPrint("protect.SYS:�ɹ���ס��غ���!\n");
		RegHooked = TRUE;

	}
}

//�������
VOID UnhookRegistry(void)
{
	if( RegHooked ) 
	{
        UNHOOK_SYSCALL(ZwCreateKey,RealZwCreateKey,FakedZwCreateKey);

//		DbgPrint("the address of ZwCreateKey is:%s\n",RealZwCreateKey);
		
		UNHOOK_SYSCALL(ZwDeleteKey,RealZwDeleteKey,FakedZwDeleteKey);
		UNHOOK_SYSCALL(ZwDeleteValueKey,RealZwDeleteValueKey,FakedZwDeleteValueKey);
		UNHOOK_SYSCALL(ZwSetValueKey,RealZwSetValueKey,FakedZwSetValueKey);
		UNHOOK_SYSCALL(ZwLoadDriver,RealZwLoadDriver,FakedZwLoadDriver);
	    UNHOOK_SYSCALL(ZwOpenProcess,RealZwOpenProcess,FakedZwOpenProcess);
		UNHOOK_SYSCALL(ZwTerminateProcess,RealZwTerminateProcess,FakedZwTerminateProcess);
		UNHOOK_SYSCALL(ZwCreateSection,RealZwCreateSection,FakedZwCreateSection);
		MYUNHOOK_SYSCALL(indexMem,RealZwWriteVirtualMemory,FakedZwWriteVirtualMemory);

//		ShadowSSDTUnloadDriver();

//		DbgPrint("protect.SYS:�ɹ��������!\n");
		shadow_ssdt_hook=1;
		RegHooked = FALSE;
	}
}


//IOCTL�����봦������
BOOLEAN  RegmonDeviceControl(IN PFILE_OBJECT FileObject, 
							 IN BOOLEAN Wait,
							 IN PVOID InputBuffer, 
							 IN ULONG InputBufferLength, 
							 OUT PVOID OutputBuffer, 
							 IN ULONG OutputBufferLength, 
							 IN ULONG IoControlCode,
							 OUT PIO_STATUS_BLOCK IoStatus, 
							 IN PDEVICE_OBJECT DeviceObject ) 
{
	NTSTATUS status;

	BOOLEAN                 retval = FALSE;
	PSTORE_BUF              old;
	char                    *buff;
	DWORD                   outaddress;
	ULONG	               desiredBufferLength;
 
	//��GUI�û��������¼�
	HANDLE				hEventReceive=NULL;
	HANDLE              hEventSend=NULL;
	OBJECT_HANDLE_INFORMATION	objHandleInfo;


	//Regemon_getprocess���õ�����ʱ����
	UCHAR* ch;
	PPROCESS_BUF oldProcess;
	int i;
	struct_process *tmp;


	IoStatus->Status      = STATUS_SUCCESS; // Assume success
	IoStatus->Information = 0;              // Assume nothing returned

	switch ( IoControlCode )
	{
	case Begin_hook:
		HookRegistry();
//		DbgPrint("begin hook\n");
		break;

	case REGMON_hook:
		DbgPrint("protect.SYS: ��ʼhook...........\n");
		buff = InputBuffer;
		memmove(&outaddress,buff,4);        //�û�������Ļ�������ʼ��ַ
		memmove(&indexMem,&buff[4],4);      //���û��������NtWriteVirtualMemory��ַ
        memmove(&protectedPid,&buff[8],4);  //�û��������ID
		memmove(&hEventReceive,&buff[12],4);//�ȴ��û�����ѡ�������¼�
		memmove(&hEventSend,&buff[16],4);   //���û��������������¼� 

 //       DbgPrint("pid........%ld\n",protectedPid);

		status = ObReferenceObjectByHandle(
					hEventReceive,
					GENERIC_ALL,
					NULL,
					KernelMode,
					&gpEventObjectReceive,
					&objHandleInfo);//���¼�����õ��¼�����
		if(status!= STATUS_SUCCESS)
		{
			DbgPrint("***************************************\n");
			DbgPrint("/******ObReferenceObjectByHandle failed! status = %x\n", status);
			DbgPrint("*******Error********************************\n");
			DbgPrint("*******Error********************************\n");
			//break;
		}

		
		status = ObReferenceObjectByHandle(
					hEventSend,
					GENERIC_ALL,
					NULL,
					KernelMode,
					&gpEventObjectSend,
					&objHandleInfo);
		if(status!= STATUS_SUCCESS)
		{
			DbgPrint("***************************************\n");
			DbgPrint("ObReferenceObjectByHandle failed! status = %x\n", status);
			DbgPrint("********Error*******************************\n");
			//break;
		}
		
		output = (char *)MmMapIoSpace(MmGetPhysicalAddress((void *)outaddress),2048,0);

		RealZwWriteVirtualMemory =(ZWWRITEVIRTUALMEMORY) (4*indexMem+(ULONG)KeServiceDescriptorTable.ServiceTableBase);
//		HookRegistry();
//		DbgPrint("protect.SYS: ���hook...........\n");
		break;

	case REGMON_unhook:
//		DbgPrint("protect.SYS: ��ʼunhook\n");
		UnhookRegistry();
//		DbgPrint("protect.SYS: ���unhook\n");
		break;

	case REGMON_zerostats:

		
		// Zero contents of buffer
//		DbgPrint("protect.SYS: ��� store\n");
		MUTEX_WAIT( StoreMutex );
		while ( Store->Next ) 
		{
			// release next
			old = Store->Next;
			Store->Next = old->Next;
			MUTEX_WAIT( StoreMutex );
			ExFreePool( old );
			NumStore--;
			MUTEX_RELEASE( StoreMutex );
		}//���
		Store->Len = 0;
		MUTEX_RELEASE( StoreMutex );
		break;

	case REGMON_getstats:

		
		// Copy buffer into user space.
//		DbgPrint ("protect.SYS: ��� store\n");

		MUTEX_WAIT( StoreMutex );
		if ( MAX_STORE > OutputBufferLength )  
		{
			// �û�����Ļ�����̫С
			MUTEX_RELEASE( StoreMutex );
			IoStatus->Status = STATUS_INVALID_PARAMETER;
//			DbgPrint("protect.SYS: �û�����Ļ�����̫С\n");
			return FALSE;                                 //���ֵû�˽���
		} 
		else if ( Store->Len  ||  Store->Next )
		{
			//�ƺ�û��ͬ�������³�������

			// Switch to a new store
			RegmonNewStore();

			// Fetch the oldest to give to user
			old = RegmonOldestStore();
			MUTEX_RELEASE( StoreMutex );

			// Copy it
			memcpy( OutputBuffer, old->Data, old->Len );
//			DbgPrint("out put ok\n");
		
			// Return length of copied info
			IoStatus->Information = old->Len;

			// Deallocate buffer
			ExFreePool( old );

		} 
		else {
			// No unread data
			MUTEX_RELEASE( StoreMutex );
//			DbgPrint("protect.SYS:������store���������!\n");
			IoStatus->Information = 0;
		}
		break;
	case REGMON_getprocess:
//		DbgPrint (("Regmon: get process\n"));
		totalProcess=0;
		EnumProcess();
		ch=(UCHAR*)OutputBuffer;//�������ĵ�ַ
		*ch=totalProcess;//�Ȱ��ܵĽ�����Ŀ�����������
		tmp=(struct_process *)(ch+1);//����һ���ֽڣ�����һ�����ݽṹ
		ch=(UCHAR*)OutputBuffer;
//		DbgPrint("the output address is:%d\n",*ch);
		while(PROCESS_HEAD)//where allocate the PROCESS_HEAD ?
		{
			tmp->pEProcess=PROCESS_HEAD->pEProcess;
			tmp->PID=PROCESS_HEAD->PID;
			memcpy(tmp->pFileName,PROCESS_HEAD->pFileName,strlen(PROCESS_HEAD->pFileName));
			tmp->pFileName[strlen(PROCESS_HEAD->pFileName)]='\0';

			memcpy(tmp->pPathName,PROCESS_HEAD->pPathName,strlen(PROCESS_HEAD->pPathName));
			tmp->pPathName[strlen(PROCESS_HEAD->pPathName)]='\0';

			tmp++;
			//memcpy(tmp,PROCESS);
  	        //DbgPrint("0x%08X  %04d   %s",PROCESS_HEAD->pEProcess,PROCESS_HEAD->PID,PROCESS_HEAD->pFileName);
  	        oldProcess=PROCESS_HEAD;
           	PROCESS_HEAD=PROCESS_HEAD->Next;
         	ExFreePool(oldProcess);	
		}
		tmp=(struct_process *)((UCHAR*)OutputBuffer+1);
		for(i=0;i<((UCHAR*)OutputBuffer)[0];i++)
		{
//			DbgPrint("%d 0x%08X  %04d   %s %s",i,tmp->pEProcess,tmp->PID,tmp->pFileName,tmp->pPathName);
			tmp++;	
		}
		IoStatus->Information = totalProcess*sizeof(struct_process)+1;
		break;


	default:
//		DbgPrint("protect.SYS: δ֪�� IRP_MJ_DEVICE_CONTROL\n");
		IoStatus->Status = STATUS_INVALID_DEVICE_REQUEST;
		break;
    }
		return TRUE;
}


NTSTATUS HookDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
	PIO_STACK_LOCATION      irpStack;
	PVOID                   inputBuffer;
	PVOID                   outputBuffer;
	ULONG                   inputBufferLength;
	ULONG                   outputBufferLength;
	ULONG                   ioControlCode;
	PSTORE_BUF              old;
	WORK_QUEUE_ITEM         workItem;//������

	
	// Go ahead and set the request up as successful
	Irp->IoStatus.Status      = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	// Get a pointer to the current location in the Irp. This is where
	//     the function codes and parameters are located.
	irpStack = IoGetCurrentIrpStackLocation (Irp);

	
	// Get the pointer to the input/output buffer and its length
	inputBuffer             = Irp->AssociatedIrp.SystemBuffer;
	inputBufferLength       = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outputBuffer            = Irp->AssociatedIrp.SystemBuffer;
	outputBufferLength      = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	ioControlCode           = irpStack->Parameters.DeviceIoControl.IoControlCode;

	switch (irpStack->MajorFunction)
	{
	case IRP_MJ_CREATE:
//		DbgPrint("protect.SYS: ����������IRP_MJ_CREATE\n");
		break;
	case IRP_MJ_CLOSE:
//		DbgPrint("protect.SYS: ����������IRP_MJ_CLOSE\n");
		RegmonResetStore();
		break;

	case IRP_MJ_DEVICE_CONTROL:
//		DbgPrint("protect.SYS: ����������IRP_MJ_DEVICE_CONTROL\n");

		RegmonDeviceControl( irpStack->FileObject, 
			                 TRUE,
			                 inputBuffer, 
							 inputBufferLength, 
			                 outputBuffer,
							 outputBufferLength,
			                 ioControlCode,
							 &Irp->IoStatus, 
							 DeviceObject);
		break;
	case IRP_MJ_PNP :
		break;
	case IRP_MJ_POWER: 
		break;
	case IRP_MJ_READ :
		break;
	case IRP_MJ_WRITE :
		break;
	case IRP_MJ_FLUSH_BUFFERS :
		break;
	case IRP_MJ_QUERY_INFORMATION :
		break;
	case IRP_MJ_SET_INFORMATION :
		break;
	case IRP_MJ_INTERNAL_DEVICE_CONTROL :
		break;
	case IRP_MJ_SYSTEM_CONTROL :
		break;
	case IRP_MJ_CLEANUP :
		break;
	case IRP_MJ_SHUTDOWN :
		break;

	}
	IoCompleteRequest( Irp, IO_NO_INCREMENT );
	return STATUS_SUCCESS;   
}

//ж������(��ɾ���豸���ͷſռ� ����������)
VOID HookUnload( IN PDRIVER_OBJECT DriverObject )
{
	UNICODE_STRING          deviceLinkUnicodeString;

	DbgPrint("protect.SYS: ж����.........\n");

    //���ע�����
	if( RegHooked ) 
		UnhookRegistry();

	//���ShadowSSDT����
	ShadowSSDTUnloadDriver();
	DbgPrint("SHADOW SSDT OK\n");

	//�ͷ�ӳ��SSDT���ÿռ�
	if(g_pmdlSystemCall)
	{
		MmUnmapLockedPages(MappedSystemCallTable, g_pmdlSystemCall);
		IoFreeMdl(g_pmdlSystemCall);
	}

	//�����������
	RtlInitUnicodeString( &deviceLinkUnicodeString, DOS_DEVICE_NAME );
	IoDeleteSymbolicLink( &deviceLinkUnicodeString );

    //ɾ���豸
	IoDeleteDevice( DriverObject->DeviceObject );
    DbgPrint("DELETE DRIVER OK\n");
    //is there any error to free this list?there may be a function is read the list 
	FreeProtectList(gpRegeditProtect);
	FreeProcessWhiteList(gpWhiteList);
    DbgPrint("FREE LIST OK\n");
	//�ͷ�ͨѶ�¼�
	if(gpEventObjectReceive)
	{
		KeClearEvent(gpEventObjectReceive);
		ObDereferenceObject(gpEventObjectReceive); 
		gpEventObjectReceive=NULL;
	}else
	{
		DbgPrint("Error!\n");
	}
	if(gpEventObjectSend)
	{
		KeClearEvent(gpEventObjectSend);
		ObDereferenceObject(gpEventObjectSend); 
		gpEventObjectSend=NULL;
	}else
	{
		DbgPrint("Error!\n");
	}
	
//	DbgPrint("protect.SYS:KeClearEvent sussfully!\n");
	//
	// Now we can free any memory we have outstanding
	//
	RegmonHashCleanup();
	RegmonFreeStore();

	DbgPrint("protect.SYS: ж�سɹ�!\n");
}


//���
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath )
{
	NTSTATUS                ntStatus;

	UNICODE_STRING          deviceNameUnicodeString;//�豸��
	UNICODE_STRING          deviceLinkUnicodeString;//����������  
    PDEVICE_OBJECT          GUIDevice=NULL;         //�豸����

	int                     i=0;

//	DbgPrint ("protect.SYS: ���� DriverEntry\n");

	//�޸�SSDT�ڴ汣������
	g_pmdlSystemCall = MmCreateMdl(NULL, 
		                           KeServiceDescriptorTable.ServiceTableBase, 
								   KeServiceDescriptorTable.NumberOfServices*4);
	if(!g_pmdlSystemCall)
	{
//		DbgPrint("ӳ��SSDT��ʧ��!\n");
		return STATUS_UNSUCCESSFUL;
	}

	MmBuildMdlForNonPagedPool(g_pmdlSystemCall);

	g_pmdlSystemCall->MdlFlags = g_pmdlSystemCall->MdlFlags | MDL_MAPPED_TO_SYSTEM_VA;

	MappedSystemCallTable = MmMapLockedPages(g_pmdlSystemCall, KernelMode);

	

	//��ʼ��ע�������
	ntStatus=InitProtectList(&gpRegeditProtect);
	if (!NT_SUCCESS(ntStatus))
	{
//		DbgPrint("��ʼ��ע�������ʧ��!\n");
		return ntStatus;
	}
	
	//��ʼ�����̱�����
	ntStatus=InitWhiteProcessList(&gpWhiteList);
	if (!NT_SUCCESS(ntStatus))
	{
		//û���ͷ�gpRegeditProtect����������
//		DbgPrint("��ʼ�����̱�����ʧ��\n");
		return ntStatus;
	}


    //��ʼ��RootKey��CurrentUser������
	for( i = 0; i < NUMROOTKEYS; i++ )
		RootKey[i].RootNameLen = strlen( RootKey[i].RootName );
	for( i = 0; i < 2; i++ )
		CurrentUser[i].RootNameLen = strlen( CurrentUser[i].RootName );

	
	//��ʼ���洢������ռ�
	Store   = ExAllocatePool( PagedPool, sizeof(*Store) );
	if ( !Store ) 
	{
//		DbgPrint("����洢�ռ�ʧ��!\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	Store->Len  = 0;
	Store->Next= NULL;
	NumStore = 1;



	// ע���豸�ʹ�����������
	RtlInitUnicodeString (&deviceNameUnicodeString,
		DEVICE_NAME );
	RtlInitUnicodeString (&deviceLinkUnicodeString,
		DOS_DEVICE_NAME );

	ntStatus = IoCreateDevice ( DriverObject,
		                        0,
	                        	&deviceNameUnicodeString,
	                        	FILE_DEVICE_UNKNOWN,
	                         	0,
	                           	FALSE,
                        		&GUIDevice );
	if(!NT_SUCCESS(ntStatus))
	{
//		DbgPrint("�����豸ʧ��!\n");
		return ntStatus;
	}

	ntStatus = IoCreateSymbolicLink (&deviceLinkUnicodeString,
		                             &deviceNameUnicodeString );
	if (!NT_SUCCESS(ntStatus))
	{
//		DbgPrint("������������ʧ��!\n");

		//�ͷ���Դ
		IoDeleteDevice( GUIDevice );
		IoDeleteSymbolicLink( &deviceLinkUnicodeString ); //?
		return ntStatus;
	}


	
	// ָ���ص�����    
	for(i=0;i<IRP_MJ_MAXIMUM_FUNCTION;i++)
		DriverObject->MajorFunction[i]          =HookDispatch;
	DriverObject->DriverUnload                          = HookUnload;


	//������ʼ��
	RealZwOpenKey=(ZWOPENKEY)(SYSTEMSERVICE(ZwOpenKey));
    //RealZwWriteVirtualMemory=(ZWWRITEVIRTUALMEMORY)(SYSTEMSERVICE(ZwWriteVirtualMemory));

	ProcessNameOffset = GetProcessNameOffset();
	
	pSystem    = PsGetCurrentProcess();
    pebAddress = GetPebAddress();
    pObjectTypeProcess = *(PULONG)((ULONG)pSystem - OBJECT_HEADER_SIZE +OBJECT_TYPE_OFFSET);  


	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	MUTEX_INIT(StoreMutex);
	MUTEX_INIT(HashMutex);
	KeInitializeEvent(&event,SynchronizationEvent,1);

	ntStatus=InitShadowSSdtEntry();
	if(!NT_SUCCESS(ntStatus))
	{
		 
		DbgPrint("�޷���ס shadow SSDT\n");
	}

//	DbgPrint("�ɹ���װprotect.sys����!\n");

	return ntStatus;
}



ULONG  GetPebAddress()
{
  ULONG Address;
  PEPROCESS pEProcess;

        //����system���̵�peb������ ����ֻ�е���������ȥ����
  pEProcess = (PEPROCESS)((ULONG)((PLIST_ENTRY)((ULONG)pSystem + PROCESS_LINK_OFFSET))->Flink - PROCESS_LINK_OFFSET);
  Address   = *(PULONG)((ULONG)pEProcess + PEB_OFFSET);

  return (Address & 0xFFFF0000);  
}

VOID EnumProcess()
{
  ULONG  uSystemAddress = (ULONG)pSystem;
  ULONG  i;
  ULONG  Address;
  ULONG  ret;

  DbgPrint("-------------------------------------------");
  DbgPrint("EProcess    PID    ImageFileName");
  DbgPrint("---------------------------------");
  

  for(i = 0x80000000; i < uSystemAddress; i += 4){//system���̵�EPROCESS��ַ�������ֵ��
    ret = VALIDpage(i); 
    if (ret == VALID){ 
      Address = *(PULONG)i;
      if (( Address & 0xFFFF0000) == pebAddress){//ÿ�����̵�PEB��ַ�����ڲ��ĵط�����ַǰ�벿������ͬ��       
        if(IsaRealProcess(i)){ 
          ShowProcess(i - PEB_OFFSET);  
           i += EPROCESS_SIZE;                
        } 
      } 
    }else if(ret == PTE_INVALID){ 
      i -=4; 
      i += 0x1000;//4k 
    }else{ 
      i-=4; 
      i+= 0x400000;//4mb 
    } 
  }

  //ShowProcess(uSystemAddress);//system��PEB������ ����ķ�����ö�ٲ����� ����������PsGetCurrentProcess���ܵõ���
  DbgPrint("-------------------------------------------");
  
}

VOID    ShowProcess(ULONG pEProcess)
{
  PLARGE_INTEGER ExitTime=NULL;
  ULONG PID;
  PUCHAR pFileName;
  char PathName[MAXPATHLEN];

  int len;
  PPROCESS_BUF pBuf;
  
  ExitTime = (PLARGE_INTEGER)(pEProcess + EXIT_TIME_OFFSET); 
  
  if(ExitTime==NULL)
	  return;
  if(ExitTime->QuadPart != 0) //�Ѿ������Ľ��̵�ExitTimeΪ����
  {
	  ExitTime=NULL;
	  return ;
  }
  ExitTime=NULL;

  PID = *(PULONG)(pEProcess + PROCESS_ID_OFFSET);
  pFileName = (PUCHAR)(pEProcess + FILE_NAME_OFFSET);
  len=strlen(pFileName);
  
  if(PROCESS_HEAD==NULL)
  {
  	PROCESS_HEAD   = ExAllocatePool( PagedPool, sizeof(PROCESS_BUF));

  	PROCESS_HEAD->pEProcess=pEProcess;
  	PROCESS_HEAD->PID=PID;
  	memcpy(PROCESS_HEAD->pFileName,pFileName,len);
  	PROCESS_HEAD->pFileName[len]='\0';

	//GetProcessImageName(PathName,(HANDLE)(pEProcess+0x1FC));
	strcpy(PathName,(char*)(pEProcess+0x1FC));
	len=strlen(PathName);
	memcpy(PROCESS_HEAD->pPathName,PathName,len);
  	PROCESS_HEAD->pPathName[len]='\0';

	
  	PROCESS_HEAD->Next=NULL;
  }
  else
  {
  	pBuf=ExAllocatePool( PagedPool, sizeof(PROCESS_BUF));
  	pBuf->pEProcess=pEProcess;
  	pBuf->PID=PID;
  	memcpy(pBuf->pFileName,pFileName,len);
  	pBuf->pFileName[len]='\0';

	//GetProcessImageName(PathName,(HANDLE)(pEProcess+0x1FC));
	strcpy(PathName,(char*)((DWORD*)pEProcess+0x1FC));
	DbgPrint("������:%s\n",PathName);
	len=strlen(PathName);
	memcpy(PROCESS_HEAD->pPathName,PathName,len);
  	PROCESS_HEAD->pPathName[len]='\0';


  	pBuf->Next=PROCESS_HEAD;
  	PROCESS_HEAD=pBuf;	
  }
  totalProcess++;
  //DbgPrint("0x%08X  %04d   %s",pEProcess,PID,pFileName);
}

ULONG VALIDpage(ULONG addr) 
{ 
  ULONG pte; 
  ULONG pde; 
  

  if(MmIsAddressValid((PVOID)addr))
    return  VALID;
 
  return PTE_INVALID; 

  pde = 0xc0300000 + (addr>>22)*4; 
  if((*(PULONG)pde & 0x1) != 0){ 
    //large page 
    if((*(PULONG)pde & 0x80) != 0){ 
      return VALID; 
    } 
    pte = 0xc0000000 + (addr>>12)*4; 
    if((*(PULONG)pte & 0x1) != 0){ 
      return VALID; 
    }else{ 
      return PTE_INVALID; 
    } 
  } 
  return PDE_INVALID; 
} 

BOOLEAN IsaRealProcess(ULONG i) 
{ 
  NTSTATUS STATUS; 
  PUNICODE_STRING pUnicode; 
  UNICODE_STRING Process; 
  ULONG pObjectType; 
  ULONG ObjectTypeAddress; 
  
  if (VALIDpage(i- PEB_OFFSET) != VALID){ 
    return FALSE; 
  } 

  ObjectTypeAddress = i - PEB_OFFSET - OBJECT_HEADER_SIZE + OBJECT_TYPE_OFFSET ;
  
  if (VALIDpage(ObjectTypeAddress) == VALID){ 
    pObjectType = *(PULONG)ObjectTypeAddress; 
  }else{ 
    return FALSE; 
  } 
  
  if(pObjectTypeProcess == pObjectType){ //ȷ��ObjectType��Process����
    return TRUE; 
  } 
  return FALSE; 

} 