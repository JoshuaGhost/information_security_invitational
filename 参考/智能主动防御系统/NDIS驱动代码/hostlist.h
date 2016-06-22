typedef struct hostlist
{
	unsigned int ip;
	unsigned char mac[6];
	struct hostlist * next;

}HostListNode,*HostListLink;
//��������ͷ�ڵ������
int CreateHostList(HostListLink *host);
//ͷ�巨����������
int InsertHostList(HostListLink host,unsigned int ip,unsigned char mac[6]);
//��ѯָip�Ƿ����
//����ֵ:0-������,1-���ڣ�����mac��ַƥ��,2-����,����mac��ַ��ƥ��
int FindHostList(HostListLink host,unsigned int ip,unsigned char mac[6]);