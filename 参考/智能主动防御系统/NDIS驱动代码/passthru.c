/*++
 
模块名称:
 
    passthru.c

    智能主动防御系统-NDIS中间层驱动程序

--*/
#include "precomp.h"
#include "IOCommon.h"
#include "hostlist.h"
#include "Packet.h"
#include "hashlink.h"
#pragma hdrstop

#pragma NDIS_INIT_FUNCTION(DriverEntry)

extern NDIS_PHYSICAL_ADDRESS	HighestAcceptableMax;

//数据包过滤规则-默认所有过滤均开启

UCHAR ICMP_FILTER=1;   //是否允许Ping入
UCHAR PACKET_FILTER=1; //是否进行数据包过滤
UCHAR ANTIARP=1;       //是否启用ARP防火墙

NDIS_SPIN_LOCK  IcmpLock;
NDIS_SPIN_LOCK  PacketLock;
NDIS_SPIN_LOCK  ArpLock;
NDIS_SPIN_LOCK  NetGateListLock;
NDIS_SPIN_LOCK  HashLinkLock;
NDIS_SPIN_LOCK  AttackInforLock;

HostListLink netgatelist=NULL; //网关信息列表

Attack_Infor attack_infor;     //攻击日志结构体

PVOID	gpEventObject = NULL;    //通信事件

NDIS_HANDLE         ProtHandle = NULL;
NDIS_HANDLE         DriverHandle = NULL;
NDIS_MEDIUM         MediumArray[4] =
                    {
                        NdisMedium802_3,    // Ethernet
                        NdisMedium802_5,    // Token-ring
                        NdisMediumFddi,     // Fddi
                        NdisMediumWan       // NDISWAN
                    };

NDIS_SPIN_LOCK     GlobalLock;

PADAPT             pAdaptList = NULL;
LONG               MiniportCount = 0;

NDIS_HANDLE        NdisWrapperHandle;

//
// To support ioctls from user-mode:
//

#define LINKNAME_STRING     L"\\DosDevices\\Passthru"
#define NTDEVICE_STRING     L"\\Device\\Passthru"

NDIS_HANDLE     NdisDeviceHandle = NULL;
PDEVICE_OBJECT  ControlDeviceObject = NULL;

enum _DEVICE_STATE
{
    PS_DEVICE_STATE_READY = 0,    // ready for create/delete
    PS_DEVICE_STATE_CREATING,    // create operation in progress
    PS_DEVICE_STATE_DELETING    // delete operation in progress
} ControlDeviceState = PS_DEVICE_STATE_READY;



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT        DriverObject,
    IN PUNICODE_STRING       RegistryPath
    )
