; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDangerDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "defender.h"
LastPage=0

ClassCount=23
Class1=CAdvanceSetting
Class2=CAutorunPage
Class3=CBasicSheet
Class4=CConnectPage
Class5=CDangerDlg
Class6=CDefenderApp
Class7=CDefenderDoc
Class8=CDefenderView
Class9=CEdithostsDlg
Class10=CHostsPage
Class11=CLoadcodeDlg
Class12=CLocalPage
Class13=CLogSheet
Class14=CMainFrame
Class15=CModuleDlg
Class16=CNetlogPage
Class17=CProcessPage
Class18=CProtectSetPage
Class19=CProtectSheet
Class20=CServicePage
Class21=CSetSheet

ResourceCount=28
Resource1=IDR_IPLIST
Resource2=IDR_PORTLIST
Resource3=IDD_DANGER
Resource4=IDR_AUTORUN
Resource5=IDD_PROTECT_AUTORUN
Resource6=IDR_NETLOG
Resource7=IDR_PROCESS
Resource8=IDR_TRAY
Resource9=IDR_SERVICE
Resource10=IDD_PROTECT_SET
Resource11=IDR_HOTBAR
Resource12=IDD_ADVANCESETTING_DIALOG
Resource13=IDR_CONNECT
Resource14=IDD_BASIC_CONNECT
Resource15=IDD_BASIC_PROCESS
Resource16=IDD_BASIC_MODULE
Resource17=IDD_PROTECT_HOSTS
Resource18=IDD_PROTECT_DIALOG
Resource19=IDD_EDITHOSTS
Resource20=IDD_LOADCODE
Resource21=IDR_MAINFRAME
Resource22=IDD_PROTECT_NETLOG
Resource23=IDR_HOSTS
Resource24=IDR_MESLIST
Resource25=IDD_PROTECT_SERVICE
Resource26=IDD_ABOUTBOX
Class22=CProtectFile
Resource27=IDD_BASIC_LOCAL
Class23=CWhiteFile
Resource28=IDD_WHITE_DIALOG

[CLS:CAdvanceSetting]
Type=0
BaseClass=CDialog
HeaderFile=AdvanceSetting.h
ImplementationFile=AdvanceSetting.cpp
LastObject=CAdvanceSetting

