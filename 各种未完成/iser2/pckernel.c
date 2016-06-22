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
#include<linux/fs.h>
#include<linux/string.h>
#include<linux/mm.h>
#include<linux/syscalls.h>
#include<linux/unistd.h>
#include<linux/proc_fs.h>
#include<asm/unistd.h>
#include<asm/uaccess.h>
#include<net/ip.h>
#include<linux/if_ether.h>
#include<linux/icmp.h>

MODULE_LICENSE("Dual BSD/GPL");

static struct linklist{
	unsigned int addr, port, protocol;
} *ll_head=0;

static struct blacklist{
	unsigned int addr, poss;
} *bl_head=0;

static struct nf_hook_ops nf_hook_in;
static struct nf_hook_ops nf_hook_out;
int blnum=0, lastdel=0;

static int proc_read(char *buffer, 
		     char **buffer_location, 
		     off_t offset, 
		     int buffer_lenth, 
		     int zero)
{
	int i,j,p;
	char temp[256][17];
	unsigned int m,t,ipt;
	if (offset>0) return 0;
	memset(temp,0,sizeof(temp));
	for (i=0;i<blnum;i++){
		ipt=bl_head[i].addr;
		p=0;
		for(j=0;j<4;j++){
			t=ipt&0x000000ff;
			if (t==0){
				temp[i][p]='0';
				p++;
				temp[i][p]='.';
				p++;
				ipt>>=8;
				continue;
			}
			m=100;
			while (t/m==0)
				m/=10;
			while (m>0){
				temp[i][p]=t/m+'0';
				t%=m;
				p++;
				m/=10;
			}
			temp[i][p]='.';
			p++;
			ipt>>=8;
		}
		p--;
		temp[i][p]='\n';
	}
	*buffer_location=(char *)temp;
	return sizeof(temp);
}

void addll(unsigned int ad, unsigned int po, unsigned int pr)
{
	int i;
	for (i=0;i<256;i++)
		if (ll_head[i].addr==0){
			ll_head[i].addr=ad;
			ll_head[i].port=po;
			ll_head[i].protocol=pr;
			return;
		}
}

unsigned int inll(unsigned int ad, unsigned int po, unsigned int pr)
{
	int i;
	for (i=0; i<256; i++)
		if (ll_head[i].addr)
			if ((ll_head[i].addr==ad)&&
			    (ll_head[i].port==po)&&
			    (ll_head[i].protocol==pr))
				return 1;
	return 0;
}

void delll(unsigned int ad, unsigned int po, unsigned int pr)
{
	int i;
	for(i=0; i<256; i++)
		if (ll_head[i].addr)
			if ((ll_head[i].addr==ad)&&
			    (ll_head[i].port==po)&&
			    (ll_head[i].protocol==pr)){
				ll_head[i].addr=0;
				ll_head[i].port=0;
				ll_head[i].protocol=0;
				lastdel=i;
				return;
			}
} 

void addbl(unsigned int blip,unsigned int blpos)
{
	int i;
	for (i=0; i<blnum; i++)
		if (bl_head[i].addr==blip){
			if (bl_head[i].poss<200)
				(bl_head[i].poss)+=blpos;
			return;
		}
	blnum++;
	bl_head[blnum].addr=blip;
	bl_head[blnum].poss=blpos;
}

unsigned int testbl(unsigned int blip)
{
	int i;
	if ((blip==16777343)||(blip==0)) return 0;
	for (i=0; i<blnum; i++)
		if (bl_head[i].addr==blip)
			if ((bl_head[i].poss)>=100)
				return 1;
			else
				return 0;
		else{}
	return 0;
}

unsigned int tcp_proc_in(struct sk_buff *skb)
{
	struct iphdr *iph_n=ip_hdr(skb);
	struct tcphdr *tcph_n=tcp_hdr(skb);
	if (testbl(iph_n->saddr))
		return NF_STOLEN;
	if (!inll(iph_n->saddr, tcph_n->source, IPPROTO_TCP)){
		if (tcph_n->syn)
			addbl(iph_n->saddr,30);
		if (tcph_n->ack || tcph_n->rst || tcph_n->fin)
			addbl(iph_n->saddr,10);
		return NF_STOLEN;
	}
	return NF_ACCEPT;
}