/*++

Routine Description:

    First entry point to be called, when this driver is loaded.
    Register with NDIS as an intermediate driver.

Arguments:

    DriverObject - pointer to the system's driver object structure
        for this driver
    
    RegistryPath - system's registry path for this driver
    
Return Value:

    STATUS_SUCCESS if all initialization is successful, STATUS_XXX
    error code if not.

--*/
{
    NDIS_STATUS                        Status;
    NDIS_PROTOCOL_CHARACTERISTICS      PChars;
    NDIS_MINIPORT_CHARACTERISTICS      MChars;
    NDIS_STRING                        Name;

    DBGPRINT(("PassThru 驱动加载!"));
		Status = NDIS_STATUS_SUCCESS;
		
		//创建网关信息链表
	  CreateHostList(&netgatelist);

    //全局锁
    NdisAllocateSpinLock(&GlobalLock);
    NdisAllocateSpinLock(&IcmpLock);
    NdisAllocateSpinLock(&PacketLock);
    NdisAllocateSpinLock(&ArpLock);
    NdisAllocateSpinLock(&NetGateListLock);
    NdisAllocateSpinLock(&HashLinkLock);
    NdisAllocateSpinLock(&AttackInforLock);
	
	 //初始化攻击日志结构体
	  NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));

    NdisMInitializeWrapper(&NdisWrapperHandle, DriverObject, RegistryPath, NULL);

    do
    {
        //
        // Register the miniport with NDIS. Note that it is the miniport
        // which was started as a driver and not the protocol. Also the miniport
        // must be registered prior to the protocol since the protocol's BindAdapter
        // handler can be initiated anytime and when it is, it must be ready to
        // start driver instances.
        //

        NdisZeroMemory(&MChars, sizeof(NDIS_MINIPORT_CHARACTERISTICS));

        MChars.MajorNdisVersion = PASSTHRU_MAJOR_NDIS_VERSION;
        MChars.MinorNdisVersion = PASSTHRU_MINOR_NDIS_VERSION;

        MChars.InitializeHandler = MPInitialize;
        MChars.QueryInformationHandler = MPQueryInformation;
        MChars.SetInformationHandler = MPSetInformation;
        MChars.ResetHandler = NULL;
        MChars.TransferDataHandler = MPTransferData;
        MChars.HaltHandler = MPHalt;
#ifdef NDIS51_MINIPORT
        MChars.CancelSendPacketsHandler = MPCancelSendPackets;
        MChars.PnPEventNotifyHandler = MPDevicePnPEvent;
        MChars.AdapterShutdownHandler = MPAdapterShutdown;
#endif // NDIS51_MINIPORT

        //
        // We will disable the check for hang timeout so we do not
        // need a check for hang handler!
        //
        MChars.CheckForHangHandler = NULL;
        MChars.ReturnPacketHandler = MPReturnPacket;

        //
        // Either the Send or the SendPackets handler should be specified.
        // If SendPackets handler is specified, SendHandler is ignored
        //
        MChars.SendHandler = NULL;    // MPSend;
        MChars.SendPacketsHandler = MPSendPackets;

        Status = NdisIMRegisterLayeredMiniport(NdisWrapperHandle,
                                                  &MChars,
                                                  sizeof(MChars),
                                                  &DriverHandle);
        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

#ifndef WIN9X
        NdisMRegisterUnloadHandler(NdisWrapperHandle, PtUnload);
#endif

        //
        // Now register the protocol.
        //
        NdisZeroMemory(&PChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
        PChars.MajorNdisVersion = PASSTHRU_PROT_MAJOR_NDIS_VERSION;
        PChars.MinorNdisVersion = PASSTHRU_PROT_MINOR_NDIS_VERSION;

        //
        // Make sure the protocol-name matches the service-name
        // (from the INF) under which this protocol is installed.
        // This is needed to ensure that NDIS can correctly determine
        // the binding and call us to bind to miniports below.
        //
        NdisInitUnicodeString(&Name, L"Passthru");    // Protocol name
        PChars.Name = Name;
        PChars.OpenAdapterCompleteHandler = PtOpenAdapterComplete;
        PChars.CloseAdapterCompleteHandler = PtCloseAdapterComplete;
        PChars.SendCompleteHandler = PtSendComplete;
        PChars.TransferDataCompleteHandler = PtTransferDataComplete;
    
        PChars.ResetCompleteHandler = PtResetComplete;
        PChars.RequestCompleteHandler = PtRequestComplete;
        PChars.ReceiveHandler = PtReceive;
        PChars.ReceiveCompleteHandler = PtReceiveComplete;
        PChars.StatusHandler = PtStatus;
        PChars.StatusCompleteHandler = PtStatusComplete;
        PChars.BindAdapterHandler = PtBindAdapter;
        PChars.UnbindAdapterHandler = PtUnbindAdapter;
        PChars.UnloadHandler = PtUnloadProtocol;

        PChars.ReceivePacketHandler = PtReceivePacket;
        PChars.PnPEventHandler= PtPNPHandler;

        NdisRegisterProtocol(&Status,
                             &ProtHandle,
                             &PChars,
                             sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

        if (Status != NDIS_STATUS_SUCCESS)
        {
            NdisIMDeregisterLayeredMiniport(DriverHandle);
            break;
        }

        NdisIMAssociateMiniport(DriverHandle, ProtHandle);
    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
    }
    return(Status);
}


NDIS_STATUS
PtRegisterDevice(
    VOID
    )
/*++

Routine Description:

    Register an ioctl interface - a device object to be used for this
    purpose is created by NDIS when we call NdisMRegisterDevice.

    This routine is called whenever a new miniport instance is
    initialized. However, we only create one global device object,
    when the first miniport instance is initialized. This routine
    handles potential race conditions with PtDeregisterDevice via
    the ControlDeviceState and MiniportCount variables.

    NOTE: do not call this from DriverEntry; it will prevent the driver
    from being unloaded (e.g. on uninstall).

Arguments:

    None

Return Value:

    NDIS_STATUS_SUCCESS if we successfully register a device object.

--*/
{
    NDIS_STATUS            Status = NDIS_STATUS_SUCCESS;
    UNICODE_STRING         DeviceName;
    UNICODE_STRING         DeviceLinkUnicodeString;
    PDRIVER_DISPATCH       DispatchTable[IRP_MJ_MAXIMUM_FUNCTION+1];

    DBGPRINT(("==>PtRegisterDevice\n"));

    NdisAcquireSpinLock(&GlobalLock);

    ++MiniportCount;
    
    if (1 == MiniportCount)
    {
        ASSERT(ControlDeviceState != PS_DEVICE_STATE_CREATING);

        //
        // Another thread could be running PtDeregisterDevice on
        // behalf of another miniport instance. If so, wait for
        // it to exit.
        //
        while (ControlDeviceState != PS_DEVICE_STATE_READY)
        {
            NdisReleaseSpinLock(&GlobalLock);
            NdisMSleep(1);
            NdisAcquireSpinLock(&GlobalLock);
        }

        ControlDeviceState = PS_DEVICE_STATE_CREATING;

        NdisReleaseSpinLock(&GlobalLock);

    
        NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION+1) * sizeof(PDRIVER_DISPATCH));

        DispatchTable[IRP_MJ_CREATE] = PtDispatch;
        DispatchTable[IRP_MJ_CLEANUP] = PtDispatch;
        DispatchTable[IRP_MJ_CLOSE] = PtDispatch;
        DispatchTable[IRP_MJ_DEVICE_CONTROL] = PtDispatch;
        

        NdisInitUnicodeString(&DeviceName, NTDEVICE_STRING);
        NdisInitUnicodeString(&DeviceLinkUnicodeString, LINKNAME_STRING);

        //
        // Create a device object and register our dispatch handlers
        //
        
        Status = NdisMRegisterDevice(
                    NdisWrapperHandle, 
                    &DeviceName,
                    &DeviceLinkUnicodeString,
                    &DispatchTable[0],
                    &ControlDeviceObject,
                    &NdisDeviceHandle
                    );

        NdisAcquireSpinLock(&GlobalLock);

        ControlDeviceState = PS_DEVICE_STATE_READY;
    }

    NdisReleaseSpinLock(&GlobalLock);

    DBGPRINT(("<==PtRegisterDevice: %x\n", Status));

    return (Status);
}


