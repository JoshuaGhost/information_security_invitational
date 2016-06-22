#include "stdafx.h"
//����У���
USHORT checksum(USHORT *buffer,int size)
{
	unsigned long cksum=0;
	while(size>1)
	{
		cksum+=*buffer++;
		size-=sizeof(USHORT);
	}
	if(size)
	{
		cksum+=*(UCHAR *)buffer;
	}
	//��32λ��ת����16
	while (cksum>>16)
		cksum=(cksum>>16)+(cksum & 0xffff);
	return (USHORT) (~cksum);
}