unsigned int udp_proc_in(struct sk_buff *skb)
{
	struct iphdr *iph_n=ip_hdr(skb);
	struct udphdr *udph_n=udp_hdr(skb);
	if (testbl(iph_n->saddr))
		return NF_STOLEN;
	addll(iph_n->saddr, udph_n->source, IPPROTO_UDP);
	return NF_ACCEPT;
}

unsigned int proc_in(unsigned int hooknum,
		     struct sk_buff *skb,
		     const struct net_device *in,
		     const struct net_device *out,
		     int (*okfn)(struct sk_buff *))
{	
	struct ethhdr *ethh_n=eth_hdr(skb);
	struct iphdr *iph_n=ip_hdr(skb);
	
	if (ethh_n->h_proto!=ETH_P_IP)
		return NF_ACCEPT;
	switch (iph_n->protocol){
	case (IPPROTO_TCP):
		return(tcp_proc_in(skb));
	case (IPPROTO_UDP):
		return(udp_proc_in(skb));
	case (IPPROTO_ICMP):
		return NF_STOLEN;
	case (IPPROTO_RAW):
		return NF_STOLEN;
	default:
		return NF_ACCEPT;
	}
	return NF_STOLEN;
}

unsigned int tcp_proc_out(struct sk_buff *skb)
{
	struct iphdr *iph_n=ip_hdr(skb);
	struct tcphdr *tcph_n=tcp_hdr(skb);
	if (testbl(iph_n->daddr))
		return NF_STOLEN;
	if ((tcph_n->syn) && (!(tcph_n->ack))){
		addll(iph_n->daddr, tcph_n->dest, iph_n->protocol);
		return NF_ACCEPT;
	}
	if (tcph_n->fin){
		delll(iph_n->daddr, tcph_n->dest, iph_n->protocol);
		return NF_ACCEPT;
	}
	return NF_ACCEPT;
}

unsigned int udp_proc_out(struct sk_buff *skb)
{
	struct iphdr *iph_n=ip_hdr(skb);
	if (testbl(iph_n->daddr))
		return NF_STOLEN;
	return NF_ACCEPT;
}

unsigned int icmp_proc_out(struct sk_buff *skb)
{
	struct iphdr *iph_n=ip_hdr(skb);
	struct icmphdr *icmph_n=icmp_hdr(skb);
	if (icmph_n->code==ICMP_DEST_UNREACH){
		addbl(iph_n->daddr,30);
		return NF_STOLEN;
	}
	return NF_ACCEPT;
}

unsigned int proc_out(unsigned int hooknum,
		      struct sk_buff *skb,
		      const struct net_device *in,
		      const struct net_device *out,
		      int (*okfn)(struct sk_buff *))
{
	struct iphdr *iph_n=ip_hdr(skb);

	switch (iph_n->protocol){
	case (IPPROTO_TCP):
		return(tcp_proc_out(skb));
	case (IPPROTO_UDP):
		return (udp_proc_out(skb));
	case (IPPROTO_ICMP):
		return (icmp_proc_out(skb));
	default:
		return NF_ACCEPT;
	}
	return NF_STOLEN;
}

static int __init pckernel_init(void)
{
	struct linklist ll[256];
	struct blacklist bl[256];
	static struct proc_dir_entry *proc_mtd;
	memset(ll,0,sizeof(ll));
	memset(bl,0,sizeof(bl));
	ll_head=ll;
	bl_head=bl;
	if ((proc_mtd=create_proc_entry("ss_blacklist",
				       0666,
				       (struct proc_dir_entry*)NULL)))
		proc_mtd->read_proc=proc_read;
	nf_hook_in.hook=proc_in;
	nf_hook_in.hooknum=NF_INET_PRE_ROUTING;
	nf_hook_in.pf=PF_INET;
	nf_hook_in.priority=NF_IP_PRI_FIRST;
	nf_register_hook(&nf_hook_in);
	nf_hook_out.hook=proc_out;
	nf_hook_out.hooknum=NF_INET_POST_ROUTING;
	nf_hook_out.pf=PF_INET;
	nf_hook_out.priority=NF_IP_PRI_FIRST;
	nf_register_hook(&nf_hook_out);
	return 0;
}

static void __exit pckernel_exit(void)
{
	nf_unregister_hook(&nf_hook_in);
	nf_unregister_hook(&nf_hook_out);
}

EXPORT_SYMBOL(blnum);
EXPORT_SYMBOL(bl_head);
EXPORT_SYMBOL(ll_head);
EXPORT_SYMBOL(lastdel);

module_init(pckernel_init);
module_exit(pckernel_exit);
