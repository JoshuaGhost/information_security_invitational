//���ݰ��ṹ��
#pragma pack(1)  
/*����֡ͷ�ṹ*/
typedef struct {
   BYTE  desmac[6];      //Ŀ��MAC��ַ
   BYTE  srcmac[6];      //ԴMAC��ַ
   USHORT  ethertype;    //֡����
}Dlc_Header; 

/*Arp֡�ṹ*/
typedef struct {
   USHORT hw_type;       //Ӳ������Ethernet:0x1
   USHORT prot_type;     //�ϲ�Э������IP:0x0800
   BYTE hw_addr_len;     //Ӳ����ַ����:6
   BYTE prot_addr_len;   //Э���ַ(IP��ַ)�ĳ���:4
   USHORT flag;          //1��ʾ����,2��ʾӦ��
   BYTE send_hw_addr[6]; //ԴMAC��ַ
   UINT send_prot_addr;  //ԴIP��ַ
   BYTE targ_hw_addr[6]; //Ŀ��MAC��ַ
   UINT targ_prot_addr;  //Ŀ��IP��ַ
   BYTE padding[18];     //�������   
}Arp_Frame;

/*ARP��=DLCͷ+ARP֡*/
typedef struct {
	Dlc_Header dlcheader;//DLCͷ
	Arp_Frame arpframe;  //ARP֡
}ARP_Packet;

/*IP��ͷ�ṹ*/
typedef struct {
	BYTE  ver_len;       //IP��ͷ������,��λ��4�ֽ�
	BYTE  tos;           //��������TOS
	USHORT total_len;    //IP���ܳ���  
	USHORT ident;        //��ʶ
	USHORT frag_and_flags;  //��־λ
	BYTE ttl;           //����ʱ��
	BYTE proto;         //Э��
	USHORT checksum;    //IP�ײ�У���
	UINT  sourceIP;  //ԴIP��ַ(32λ)
	UINT  destIP;    //Ŀ��IP��ַ(32λ)
}Ip_Header;

/*TCP��ͷ�ṹ*/
typedef struct { 
	USHORT srcport;   // Դ�˿�
	USHORT dstport;   // Ŀ�Ķ˿�
	UINT seqnum;      // ˳���
	UINT acknum;      // ȷ�Ϻ�
	BYTE dataoff;     // TCPͷ��
	BYTE flags;       // ��־��URG��ACK�ȣ�
	USHORT window;    // ���ڴ�С
	USHORT chksum;    // У���
	USHORT urgptr;    // ����ָ��
}Tcp_Header;

//TCPα�ײ� ���ڽ���TCPУ��͵ļ���,��֤TCPЧ�����Ч��
typedef struct{
 ULONG  sourceip;    //ԴIP��ַ
 ULONG  destip;      //Ŀ��IP��ַ
 BYTE mbz;           //�ÿ�(0)
 BYTE ptcl;          //Э������(IPPROTO_TCP)
 USHORT tcpl;        //TCP���ܳ��ȳ���(��λ:�ֽ�)
}Tcp_Psd_Header;

/*UDP��ͷ*/
typedef struct  {  
 USHORT srcport;     // Դ�˿�
 USHORT dstport;     // Ŀ�Ķ˿�
 USHORT total_len;   // ����UDP��ͷ��UDP���ݵĳ���(��λ:�ֽ�)
 USHORT chksum;      // У���
}Udp_Header;

/*UDPα�ײ�-�����ڼ���У���*/
typedef struct tsd_hdr  
{  
 BYTE  sourceip[4];   //ԴIP��ַ
 BYTE  destip[4];     //Ŀ��IP��ַ
 BYTE  mbz;           //�ÿ�(0)
 BYTE  ptcl;          //Э������(IPPROTO_UDP)
 USHORT udpl;         //UDP���ܳ���(��λ:�ֽ�)  
}Udp_Psd_Header; 

/*ICMP��ͷ*/
typedef struct{
	BYTE i_type;     //���� �����ǹؼ�:0->����Ӧ��(PingӦ��) 8->��������(Ping����)
	BYTE i_code;     //���� ����������й� ������Ϊ0��8ʱ���ﶼ��0
	USHORT i_cksum;  //ICMP��У���
	USHORT i_id;     //ʶ���(һ���ý���ID��Ϊ��ʶ��)
	USHORT i_seq;    //�������к�(һ������Ϊ0)
	//UINT timestamp;  //ʱ��� 
	BYTE padding[32];//������� 
}Icmp_Header;
/*ICMP���ݰ�*/
typedef struct
{
	Dlc_Header dlc_header;  //��̫֡
	Ip_Header  ip_header;   //IPͷ
	Icmp_Header icmp_header;//ICMP֡
}Icmp_Packet;

/*������Ϣ*/
typedef struct
{
	unsigned char flag;     //�������ݰ�����1-arp,2-tcp,3-udp
	unsigned int srcip;     //������IP
	unsigned char code[33]; //����������
}Attack_Infor;
#pragma pack()  
