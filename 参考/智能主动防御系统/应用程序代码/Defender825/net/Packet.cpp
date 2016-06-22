#include "stdafx.h"
//计算校验和
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
	//将32位数转换成16
	while (cksum>>16)
		cksum=(cksum>>16)+(cksum & 0xffff);
	return (USHORT) (~cksum);
}
