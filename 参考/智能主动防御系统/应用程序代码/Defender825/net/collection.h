typedef unsigned int ElemType;
//���Ͻڵ�
typedef struct CNode{
	ElemType data;     //���Ͻڵ��������
	BYTE	 flag;     //�Ƿ��޸�(1���޸�,0δ���޸�)
    struct CNode *next;//���Ͻڵ�ڵ�ָ����ָ����һ���ڵ�
}CNode,*Collection;

//�������� ��ͷ���
int CreateCollection(Collection &coll);
//������ͷ�� ��ӽڵ�
int InsertColletion(Collection &coll,unsigned int dat);
//�������в���ĳ����ֵ�Ƿ���� ����ҵ�����λ��i,���򷵻�0
int ExistCollection(Collection coll,unsigned int dat);
//ɾ�������� ֵΪdat�Ľڵ�
int DeleteNode(Collection &coll,unsigned int dat);
//ɾ����ͷ�������������нڵ�
int DeleteCollection(Collection &coll);
//ɾ����־flag=0�Ľڵ�
int DeleteFlagZNode(Collection &coll);
//�����нڵ�ı�־λ��0
int ResetNodeFlag(Collection &coll);
//����ֵΪdat�Ľڵ��Ƿ����,���ڷ���λ��i�����򷵻�0,��Ӱ���־λ
int FindNode(Collection coll,unsigned int dat);