NTSTATUS
PtDispatch(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
/*++
Routine Description:

    Process IRPs sent to this device.

Arguments:

    DeviceObject - pointer to a device object
    Irp      - pointer to an I/O Request Packet

Return Value:

    NTSTATUS - STATUS_SUCCESS always - change this when adding
    real code to handle ioctls.

--*/
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               FunctionCode;
    PUCHAR              ioBuffer = NULL;
    ULONG               inputBufferLength;
    ULONG               outputBufferLength;
	OBJECT_HANDLE_INFORMATION	objHandleInfo;
	HANDLE				hEvent;
	unsigned int ip=0;
	unsigned char mac[6]={0};
	unsigned int i=0;


    UNREFERENCED_PARAMETER(DeviceObject);
    
    DBGPRINT(("==>Pt Dispatch\n"));
    irpStack = IoGetCurrentIrpStackLocation(Irp);     
    switch (irpStack->MajorFunction)
    {
      case IRP_MJ_CREATE:
           break;
            
      case IRP_MJ_CLEANUP:
           break;
            
      case IRP_MJ_CLOSE:
           break;        
            
      case IRP_MJ_DEVICE_CONTROL:
			FunctionCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
			ioBuffer = Irp->AssociatedIrp.SystemBuffer;
			inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
			outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;   
			switch (FunctionCode)
			{
		 	case IADS_PACKET_FILTER:  //数据包过滤开关
				 DbgPrint("iads_packet_fiter setting!");
		 		 NdisAcquireSpinLock(&PacketLock);
				 PACKET_FILTER=ioBuffer[0];
				 NdisReleaseSpinLock(&PacketLock);
				 break;
			case IADS_ICMP_FILTER:    //Ping入开关
				 DbgPrint("iads_icmp_filter setting!");
				 NdisAcquireSpinLock(&IcmpLock);
				 ICMP_FILTER=ioBuffer[0];
				 NdisReleaseSpinLock(&IcmpLock);
				 break;
			case IADS_ANTIARP:        //防ARP欺骗开关
				 DbgPrint("iads_antiarp setting!");
				 NdisAcquireSpinLock(&ArpLock);
				 ANTIARP=ioBuffer[0];
				 NdisReleaseSpinLock(&ArpLock);
				 break;
			case IADS_DRIVER_NETGATE: //添加网关信息
				 DbgPrint("add netgate infor!");
				 NdisAcquireSpinLock(&NetGateListLock);
				 ip=*(unsigned int *)ioBuffer;
				 mac[0]=ioBuffer[4];
				 mac[1]=ioBuffer[5];
				 mac[2]=ioBuffer[6];
				 mac[3]=ioBuffer[7];
				 mac[4]=ioBuffer[8];
				 mac[5]=ioBuffer[9];
				 InsertHostList(netgatelist,ip,mac);
				 DbgPrint("%d.%d.%d.%d %02x-%02x-%02x-%02x-%02x-%02x",
					 ioBuffer[0],
					 ioBuffer[1],
					 ioBuffer[2],
					 ioBuffer[3],
					 ioBuffer[4],
					 ioBuffer[5],
					 ioBuffer[6],
					 ioBuffer[7],
					 ioBuffer[8],
					 ioBuffer[9]);
				 NdisReleaseSpinLock(&NetGateListLock);
				 break;	
			case IADS_DRIVER_ADDCODE://添加特征码
				DbgPrint("特征码数量:%d",inputBufferLength/32);
				for(i=0;i<inputBufferLength/32;i++)
				{
					NdisAcquireSpinLock(&HashLinkLock);
					md5_insert((char *)ioBuffer+i*32);
					NdisReleaseSpinLock(&HashLinkLock);
				}
				break;
			case IADS_NEWATTACK_EVENT://读取新的攻击日志
				DbgPrint("有新的攻击日志!");
				NdisAcquireSpinLock(&AttackInforLock);				
				NdisMoveMemory(Irp->UserBuffer,&attack_infor,sizeof(Attack_Infor));
				NdisZeroMemory (&attack_infor, sizeof(Attack_Infor));
				NdisReleaseSpinLock(&AttackInforLock);
				Irp->IoStatus.Status = STATUS_SUCCESS;
				Irp->IoStatus.Information=sizeof(Attack_Infor);
				IoCompleteRequest(Irp, IO_NO_INCREMENT);
				return status;
			case IO_REFERENCE_EVENT://传递通信事件
				hEvent = (HANDLE)irpStack->Parameters.DeviceIoControl.Type3InputBuffer;
				status = ObReferenceObjectByHandle(
					hEvent,
					GENERIC_ALL,
					NULL,
					KernelMode,
					&gpEventObject,
					&objHandleInfo);
				if(status!= STATUS_SUCCESS)
				{
					DbgPrint("ObReferenceObjectByHandle failed! status = %x\n", status);
					break;
				}
				DbgPrint("Referenct object sussfully!\n");
				break;
			case IO_CLEAR_EVENT://清除通信事件
				if(gpEventObject)
				{
					KeClearEvent(gpEventObject);
					ObDereferenceObject(gpEventObject); 
					gpEventObject=NULL;
				}
				DbgPrint("KeClearEvent sussfully!\n");
				break;
			default:
				 status = STATUS_NOT_SUPPORTED;
				 break;
			} 
          break;        
        default:
           break;
    }
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(("<== Pt Dispatch\n"));

    return status;
} 

