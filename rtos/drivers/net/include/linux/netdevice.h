#ifndef __NETDEVICE__
#define __NETDEVICE__

#include <linux/config.h>
#include <linux/compatib.h>

#include <linux/if.h>

#include <linux/skbuff.h>
#include <linux/notifier.h>
#include <time.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

// for 3c59x.c (!!!)
#define le32_to_cpu(val) (val)
#define cpu_to_le32(val) (val)
#define cpu_to_le16(val) (val)
#define test_and_set_bit(val, addr) set_bit(val, addr)

static __inline__ void mdelay(int x)
{
  struct timespec delay;
  delay.tv_sec=x/1000;
  delay.tv_nsec=(x%1000)*1000000;
  nanosleep(&delay, NULL);
}

#define kfree(x) free(x)
#define ioremap(a,b) \
        (((a)<0x100000) ? (void *)((u_long)(a)) : 0)
// was vremap(a,b)) instead of 0)) PJ

#define iounmap(v) 
//PJ was #define iounmap do { if ((u_long)(v) > 0x100000) vfree(v); } while (0)

/* for future expansion when we will have different priorities. */
#define DEV_NUMBUFFS	3
#define MAX_ADDR_LEN	7
#ifndef CONFIG_AX25
#ifndef CONFIG_TR
#ifndef CONFIG_NET_IPIP
#define MAX_HEADER	32		/* We really need about 18 worst case .. so 32 is aligned */
#else
#define MAX_HEADER	80		/* We need to allow for having tunnel headers */
#endif  /* IPIP */
#else
#define MAX_HEADER	48		/* Token Ring header needs 40 bytes ... 48 is aligned */ 
#endif /* TR */
#else
#define MAX_HEADER	96		/* AX.25 + NetROM */
#endif /* AX25 */

#define IS_MYADDR	1		/* address is (one of) our own	*/
#define IS_LOOPBACK	2		/* address is for LOOPBACK	*/
#define IS_BROADCAST	3		/* address is a valid broadcast	*/
#define IS_INVBCAST	4		/* Wrong netmask bcast not for us (unused)*/
#define IS_MULTICAST	5		/* Multicast IP address */



struct dev_mc_list
{	
	struct dev_mc_list	*next;
	__u8			dmi_addr[MAX_ADDR_LEN];
	unsigned char		dmi_addrlen;
	int			dmi_users;
	int			dmi_gusers;
};



/*
 *	Network device statistics. Akin to the 2.0 ether stats but
 *	with byte counters.
 */
 
struct net_device_stats
{
	unsigned long	rx_packets;		/* total packets received	*/
	unsigned long	tx_packets;		/* total packets transmitted	*/
	unsigned long	rx_bytes;		/* total bytes received 	*/
	unsigned long	tx_bytes;		/* total bytes transmitted	*/
	unsigned long	rx_errors;		/* bad packets received		*/
	unsigned long	tx_errors;		/* packet transmit problems	*/
	unsigned long	rx_dropped;		/* no space in linux buffers	*/
	unsigned long	tx_dropped;		/* no space available in linux	*/
	unsigned long	multicast;		/* multicast packets received	*/
	unsigned long	collisions;

	/* detailed rx_errors: */
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;		/* receiver ring buff overflow	*/
	unsigned long	rx_crc_errors;		/* recved pkt with crc error	*/
	unsigned long	rx_frame_errors;	/* recv'd frame alignment error */
	unsigned long	rx_fifo_errors;		/* recv'r fifo overrun		*/
	unsigned long	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;
	
	/* for cslip etc */
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};


struct hh_cache
{
	struct hh_cache *hh_next;
	void		*hh_arp;	/* Opaque pointer, used by
					 * any address resolution module,
					 * not only ARP.
					 */
	int		hh_refcnt;	/* number of users */
	unsigned short  hh_type;	/* protocol identifier, f.e ETH_P_IP */
	char		hh_uptodate;	/* hh_data is valid */
	char		hh_data[16];    /* cached hardware header */
};

/*
 * The DEVICE structure.
 * Actually, this whole structure is a big mistake.  It mixes I/O
 * data with strictly "high-level" data, and it has to know about
 * almost every data structure used in the INET module.  
 */
struct device 
{

  /*
   * This is the first field of the "visible" part of this structure
   * (i.e. as seen by users in the "Space.c" file).  It is the name
   * the interface.
   */
  char			  *name;

  /* I/O specific fields - FIXME: Merge these and struct ifmap into one */
  unsigned long		  rmem_end;		/* shmem "recv" end	*/
  unsigned long		  rmem_start;		/* shmem "recv" start	*/
  unsigned long		  mem_end;		/* shared mem end	*/
  unsigned long		  mem_start;		/* shared mem start	*/
  unsigned long		  base_addr;		/* device I/O address	*/
  unsigned char		  irq;			/* device IRQ number	*/

