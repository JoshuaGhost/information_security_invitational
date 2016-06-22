#include "stdafx.h"
#include <stdlib.h>
#include <malloc.h>
#include "collection.h"

//创建集合 带头结点
int CreateCollection(Collection &coll)
{
	coll=(Collection)malloc(sizeof(CNode));
	if(!coll)	return 1;
	coll->data=0;     //头结点数据区
	coll->flag=0;
	coll->next=NULL;  //头结点指针区
	return 0;
}
//在链表头部 添加节点
int InsertColletion(Collection &coll,unsigned int dat)
{
	CNode *p;
	if(!ExistCollection(coll,dat))//如果集合中不存在dat
	{
		p=(Collection)malloc(sizeof(CNode));
		if(!p) return 1;
		p->data=dat;
		p->flag=1;       //被修改
		p->next=coll->next;
		coll->next=p;
	}
	return 0;
}
//在链表中查找某数据值是否存在 如果找到返回位置i,并将flag置为1,否则返回0
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
//删除链表中 值为dat的节点
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
//删除标志flag=0的节点
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
//将所有节点的标志位置0
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
//删除除头结点外的其余所有节点
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
//查找值为dat的节点是否存在,存在返回位置i，否则返回0,不影响标志位
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