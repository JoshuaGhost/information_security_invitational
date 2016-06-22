typedef unsigned int ElemType;
//集合节点
typedef struct CNode{
	ElemType data;     //集合节点的数据域
	BYTE	 flag;     //是否被修改(1被修改,0未被修改)
    struct CNode *next;//集合节点节点指针域，指向下一个节点
}CNode,*Collection;

//创建集合 带头结点
int CreateCollection(Collection &coll);
//在链表头部 添加节点
int InsertColletion(Collection &coll,unsigned int dat);
//在链表中查找某数据值是否存在 如果找到返回位置i,否则返回0
int ExistCollection(Collection coll,unsigned int dat);
//删除链表中 值为dat的节点
int DeleteNode(Collection &coll,unsigned int dat);
//删除除头结点外的其余所有节点
int DeleteCollection(Collection &coll);
//删除标志flag=0的节点
int DeleteFlagZNode(Collection &coll);
//将所有节点的标志位置0
int ResetNodeFlag(Collection &coll);
//查找值为dat的节点是否存在,存在返回位置i，否则返回0,不影响标志位
int FindNode(Collection coll,unsigned int dat);
