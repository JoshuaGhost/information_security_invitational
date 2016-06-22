#include "stdafx.h"
#include <stdlib.h>
#include <malloc.h>
#include "collection.h"

//�������� ��ͷ���
int CreateCollection(Collection &coll)
{
	coll=(Collection)malloc(sizeof(CNode));
	if(!coll)	return 1;
	coll->data=0;     //ͷ���������
	coll->flag=0;
	coll->next=NULL;  //ͷ���ָ����
	return 0;
}
//������ͷ�� ��ӽڵ�
int InsertColletion(Collection &coll,unsigned int dat)
{
	CNode *p;
	if(!ExistCollection(coll,dat))//��������в�����dat
	{
		p=(Collection)malloc(sizeof(CNode));
		if(!p) return 1;
		p->data=dat;
		p->flag=1;       //���޸�
		p->next=coll->next;
		coll->next=p;
	}
	return 0;
}
//�������в���ĳ����ֵ�Ƿ���� ����ҵ�����λ��i,����flag��Ϊ1,���򷵻�0
int ExistCollection(Collection coll,unsigned int dat)
{
	CNode *p;
	int i=0;
	p=coll->next;
	while(p)
	{
		i+=1;
		if(dat==p->data) 
		{
			p->flag=1;
			return i;
		}
		p=p->next;
	}
	return 0;
}
//ɾ�������� ֵΪdat�Ľڵ�
int DeleteNode(Collection &coll,unsigned int dat)
{
	CNode * p,*q;
	q=coll;
	p=q->next;
	while(p)
	{
		if(dat==p->data)
		{
			q->next=p->next;
			free(p);
			return 1;
		}
		q=q->next;
		p=q->next;
	}
	return 0;
}
//ɾ����־flag=0�Ľڵ�
int DeleteFlagZNode(Collection &coll)
{
	CNode *p,*q;
	q=coll;
	p=q->next;
	while(p)
	{
		if(!(p->flag))
		{
			q->next=p->next;
			free(p);
		}
		q=q->next;
		p=q->next;
	}
	return 0;
}
//�����нڵ�ı�־λ��0
int ResetNodeFlag(Collection &coll)
{
	CNode *p;
	p=coll->next;
	while(p)
	{
		p->flag=0;
		p=p->next;
	}
	return 0;
}
//ɾ����ͷ�������������нڵ�
int DeleteCollection(Collection &coll)
{
	CNode *p,*q;
	p=coll->next;
	while(p)
	{
		q=p->next;
		free(p);
		p=q;
	}
	return 0;
}
//����ֵΪdat�Ľڵ��Ƿ����,���ڷ���λ��i�����򷵻�0,��Ӱ���־λ
int FindNode(Collection coll,unsigned int dat)
{
	CNode *p;
	int i=0;
	p=coll->next;
	while(p)
	{
		i+=1;
		if(dat==p->data) return i;
		p=p->next;
	}
	return 0;
}