  /* Low-level status flags. */
  volatile unsigned char  start,		/* start an operation	*/
                          interrupt;		/* interrupt arrived	*/
  unsigned long		  tbusy;		/* transmitter busy must be long for bitops */

  struct device		  *next;

  /* The device initialization function. Called only once. */
  int			  (*init)(struct device *dev);

  /* Some hardware also needs these fields, but they are not part of the
     usual set specified in Space.c. */
  unsigned char		  if_port;		/* Selectable AUI, TP,..*/
  unsigned char		  dma;			/* DMA channel		*/

  struct enet_statistics* (*get_stats)(struct device *dev);

  /*
   * This marks the end of the "visible" part of the structure. All
   * fields hereafter are internal to the system, and may change at
   * will (read: may be cleaned up at will).
   */

  /* These may be needed for future network-power-down code. */
  unsigned long		  trans_start;	/* Time (in jiffies) of last Tx	*/
  unsigned long		  last_rx;	/* Time of last Rx		*/

  unsigned short	  flags;	/* interface flags (a la BSD)	*/
  unsigned short	  family;	/* address family ID (AF_INET)	*/
  unsigned short	  metric;	/* routing metric (not used)	*/
  unsigned short	  mtu;		/* interface MTU value		*/
  unsigned short	  type;		/* interface hardware type	*/
  unsigned short	  hard_header_len;	/* hardware hdr length	*/
  void			  *priv;	/* pointer to private data	*/

  /* Interface address info. */
  unsigned char		  broadcast[MAX_ADDR_LEN];	/* hw bcast add	*/
  unsigned char		  pad;				/* make dev_addr aligned to 8 bytes */
  unsigned char		  dev_addr[MAX_ADDR_LEN];	/* hw address	*/
  unsigned char		  addr_len;	/* hardware address length	*/
  unsigned long		  pa_addr;	/* protocol address		*/
  unsigned long		  pa_brdaddr;	/* protocol broadcast addr	*/
  unsigned long		  pa_dstaddr;	/* protocol P-P other side addr	*/
  unsigned long		  pa_mask;	/* protocol netmask		*/
  unsigned short	  pa_alen;	/* protocol address length	*/

  struct dev_mc_list	 *mc_list;	/* Multicast mac addresses	*/
  int			 mc_count;	/* Number of installed mcasts	*/
  
  struct ip_mc_list	 *ip_mc_list;	/* IP multicast filter chain    */
  __u32			tx_queue_len;	/* Max frames per queue allowed */
    
  /* For load balancing driver pair support */
  
  unsigned long		   pkt_queue;	/* Packets queued */
  struct device		  *slave;	/* Slave device */
  struct net_alias_info		*alias_info;	/* main dev alias info */
  struct net_alias		*my_alias;	/* alias devs */
  
  /* Pointer to the interface buffers. */
  struct sk_buff_head	  buffs[DEV_NUMBUFFS];

  /* Pointers to interface service routines. */
  int			  (*open)(struct device *dev);
  int			  (*stop)(struct device *dev);
  int			  (*hard_start_xmit) (struct sk_buff *skb,
					      struct device *dev);
  int			  (*hard_header) (struct sk_buff *skb,
					  struct device *dev,
					  unsigned short type,
					  void *daddr,
					  void *saddr,
					  unsigned len);
  int			  (*rebuild_header)(void *eth, struct device *dev,
				unsigned long raddr, struct sk_buff *skb);



#define HAVE_PRIVATE_IOCTL
  int			  (*do_ioctl)(struct device *dev, struct ifreq *ifr, int cmd);
#define HAVE_SET_CONFIG
  int			  (*set_config)(struct device *dev, struct ifmap *map);
#define HAVE_CHANGE_MTU
  int			  (*change_mtu)(struct device *dev, int new_mtu);            
#define HAVE_SET_MAC_ADDR  		 
  int			  (*set_mac_address)(struct device *dev, void *addr);

#define HAVE_HEADER_CACHE
  void			  (*header_cache_bind)(struct hh_cache **hhp, struct device *dev, unsigned short htype, __u32 daddr);
  void			  (*header_cache_update)(struct hh_cache *hh, struct device *dev, unsigned char *  haddr);

#define HAVE_MULTICAST			 
  void			  (*set_multicast_list)(struct device *dev);
};

extern struct device	loopback_dev;
extern struct device	*dev_base;
extern struct packet_type *ptype_base[16];


extern void		ether_setup(struct device *dev);
extern void		tr_setup(struct device *dev);
extern void		fddi_setup(struct device *dev);
extern int		ether_config(struct device *dev, struct ifmap *map);
/* Support for loadable net-drivers */
extern int		register_netdev(struct device *dev);
extern void		unregister_netdev(struct device *dev);
extern int 		register_netdevice_notifier(struct notifier_block *nb);
extern int		unregister_netdevice_notifier(struct notifier_block *nb);

/*
unsigned short htons(unsigned short host);
unsigned short ntohs(unsigned short net);
*/

void netif_rx(struct sk_buff *skb);

__END_DECLS
#endif