[CLS:CAutorunPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=AutorunPage.h
ImplementationFile=AutorunPage.cpp

[CLS:CBasicSheet]
Type=0
BaseClass=CPropertySheet
HeaderFile=BasicSheet.h
ImplementationFile=BasicSheet.cpp

[CLS:CConnectPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=ConnectPage.h
ImplementationFile=ConnectPage.cpp

[CLS:CDangerDlg]
Type=0
BaseClass=CDialog
HeaderFile=DangerDlg.h
ImplementationFile=DangerDlg.cpp
LastObject=IDC_INFO_STATIC
Filter=D
VirtualFilter=dWC

[CLS:CDefenderApp]
Type=0
BaseClass=CWinApp
HeaderFile=Defender.h
ImplementationFile=Defender.cpp

[CLS:CDefenderDoc]
Type=0
BaseClass=CDocument
HeaderFile=DefenderDoc.h
ImplementationFile=DefenderDoc.cpp

[CLS:CDefenderView]
Type=0
BaseClass=CView
HeaderFile=DefenderView.h
ImplementationFile=DefenderView.cpp

[CLS:CEdithostsDlg]
Type=0
BaseClass=CDialog
HeaderFile=EdithostsDlg.h
ImplementationFile=EdithostsDlg.cpp

[CLS:CHostsPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=HostsPage.h
ImplementationFile=HostsPage.cpp

[CLS:CLoadcodeDlg]
Type=0
BaseClass=CDialog
HeaderFile=LoadcodeDlg.h
ImplementationFile=LoadcodeDlg.cpp

[CLS:CLocalPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=LocalPage.h
ImplementationFile=LocalPage.cpp
Filter=D
VirtualFilter=idWC
LastObject=CLocalPage

[CLS:CLogSheet]
Type=0
BaseClass=CPropertySheet
HeaderFile=LogSheet.h
ImplementationFile=LogSheet.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
LastObject=IDR_EXIT
Filter=T
VirtualFilter=fWC

[CLS:CModuleDlg]
Type=0
BaseClass=CDialog
HeaderFile=ModuleDlg.h
ImplementationFile=ModuleDlg.cpp

[CLS:CNetlogPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=NetlogPage.h
ImplementationFile=NetlogPage.cpp

[CLS:CProcessPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=ProcessPage.h
ImplementationFile=ProcessPage.cpp

[CLS:CProtectSetPage]
Type=0
BaseClass=CPropertyPage
HeaderFile=ProtectSetPage.h
ImplementationFile=ProtectSetPage.cpp
Filter=D
VirtualFilter=idWC
LastObject=IDM_HELP

[CLS:CProtectSheet]
Type=0
BaseClass=CPropertySheet
HeaderFile=ProtectSheet.h
ImplementationFile=ProtectSheet.cpp

[CLS:CServicePage]
Type=0
BaseClass=CPropertyPage
HeaderFile=ServicePage.h
ImplementationFile=ServicePage.cpp

[CLS:CSetSheet]
Type=0
BaseClass=CPropertySheet
HeaderFile=SetSheet.h
ImplementationFile=SetSheet.cpp

[DLG:IDD_ADVANCESETTING_DIALOG]
Type=1
Class=CAdvanceSetting
ControlCount=22
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,static,1342308352
Control5=IDC_OSLIST,combobox,1344340226
Control6=IDC_STATIC,button,1342177287
Control7=IDC_ICMPACCEPT,button,1342242819
Control8=IDC_PORTLIST,SysListView32,1350631425
Control9=IDC_ADDPORT,button,1342242816
Control10=IDC_STATIC,button,1342177287
Control11=IDC_STATIC,static,1342308352
Control12=IDC_PORTNUM,edit,1350631552
Control13=IDC_STATIC,static,1342308352
Control14=IDC_PROTOCOLLIST,combobox,1344340226
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATELIST,combobox,1344340226
Control17=IDC_STATIC,static,1342308352
Control18=IDC_SCRIPTLIST,combobox,1344340226
Control19=IDC_SAVEHOST,button,1342242816
Control20=IDC_IPADDRESS,SysIPAddress32,1342242816
Control21=IDC_ADDHOST,button,1342242816
Control22=IDC_IPLIST,SysListView32,1350631427

[DLG:IDD_PROTECT_AUTORUN]
Type=1
Class=CAutorunPage
ControlCount=1
Control1=IDC_AUTORUN_LIST,SysListView32,1350631425

[DLG:IDD_BASIC_CONNECT]
Type=1
Class=CConnectPage
ControlCount=1
Control1=IDC_CONNECT_LIST,SysListView32,1350631425

[DLG:IDD_DANGER]
Type=1
Class=CDangerDlg
ControlCount=7
Control1=IDC_INFO_ETIT,edit,1352728580
Control2=IDC_END_BUTTON,button,1342177289
Control3=IDC_REFUSE_BUTTON,button,1342177289
Control4=IDC_ADD_BUTTON,button,1342177289
Control5=IDC_IGNORE_BUTTON,button,1342177289
Control6=IDOK,button,1342242817
Control7=IDC_INFO_STATIC,static,1342308364

[DLG:IDD_EDITHOSTS]
Type=1
Class=CEdithostsDlg
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_CHECK,button,1342242819
Control4=IDC_STATIC,static,1342308352
Control5=IDC_IPADDRESS,SysIPAddress32,1342242816
Control6=IDC_DOMAIN,edit,1350631552
Control7=IDC_STATIC,static,1342308352

[DLG:IDD_PROTECT_HOSTS]
Type=1
Class=CHostsPage
ControlCount=1
Control1=IDC_HOSTS_LIST,SysListView32,1350631427

[DLG:IDD_LOADCODE]
Type=1
Class=CLoadcodeDlg
ControlCount=1
Control1=IDC_PERCENT,static,1342308352

[DLG:IDD_BASIC_LOCAL]
Type=1
Class=CLocalPage
ControlCount=27
Control1=IDC_STATIC,static,1342308352
Control2=IDC_ADAPTER_LIST,combobox,1344340227
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_MACADDR,static,1342308352
Control11=IDC_IPADDR,static,1342308352
Control12=IDC_NETMASK,static,1342308352
Control13=IDC_NETGATE,static,1342308352
Control14=IDC_NETGATEMAC,static,1342308352
Control15=IDC_STATIC,button,1342177287
Control16=IDC_STATIC,static,1342308352
Control17=IDC_MESLIST,SysListView32,1350631425
Control18=IDC_STATIC,static,1342308352
Control19=IDC_PACKETFILTER,static,1342308352
Control20=IDC_STATIC,static,1342308352
Control21=IDC_ARPFILTER,static,1342308352
Control22=IDC_STATIC,static,1342308352
Control23=IDC_ICMPFILTER,static,1342308352
Control24=IDC_STATIC,static,1342308352
Control25=IDC_SCCOUNT,static,1342308352
Control26=IDC_STATIC,static,1342308352
Control27=IDC_PCOUNT,static,1342308352

[DLG:IDD_BASIC_MODULE]
Type=1
Class=CModuleDlg
ControlCount=6
Control1=IDOK,button,1342242817
Control2=IDC_MODULE_LIST,SysListView32,1350631425
Control3=IDC_REFRESH_BUTTON,button,1342242816
Control4=IDC_SOFT_BUTTON,button,1342242816
Control5=IDC_FORCEALL_BUTTON,button,1342251008
Control6=IDC_FORCE_BUTTON,button,1342242816

[DLG:IDD_PROTECT_NETLOG]
Type=1
Class=CNetlogPage
ControlCount=1
Control1=IDC_ATTACKLOG_LIST,SysListView32,1350631425

[DLG:IDD_BASIC_PROCESS]
Type=1
Class=CProcessPage
ControlCount=1
Control1=IDC_PROCESS_LIST,SysListView32,1350631425

[DLG:IDD_PROTECT_SET]
Type=1
Class=CProtectSetPage
ControlCount=11
Control1=IDC_STATIC,button,1342177287
Control2=IDC_PACKETFILTER_CHECK,button,1342242819
Control3=IDC_ANTIARP_CHECK,button,1342242819
Control4=IDC_ICMPFILTER_CHECK,button,1342242819
Control5=IDC_APPLY,button,1342242816
Control6=IDC_ADVANCESETTING,button,1342242816
Control7=IDC_PLAYALLROLE,button,1342242819
Control8=IDC_WHITE_BUTTON,button,1342242816
Control9=IDC_PROTECT_BUTTON,button,1342242816
Control10=IDC_STATIC,button,1342177287
Control11=IDC_BEGIN_BUTTON,button,1342242819

[DLG:IDD_PROTECT_SERVICE]
Type=1
Class=CServicePage
ControlCount=1
Control1=IDC_SERVICE_LIST,SysListView32,1350631425

[TB:IDR_HOTBAR]
Type=1
Class=?
Command1=IDM_PROTECT
Command2=IDM_SET
Command3=IDM_LOG
Command4=IDM_HELP
CommandCount=4

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=IDM_PROTECT
Command2=IDM_SET
Command3=IDM_LOG
Command4=IDM_HELP
CommandCount=4

[MNU:IDR_PROCESS]
Type=1
Class=?
Command1=IDM_PROCESS_REFRESH
Command2=IDM_PROCESS_END
Command3=IDM_PROCESS_MODULE
CommandCount=3

[MNU:IDR_CONNECT]
Type=1
Class=?
Command1=IDM_CONNECT_REFRESH
Command2=IDM_CONNECT_END
CommandCount=2

[MNU:IDR_NETLOG]
Type=1
Class=?
Command1=IDM_ATTACKLOG_REFRESH
Command2=IDM_ATTACKLOG_SAVE
Command3=IDM_ATTACKLOG_CLEARALL
CommandCount=3

[MNU:IDR_TRAY]
Type=1
Class=?
Command1=IDM_HELP
Command2=IDR_OPEN
Command3=IDR_EXIT
CommandCount=3

[MNU:IDR_SERVICE]
Type=1
Class=?
Command1=IDM_SERVICE_REFRESH
Command2=IDM_SERVICE_START
Command3=IDM_SERVICE_STOP
Command4=IDM_SERVICE_AUTO
Command5=IDM_SERVICE_MANUAL
Command6=IDM_SERVICE_DISABLE
CommandCount=6

[MNU:IDR_AUTORUN]
Type=1
Class=?
Command1=IDM_AUTORUN_REFRESH
Command2=IDM_AUTORUN_DELETE
CommandCount=2

[MNU:IDR_HOSTS]
Type=1
Class=?
Command1=IDM_HOSTS_NEW
Command2=IDM_HOSTS_CHANGE
Command3=IDM_HOSTS_DELETE
Command4=IDM_HOSTS_REFRESH
CommandCount=4

[MNU:IDR_PORTLIST]
Type=1
Class=?
Command1=IDM_DEL
CommandCount=1

[MNU:IDR_IPLIST]
Type=1
Class=?
Command1=IDM_NEWHOST
Command2=IDM_DELHOST
CommandCount=2

[MNU:IDR_MESLIST]
Type=1
Class=?
Command1=IDM_NETSHOW
Command2=IDM_HOSTSHOW
Command3=IDM_DELETE
Command4=IDM_DELETEALL
CommandCount=4

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[DLG:IDD_ABOUTBOX]
Type=1
Class=?
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_PROTECT_DIALOG]
Type=1
Class=CProtectFile
ControlCount=7
Control1=IDC_PROTECT_LIST,SysListView32,1350631424
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDC_PROTECT_EDIT,edit,1350631552
Control5=IDC_PROTECT_EDIT2,edit,1350631552
Control6=IDC_ADD_BUTTON,button,1342242816
Control7=IDC_DELETE_BUTTON,button,1342242816

[CLS:CProtectFile]
Type=0
HeaderFile=ProtectFile.h
ImplementationFile=ProtectFile.cpp
BaseClass=CDialog
Filter=D
LastObject=CProtectFile
VirtualFilter=dWC

[CLS:CWhiteFile]
Type=0
HeaderFile=WhiteFile.h
ImplementationFile=WhiteFile.cpp
BaseClass=CDialog
Filter=D
LastObject=CWhiteFile
VirtualFilter=dWC

[DLG:IDD_WHITE_DIALOG]
Type=1
Class=CWhiteFile
ControlCount=7
Control1=IDC_INFO_LIST,SysListView32,1350631424
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDC_PROC_EDIT,edit,1350631552
Control5=IDC_ACT_EDIT,edit,1350631552
Control6=IDC_ADD_BUTTON,button,1342242816
Control7=IDC_DELETE_BUTTON,button,1342242816

