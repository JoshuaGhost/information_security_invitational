#define MD5LENGTH 256
typedef struct md5_node
{
	char md5[32];
	struct md5_node* next;
}md5_node,*PNode;
/*��ϣ�������ĳ��λ��
����source�����ַ���ĩ�ַ�
��д�ַ�
  */
int getIndex(char *source);
/**str1 == str2 return 0*/
int mycmp(char* str1,char*str2);
int md5_insert(char *source);
/**
 1:�ҵ�  0��û�ҵ�
**/
int md5_search(char *source);