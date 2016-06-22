/*++
 
ģ������:
 
    hashlink.c

    ������������ϵͳ-NDIS�м����������-��ϣ�����ڴ�������룩

--*/

#include "precomp.h"
#include "hashlink.h"
PNode md5_array[MD5LENGTH]={0};//����
NDIS_PHYSICAL_ADDRESS	HighestAcceptableMax= NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

/*��ϣ�������ĳ��λ�ò���source�����ַ���ĩ�ַ���д�ַ�*/
int getIndex(char *source)
{
	int index=source[0]>'9'?(source[0]-'a'):(source[0]-'0');
	index<<=4;
	index+=source[31]>'9'?(source[31]-'a'):(source[31]-'0');

	return index;
}
/**str1 == str2 return 0*/
int mycmp(char* str1,char*str2)
{
	int i;
	for(i=0;i<32;i++)
		if(str1[i]!=str2[i])
			return -1;
	return 0;
}
int md5_insert(char *source)
{
	int i;
	int index=getIndex(source);
	PNode node=md5_array[index];
  NDIS_STATUS Status;
	PUCHAR     pBuf;
	
	DbgPrint("����λ�ã�%d \n",getIndex(source));

	//��һ���ڵ�
	if(!node)
	{
		Status= NdisAllocateMemory(&pBuf, sizeof(md5_node), 0,HighestAcceptableMax);
		if (Status!=NDIS_STATUS_SUCCESS ) return 1;
		md5_array[index]=(PNode)pBuf;
		NdisZeroMemory (pBuf,sizeof(md5_node));

		md5_array[index]->next=NULL;
		for(i=0;i<32;i++)
			md5_array[index]->md5[i]=source[i];
		return 0;
	}

	//�嵽�����
	while(node->next) 
		node=node->next;

	Status= NdisAllocateMemory(&pBuf, sizeof(md5_node), 0,HighestAcceptableMax);
	if (Status!=NDIS_STATUS_SUCCESS ) return 1;
	node->next=(PNode)pBuf;
	NdisZeroMemory (pBuf,sizeof(md5_node));

	node->next->next=NULL;
	
	for(i=0;i<32;i++)
		node->next->md5[i]=source[i];

	return 0;

}

/*1:�ҵ�  0��û�ҵ�*/
int md5_search(char *source)
{
	
	PNode node=md5_array[getIndex(source)];

	while(node&&mycmp(node->md5,source)!=0) 
		node=node->next;

	if(node)
		return 1;
	return 0;
}
