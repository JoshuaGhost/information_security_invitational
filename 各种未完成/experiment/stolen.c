/*
 *	author:Zhang Zijian
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	This program is writed to authorize data group from internet to PC by
 *	banning all of actibe connection requests from outer cyber
 *
 *	Attention: This is a kernel program. If you want to run this, you have
 *	to be a root or a superuser
 *
 */

#include<linux/module.h>
#include<linux/init.h>
#include<linux/skbuff.h>
#include<linux/netfilter.h>
#include<linux/netfilter_ipv4.h>
#include<linux/string.h>
#include<linux/ip.h>
#include<linux/in.h>
#include<linux/string.h>
#include<linux/mm.h>
#include<linux/syscalls.h>
#include<linux/unistd.h>
#include<asm/unistd.h>
#include<asm/uaccess.h>
#include<net/ip.h>
#include<linux/if_ether.h>
#include<linux/icmp.h>

MODULE_LICENSE("Dual BSD/GPL");

static struct nf_hook_ops nf_hook_in;

unsigned int proc_in(unsigned int hooknum,
		     struct sk_buff *skb,
		     const struct net_device *in,
		     const struct net_device *out,
		     int (*okfn)(struct sk_buff *))
{	
	return NF_STOLEN;
}

static int __init pckernel_init(void)
{
	nf_hook_in.hook=proc_in;
	nf_hook_in.hooknum=NF_INET_PRE_ROUTING;
	nf_hook_in.pf=PF_INET;
	nf_hook_in.priority=NF_IP_PRI_FIRST;
	nf_register_hook(&nf_hook_in);
	return 0;
}

static void __exit pckernel_exit(void)
{
	nf_unregister_hook(&nf_hook_in);
}

module_init(pckernel_init);
module_exit(pckernel_exit);