NDIS_STATUS
PtDeregisterDevice(
    VOID
    )
/*++

Routine Description:

    Deregister the ioctl interface. This is called whenever a miniport
    instance is halted. When the last miniport instance is halted, we
    request NDIS to delete the device object

Arguments:

    NdisDeviceHandle - Handle returned by NdisMRegisterDevice

Return Value:

    NDIS_STATUS_SUCCESS if everything worked ok

--*/
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    DBGPRINT(("==>PassthruDeregisterDevice\n"));

    NdisAcquireSpinLock(&GlobalLock);

    ASSERT(MiniportCount > 0);

    --MiniportCount;
    
    if (0 == MiniportCount)
    {
        //
        // All miniport instances have been halted. Deregister
        // the control device.
        //

        ASSERT(ControlDeviceState == PS_DEVICE_STATE_READY);

        //
        // Block PtRegisterDevice() while we release the control
        // device lock and deregister the device.
        // 
        ControlDeviceState = PS_DEVICE_STATE_DELETING;

        NdisReleaseSpinLock(&GlobalLock);

        if (NdisDeviceHandle != NULL)
        {
            Status = NdisMDeregisterDevice(NdisDeviceHandle);
            NdisDeviceHandle = NULL;
        }

        NdisAcquireSpinLock(&GlobalLock);
        ControlDeviceState = PS_DEVICE_STATE_READY;
    }

    NdisReleaseSpinLock(&GlobalLock);

    DBGPRINT(("<== PassthruDeregisterDevice: %x\n", Status));
    return Status;
    
}

VOID
PtUnload(
    IN PDRIVER_OBJECT        DriverObject
    )
//
// PassThru driver unload function
//
{
    UNREFERENCED_PARAMETER(DriverObject);
    
    DBGPRINT(("PtUnload: entered\n"));
    PtUnloadProtocol();
    NdisIMDeregisterLayeredMiniport(DriverHandle);
    NdisFreeSpinLock(&GlobalLock);
    DBGPRINT(("PtUnload: done!\n"));
}

