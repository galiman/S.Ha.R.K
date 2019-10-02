#ifndef __SKBUFF__
#define __SKBUFF__

#include <linux/compatib.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define HAVE_ALLOC_SKB		/* For the drivers to know */
#define HAVE_ALIGNABLE_SKB	/* Ditto 8)		   */


#define FREE_READ	1
#define FREE_WRITE	0

#define CHECKSUM_NONE 0
#define CHECKSUM_HW 1
#define CHECKSUM_UNNECESSARY 2


struct sk_buff_head 
{
	struct sk_buff	* next;
	struct sk_buff	* prev;
	__u32		qlen;		/* Must be same length as a pointer
					   for using debugging */
#if CONFIG_SKB_CHECK
	int		magic_debug_cookie;
#endif
};


struct sk_buff 
{
	struct sk_buff	* next;			/* Next buffer in list 				*/
	struct sk_buff	* prev;			/* Previous buffer in list 			*/
	struct sk_buff_head * list;		/* List we are on				*/
#if CONFIG_SKB_CHECK
	int		magic_debug_cookie;
#endif
#if 0
	struct sk_buff	*link3;			/* Link for IP protocol level buffer chains 	*/
#endif

#if 0
	struct sock	*sk;			/* Socket we are owned by 			*/
#endif
	struct device	*dev;			/* Device we arrived on/are leaving by		*/
#ifdef NOTWORK
	unsigned long	when;			/* used to compute rtt's			*/
	struct timeval	stamp;			/* Time we arrived				*/
	union 
	{
		struct tcphdr	*th;
		struct ethhdr	*eth;
		struct iphdr	*iph;
		struct udphdr	*uh;
		unsigned char	*raw;
		/* for passing file handles in a unix domain socket */
		void *filp;
	} h;
  
	union 
	{	
		/* As yet incomplete physical layer views */
	  	unsigned char 	*raw;
	  	struct ethhdr	*ethernet;
	} mac;
  
	struct iphdr	*ip_hdr;		/* For IPPROTO_RAW 				*/

#endif /*NOTWORK*/

        char            cb[48];                 /* added by PJ */

	unsigned long 	len;			/* Length of actual data			*/
	unsigned long	csum;			/* Checksum 					*/
#if 0
	__u32		saddr;			/* IP source address				*/
	__u32		daddr;			/* IP target address				*/
	__u32		raddr;			/* IP next hop address				*/
	__u32		seq;			/* TCP sequence number				*/
	__u32		end_seq;		/* seq [+ fin] [+ syn] + datalen		*/
	__u32		ack_seq;		/* TCP ack sequence number			*/
	unsigned char	proto_priv[16];	        /* Protocol private data			*/
#endif
	volatile char 	acked,			/* Are we acked ?				*/
			used,			/* Are we in use ?				*/
			free,			/* How to free this buffer			*/
			arp;			/* Has IP/ARP resolution finished		*/
#if 0
	unsigned char	tries,			/* Times tried					*/
  			lock,			/* Are we locked ?				*/
  			localroute,		/* Local routing asserted for this frame	*/
  			pkt_type,		/* Packet class					*/
  			pkt_bridged,		/* Tracker for bridging 			*/
#endif
    unsigned char	ip_summed;		/* Driver fed us an IP checksum			*/
#if 0
#define PACKET_HOST		0		/* To us					*/
#define PACKET_BROADCAST	1		/* To all					*/
#define PACKET_MULTICAST	2		/* To group					*/
#define PACKET_OTHERHOST	3		/* To someone else 				*/
	unsigned short	users;			/* User count - see datagram.c,tcp.c 		*/
#endif 
	unsigned short	protocol;		/* Packet protocol from driver. 		*/
	unsigned int	truesize;		/* Buffer size 					*/
#if 0



	atomic_t	count;			/* reference count				*/


	struct sk_buff	*data_skb;		/* Link to the actual data skb			*/
#endif 
	unsigned char	*head;			/* Head of buffer 				*/
	unsigned char	*data;			/* Data head pointer				*/
	unsigned char	*tail;			/* Tail pointer					*/
#if 0
	unsigned char 	*end;			/* End pointer					*/
	void 		(*destructor)(struct sk_buff *);	/* Destruct function		*/
	__u16		redirport;		/* Redirect port				*/
#endif 
};

#define dev_kfree_skb(a)
#define alloc_skb(a,b) dev_alloc_skb(a)
/* #define skb_device_unlock(skb); */
extern struct sk_buff *		dev_alloc_skb(unsigned int size);
extern unsigned char *		skb_put(struct sk_buff *skb, int len);
extern void skb_queue_head_init(struct sk_buff_head *list);
extern struct sk_buff *		skb_clone(struct sk_buff *skb, int priority);
extern void                     skb_reserve(struct sk_buff *skb, int len);

extern __inline__ int skb_headroom(struct sk_buff *skb)
{
        return skb->data-skb->head;
}

extern struct sk_buff *         skb_realloc_headroom(struct sk_buff *skb, int newheadroom);

extern __inline__ unsigned char *skb_push(struct sk_buff *skb,
unsigned int len)
{
        skb->data-=len;
        skb->len+=len;
        if(skb->data<skb->head)
        {
	  printk(KERN_ERR "skb_push: PANIC!!!");
	  //                __label__ here;
	  //                skb_under_panic(skb, len, &&here);
	  //here:           ;
        }
        return skb->data;
}

extern __inline__ char *skb_pull(struct sk_buff *skb, unsigned int len)
{
        skb->len-=len;
        return  skb->data+=len;
}

__END_DECLS
#endif
