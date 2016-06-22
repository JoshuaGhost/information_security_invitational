typedef struct hostlist
{
	unsigned int ip;
	unsigned char mac[6];
	struct hostlist * next;

}HostListNode,*HostListLink;
//创建带有头节点的链表
int CreateHostList(HostListLink *host);
//头插法，插入链表
int InsertHostList(HostListLink host,unsigned int ip,unsigned char mac[6]);
//查询指ip是否存在
//返回值:0-不存在,1-存在，并且mac地址匹配,2-存在,并且mac地址不匹配
int FindHostList(HostListLink host,unsigned int ip,unsigned char mac[6]);