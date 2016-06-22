//数据包结构体
#pragma pack(1)  
/*物理帧头结构*/
typedef struct {
   UCHAR  desmac[6];      //目的MAC地址
   UCHAR  srcmac[6];      //源MAC地址
   USHORT  ethertype;    //帧类型
}Dlc_Header; 

/*Arp帧结构*/
typedef struct {
   USHORT hw_type;       //硬件类型Ethernet:0x1
   USHORT prot_type;     //上层协议类型IP:0x0800
   UCHAR hw_addr_len;     //硬件地址长度:6
   UCHAR prot_addr_len;   //协议地址(IP地址)的长度:4
   USHORT flag;          //1表示请求,2表示应答
   UCHAR send_hw_addr[6]; //源MAC地址
   UINT send_prot_addr;  //源IP地址
   UCHAR targ_hw_addr[6]; //目的MAC地址
   UINT targ_prot_addr;  //目的IP地址
   UCHAR padding[18];     //填充数据   
}Arp_Frame;

/*ARP数据包*/
typedef struct {
	Dlc_Header dlcheader;//DLC头
	Arp_Frame arpframe;  //ARP帧
}ARP_Packet;

/*IP报头结构*/
typedef struct {
	UCHAR  ver_len;       //IP包头部长度,单位：4字节
	UCHAR  tos;           //服务类型TOS
	USHORT total_len;    //IP包总长度  
	USHORT ident;        //标识
	USHORT frag_and_flags;  //标志位
	UCHAR ttl;           //生存时间
	UCHAR proto;         //协议
	USHORT checksum;    //IP首部校验和
	UINT  sourceIP;  //源IP地址(32位)
	UINT  destIP;    //目的IP地址(32位)
}Ip_Header;

/*TCP报头结构*/
typedef struct { 
	USHORT srcport;   // 源端口
	USHORT dstport;   // 目的端口
	UINT seqnum;      // 顺序号
	UINT acknum;      // 确认号
	UCHAR dataoff;     // TCP头长
	UCHAR flags;       // 标志（URG、ACK等）
	USHORT window;    // 窗口大小
	USHORT chksum;    // 校验和
	USHORT urgptr;    // 紧急指针
}Tcp_Header;

/*TCP数据包*/
typedef struct{
	Dlc_Header dlc_header; //以太帧
	Ip_Header ip_header;   //IP头
	Tcp_Header tcp_header; //TCP头
}Tcp_Packet;

/*UDP报头*/
typedef struct  {  
 USHORT srcport;     // 源端口
 USHORT dstport;     // 目的端口
 USHORT total_len;   // 包括UDP报头及UDP数据的长度(单位:字节)
 USHORT chksum;      // 校验和
}Udp_Header;

/*UDP数据包*/
typedef struct {
	Dlc_Header dlc_header; //以太帧
	Ip_Header ip_header;   //IP头
	Udp_Header udp_header; //udp头
}Udp_Packet;

/*ICMP报头*/
typedef struct{
	UCHAR i_type;     //类型 类型是关键:0->回送应答(Ping应答) 8->回送请求(Ping请求)
	UCHAR i_code;     //代码 这个与类型有关 当类型为0或8时这里都是0
	USHORT i_cksum;  //ICMP包校验和
	USHORT i_id;     //识别号(一般用进程ID作为标识号)
	USHORT i_seq;    //报文序列号(一般设置为0)
	//UINT timestamp;  //时间戳 
	UCHAR padding[32];//填充数据 
}Icmp_Header;

/*ICMP数据包*/
typedef struct
{
	Dlc_Header dlc_header;  //以太帧
	Ip_Header  ip_header;   //IP头
	Icmp_Header icmp_header;//ICMP帧
}Icmp_Packet;
/*攻击信息*/
typedef struct
{
	unsigned char flag;     //攻击数据包类型1-arp,2-tcp,3-udp
	unsigned int srcip;     //攻击者IP
	unsigned char code[33]; //攻击特征码
}Attack_Infor;
#pragma pack()  
