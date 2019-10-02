#ifndef __ETHERDEVICE__
#define __ETHERDEVICE__

#include <linux/compatib.h>

#include <linux/if_ether.h>
#include <linux/socket.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define eth_header panic_stub
#if 0
extern int		eth_header(struct sk_buff *skb, struct device *dev,
				   unsigned short type, void *daddr,
				   void *saddr, unsigned len);
extern int		eth_rebuild_header(void *buff, struct device *dev,
			unsigned long dst, struct sk_buff *skb);
#endif

/* extern unsigned short	eth_type_trans(struct sk_buff *skb, struct device *dev); */
#define eth_type_trans(a,b) 0

#if 0
extern void eth_header_cache_bind(struct hh_cache ** hhp, struct device *dev,
				  unsigned short htype, __u32 daddr);
extern void eth_header_cache_update(struct hh_cache *hh, struct device *dev, unsigned char * haddr);
#endif
extern void		eth_copy_and_sum(struct sk_buff *dest,
				unsigned char *src, int length, int base);
extern struct device	* init_etherdev(struct device *, int);

__END_DECLS
#endif
