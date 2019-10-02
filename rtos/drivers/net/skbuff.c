/* Emulates Linux sk buffers using hartik net buffers... */

#include <kernel/kern.h>
#include <semaphore.h>

#include "../net/netbuff.h"
#include "../net/eth_priv.h"

#include"linux/skbuff.h"

#define LOWLEV_TX_BUFFERS 50
#define LOWLEV_RX_BUFFERS 50
struct netbuff rxbuff;
struct netbuff lowlevel_txbuff;

void skb_init(void)
{
	netbuff_init(&lowlevel_txbuff, LOWLEV_TX_BUFFERS, ETH_MAX_LEN);
	netbuff_init(&rxbuff, LOWLEV_RX_BUFFERS, ETH_MAX_LEN);
}

struct sk_buff *dev_alloc_skb(unsigned int len)
{
	struct sk_buff *skb;

	kern_cli();
	skb = kern_alloc(sizeof(struct sk_buff));
	kern_sti();
	
	skb->data = netbuff_get(&rxbuff, NON_BLOCK);
	if (skb->data == NULL) {
			return NULL;
	}
	skb->head = skb->data;
	skb->tail = skb->data;
	skb->len = 0;
	skb->truesize = len;

	return skb;
}

void skb_reserve(struct sk_buff *skb, int len)
{
	/* changed by PJ ... before it did nothing... */
        skb->data+=len;
        skb->tail+=len;
}

unsigned char *skb_put(struct sk_buff *skb, int len)
{
	unsigned char *tmp=skb->tail;
	
	skb->tail+=len;
	skb->len+=len;
	
	return tmp;
}

void skb_queue_head_init(struct sk_buff_head *list)
{    
	list->prev = (struct sk_buff *)list;
	list->next = (struct sk_buff *)list;
	list->qlen = 0;
}

struct sk_buff *skb_realloc_headroom(struct sk_buff *skb, int newheadroom)
{
	struct sk_buff *n;
	unsigned long offset;
	int headroom = skb_headroom(skb);

	/* Allocate the copy buffer */

	n = alloc_skb(skb->truesize + newheadroom-headroom, GFP_ATOMIC);
	if (n == NULL)
		return NULL;

	skb_reserve(n, newheadroom);

	/*
	*      Shift between the two data areas in bytes
	*/

	offset = n->data - skb->data;

	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	/* Copy the bytes */
	memcpy(n->data, skb->data, skb->len);
	n->list = NULL;
	// n->sk = NULL;
	// n->priority = skb->priority;
	n->protocol = skb->protocol;
	n->dev = skb->dev;
	// n->dst = dst_clone(skb->dst);
	// n->h.raw = skb->h.raw+offset;
	// n->nh.raw = skb->nh.raw+offset;
	// n->mac.raw = skb->mac.raw+offset;
	memcpy(n->cb, skb->cb, sizeof(skb->cb));
	n->used = skb->used;
	// n->is_clone = 0;
	// atomic_set(&n->users, 1);
	// n->pkt_type = skb->pkt_type;
	// n->stamp = skb->stamp;
	// n->destructor = NULL;
	// n->security = skb->security;
#ifdef CONFIG_IP_FIREWALL
	n->fwmark = skb->fwmark;
#endif

	return n;
}
