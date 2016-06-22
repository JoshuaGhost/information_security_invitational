#define MD5LENGTH 256
typedef struct md5_node
{
	char md5[32];
	struct md5_node* next;
}md5_node,*PNode;
/*哈希到数组的某个位置
采用source的首字符和末字符
大写字符
  */
int getIndex(char *source);
/**str1 == str2 return 0*/
int mycmp(char* str1,char*str2);
int md5_insert(char *source);
/**
 1:找到  0：没找到
**/
int md5_search(char *source);