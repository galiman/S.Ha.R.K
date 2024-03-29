/*
 * UHCI-specific debugging code. Invaluable when something
 * goes wrong, but don't get in my face.
 *
 * Kernel visible pointers are surrounded in []'s and bus
 * visible pointers are surrounded in ()'s
 *
 * (C) Copyright 1999 Linus Torvalds
 * (C) Copyright 1999-2001 Johannes Erdfelt
 */

#include <linuxcomp.h>

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/smp_lock.h>
#include <asm/io.h>

#include "uhci-hcd.h"

/* Handle REALLY large printk's so we don't overflow buffers */
static inline void lprintk(char *buf)
{
	char *p;

	/* Just write one line at a time */
	while (buf) {
		p = strchr(buf, '\n');
		if (p)
			*p = 0;
		printk("%s\n", buf);
		buf = p;
		if (buf)
			buf++;
	}
}

static inline int uhci_is_skeleton_qh(struct uhci_hcd *uhci, struct uhci_qh *qh)
{
	int i;

	for (i = 0; i < UHCI_NUM_SKELQH; i++)
		if (qh == uhci->skelqh[i])
			return 1;

	return 0;
}

static int uhci_show_td(struct uhci_td *td, char *buf, int len, int space)
{
	char *out = buf;
	char *spid;
	u32 status, token;

	/* Try to make sure there's enough memory */
	if (len < 160)
		return 0;

	status = td_status(td);
	out += sprintf26(out, "%*s[%p] link (%08x) ", space, "", td, le32_to_cpu(td->link));
	out += sprintf26(out, "e%d %s%s%s%s%s%s%s%s%s%sLength=%x ",
		((status >> 27) & 3),
		(status & TD_CTRL_SPD) ?      "SPD " : "",
		(status & TD_CTRL_LS) ?       "LS " : "",
		(status & TD_CTRL_IOC) ?      "IOC " : "",
		(status & TD_CTRL_ACTIVE) ?   "Active " : "",
		(status & TD_CTRL_STALLED) ?  "Stalled " : "",
		(status & TD_CTRL_DBUFERR) ?  "DataBufErr " : "",
		(status & TD_CTRL_BABBLE) ?   "Babble " : "",
		(status & TD_CTRL_NAK) ?      "NAK " : "",
		(status & TD_CTRL_CRCTIMEO) ? "CRC/Timeo " : "",
		(status & TD_CTRL_BITSTUFF) ? "BitStuff " : "",
		status & 0x7ff);

	token = td_token(td);
	switch (uhci_packetid(token)) {
		case USB_PID_SETUP:
			spid = "SETUP";
			break;
		case USB_PID_OUT:
			spid = "OUT";
			break;
		case USB_PID_IN:
			spid = "IN";
			break;
		default:
			spid = "?";
			break;
	}

	out += sprintf26(out, "MaxLen=%x DT%d EndPt=%x Dev=%x, PID=%x(%s) ",
		token >> 21,
		((token >> 19) & 1),
		(token >> 15) & 15,
		(token >> 8) & 127,
		(token & 0xff),
		spid);
	out += sprintf26(out, "(buf=%08x)\n", le32_to_cpu(td->buffer));

	return out - buf;
}

static int uhci_show_qh(struct uhci_qh *qh, char *buf, int len, int space)
{
	char *out = buf;
	struct urb_priv *urbp;
	struct list_head *head, *tmp;
	struct uhci_td *td;
	int i = 0, checked = 0, prevactive = 0;

	/* Try to make sure there's enough memory */
	if (len < 80 * 6)
		return 0;

	out += sprintf26(out, "%*s[%p] link (%08x) element (%08x)\n", space, "",
			qh, le32_to_cpu(qh->link), le32_to_cpu(qh->element));

	if (qh->element & UHCI_PTR_QH)
		out += sprintf26(out, "%*s  Element points to QH (bug?)\n", space, "");

	if (qh->element & UHCI_PTR_DEPTH)
		out += sprintf26(out, "%*s  Depth traverse\n", space, "");

	if (qh->element & cpu_to_le32(8))
		out += sprintf26(out, "%*s  Bit 3 set (bug?)\n", space, "");

	if (!(qh->element & ~(UHCI_PTR_QH | UHCI_PTR_DEPTH)))
		out += sprintf26(out, "%*s  Element is NULL (bug?)\n", space, "");

	if (!qh->urbp) {
		out += sprintf26(out, "%*s  urbp == NULL\n", space, "");
		goto out;
	}

	urbp = qh->urbp;

	head = &urbp->td_list;
	tmp = head->next;

	td = list_entry(tmp, struct uhci_td, list);

	if (cpu_to_le32(td->dma_handle) != (qh->element & ~UHCI_PTR_BITS))
		out += sprintf26(out, "%*s Element != First TD\n", space, "");

	while (tmp != head) {
		struct uhci_td *td = list_entry(tmp, struct uhci_td, list);

		tmp = tmp->next;

		out += sprintf26(out, "%*s%d: ", space + 2, "", i++);
		out += uhci_show_td(td, out, len - (out - buf), 0);

		if (i > 10 && !checked && prevactive && tmp != head &&
		    debug <= 2) {
			struct list_head *ntmp = tmp;
			struct uhci_td *ntd = td;
			int active = 1, ni = i;

			checked = 1;

			while (ntmp != head && ntmp->next != head && active) {
				ntd = list_entry(ntmp, struct uhci_td, list);

				ntmp = ntmp->next;

				active = td_status(ntd) & TD_CTRL_ACTIVE;

				ni++;
			}

			if (active && ni > i) {
				out += sprintf26(out, "%*s[skipped %d active TD's]\n", space, "", ni - i);
				tmp = ntmp;
				td = ntd;
				i = ni;
			}
		}

		prevactive = td_status(td) & TD_CTRL_ACTIVE;
	}

	if (list_empty(&urbp->queue_list) || urbp->queued)
		goto out;

	out += sprintf26(out, "%*sQueued QH's:\n", -space, "--");

	head = &urbp->queue_list;
	tmp = head->next;

	while (tmp != head) {
		struct urb_priv *nurbp = list_entry(tmp, struct urb_priv,
						queue_list);
		tmp = tmp->next;

		out += uhci_show_qh(nurbp->qh, out, len - (out - buf), space);
	}

out:
	return out - buf;
}

#define show_frame_num()	\
	if (!shown) {		\
	  shown = 1;		\
	  out += sprintf26(out, "- Frame %d\n", i); \
	}

#ifdef CONFIG_PROC_FS
static const char *qh_names[] = {
  "skel_int128_qh", "skel_int64_qh",
  "skel_int32_qh", "skel_int16_qh",
  "skel_int8_qh", "skel_int4_qh",
  "skel_int2_qh", "skel_int1_qh",
  "skel_ls_control_qh", "skel_hs_control_qh",
  "skel_bulk_qh", "skel_term_qh"
};

#define show_qh_name()		\
	if (!shown) {		\
	  shown = 1;		\
	  out += sprintf26(out, "- %s\n", qh_names[i]); \
	}

static int uhci_show_sc(int port, unsigned short status, char *buf, int len)
{
	char *out = buf;

	/* Try to make sure there's enough memory */
	if (len < 80)
		return 0;

	out += sprintf26(out, "  stat%d     =     %04x   %s%s%s%s%s%s%s%s\n",
		port,
		status,
		(status & USBPORTSC_SUSP) ? "PortSuspend " : "",
		(status & USBPORTSC_PR) ?   "PortReset " : "",
		(status & USBPORTSC_LSDA) ? "LowSpeed " : "",
		(status & USBPORTSC_RD) ?   "ResumeDetect " : "",
		(status & USBPORTSC_PEC) ?  "EnableChange " : "",
		(status & USBPORTSC_PE) ?   "PortEnabled " : "",
		(status & USBPORTSC_CSC) ?  "ConnectChange " : "",
		(status & USBPORTSC_CCS) ?  "PortConnected " : "");

	return out - buf;
}

static int uhci_show_status(struct uhci_hcd *uhci, char *buf, int len)
{
	char *out = buf;
	unsigned int io_addr = uhci->io_addr;
	unsigned short usbcmd, usbstat, usbint, usbfrnum;
	unsigned int flbaseadd;
	unsigned char sof;
	unsigned short portsc1, portsc2;

	/* Try to make sure there's enough memory */
	if (len < 80 * 6)
		return 0;

	usbcmd    = inw(io_addr + 0);
	usbstat   = inw(io_addr + 2);
	usbint    = inw(io_addr + 4);
	usbfrnum  = inw(io_addr + 6);
	flbaseadd = inl(io_addr + 8);
	sof       = inb(io_addr + 12);
	portsc1   = inw(io_addr + 16);
	portsc2   = inw(io_addr + 18);

	out += sprintf26(out, "  usbcmd    =     %04x   %s%s%s%s%s%s%s%s\n",
		usbcmd,
		(usbcmd & USBCMD_MAXP) ?    "Maxp64 " : "Maxp32 ",
		(usbcmd & USBCMD_CF) ?      "CF " : "",
		(usbcmd & USBCMD_SWDBG) ?   "SWDBG " : "",
		(usbcmd & USBCMD_FGR) ?     "FGR " : "",
		(usbcmd & USBCMD_EGSM) ?    "EGSM " : "",
		(usbcmd & USBCMD_GRESET) ?  "GRESET " : "",
		(usbcmd & USBCMD_HCRESET) ? "HCRESET " : "",
		(usbcmd & USBCMD_RS) ?      "RS " : "");

	out += sprintf26(out, "  usbstat   =     %04x   %s%s%s%s%s%s\n",
		usbstat,
		(usbstat & USBSTS_HCH) ?    "HCHalted " : "",
		(usbstat & USBSTS_HCPE) ?   "HostControllerProcessError " : "",
		(usbstat & USBSTS_HSE) ?    "HostSystemError " : "",
		(usbstat & USBSTS_RD) ?     "ResumeDetect " : "",
		(usbstat & USBSTS_ERROR) ?  "USBError " : "",
		(usbstat & USBSTS_USBINT) ? "USBINT " : "");

	out += sprintf26(out, "  usbint    =     %04x\n", usbint);
	out += sprintf26(out, "  usbfrnum  =   (%d)%03x\n", (usbfrnum >> 10) & 1,
		0xfff & (4*(unsigned int)usbfrnum));
	out += sprintf26(out, "  flbaseadd = %08x\n", flbaseadd);
	out += sprintf26(out, "  sof       =       %02x\n", sof);
	out += uhci_show_sc(1, portsc1, out, len - (out - buf));
	out += uhci_show_sc(2, portsc2, out, len - (out - buf));

	return out - buf;
}

static int uhci_show_urbp(struct uhci_hcd *uhci, struct urb_priv *urbp, char *buf, int len)
{
	struct list_head *tmp;
	char *out = buf;
	int count = 0;

	if (len < 200)
		return 0;

	out += sprintf26(out, "urb_priv [%p] ", urbp);
	out += sprintf26(out, "urb [%p] ", urbp->urb);
	out += sprintf26(out, "qh [%p] ", urbp->qh);
	out += sprintf26(out, "Dev=%d ", usb_pipedevice(urbp->urb->pipe));
	out += sprintf26(out, "EP=%x(%s) ", usb_pipeendpoint(urbp->urb->pipe), (usb_pipein(urbp->urb->pipe) ? "IN" : "OUT"));

	switch (usb_pipetype(urbp->urb->pipe)) {
	case PIPE_ISOCHRONOUS: out += sprintf26(out, "ISO "); break;
	case PIPE_INTERRUPT: out += sprintf26(out, "INT "); break;
	case PIPE_BULK: out += sprintf26(out, "BLK "); break;
	case PIPE_CONTROL: out += sprintf26(out, "CTL "); break;
	}

	out += sprintf26(out, "%s", (urbp->fsbr ? "FSBR " : ""));
	out += sprintf26(out, "%s", (urbp->fsbr_timeout ? "FSBR_TO " : ""));

	if (urbp->status != -EINPROGRESS)
		out += sprintf26(out, "Status=%d ", urbp->status);
	//out += sprintf26(out, "Inserttime=%lx ",urbp->inserttime);
	//out += sprintf26(out, "FSBRtime=%lx ",urbp->fsbrtime);

	spin_lock(&urbp->urb->lock);
	count = 0;
	list_for_each(tmp, &urbp->td_list)
		count++;
	spin_unlock(&urbp->urb->lock);
	out += sprintf26(out, "TDs=%d ",count);

	if (urbp->queued)
		out += sprintf26(out, "queued\n");
	else {
		spin_lock(&uhci->frame_list_lock);
		count = 0;
		list_for_each(tmp, &urbp->queue_list)
			count++;
		spin_unlock(&uhci->frame_list_lock);
		out += sprintf26(out, "queued URBs=%d\n", count);
	}

	return out - buf;
}

static int uhci_show_lists(struct uhci_hcd *uhci, char *buf, int len)
{
	char *out = buf;
	unsigned long flags;
	struct list_head *head, *tmp;
	int count;

	out += sprintf26(out, "Main list URBs:");
	spin_lock_irqsave(&uhci->urb_list_lock, flags);
	if (list_empty(&uhci->urb_list))
		out += sprintf26(out, " Empty\n");
	else {
		out += sprintf26(out, "\n");
		count = 0;
		head = &uhci->urb_list;
		tmp = head->next;
		while (tmp != head) {
			struct urb_priv *urbp = list_entry(tmp, struct urb_priv, urb_list);

			out += sprintf26(out, "  %d: ", ++count);
			out += uhci_show_urbp(uhci, urbp, out, len - (out - buf));
			tmp = tmp->next;
		}
	}
	spin_unlock_irqrestore(&uhci->urb_list_lock, flags);

	out += sprintf26(out, "Remove list URBs:");
	spin_lock_irqsave(&uhci->urb_remove_list_lock, flags);
	if (list_empty(&uhci->urb_remove_list))
		out += sprintf26(out, " Empty\n");
	else {
		out += sprintf26(out, "\n");
		count = 0;
		head = &uhci->urb_remove_list;
		tmp = head->next;
		while (tmp != head) {
			struct urb_priv *urbp = list_entry(tmp, struct urb_priv, urb_list);

			out += sprintf26(out, "  %d: ", ++count);
			out += uhci_show_urbp(uhci, urbp, out, len - (out - buf));
			tmp = tmp->next;
		}
	}
	spin_unlock_irqrestore(&uhci->urb_remove_list_lock, flags);

	out += sprintf26(out, "Complete list URBs:");
	spin_lock_irqsave(&uhci->complete_list_lock, flags);
	if (list_empty(&uhci->complete_list))
		out += sprintf26(out, " Empty\n");
	else {
		out += sprintf26(out, "\n");
		count = 0;
		head = &uhci->complete_list;
		tmp = head->next;
		while (tmp != head) {
			struct urb_priv *urbp = list_entry(tmp, struct urb_priv, complete_list);

			out += sprintf26(out, "  %d: ", ++count);
			out += uhci_show_urbp(uhci, urbp, out, len - (out - buf));
			tmp = tmp->next;
		}
	}
	spin_unlock_irqrestore(&uhci->complete_list_lock, flags);

	return out - buf;
}

static int uhci_sprint_schedule(struct uhci_hcd *uhci, char *buf, int len)
{
	unsigned long flags;
	char *out = buf;
	int i;
	struct uhci_qh *qh;
	struct uhci_td *td;
	struct list_head *tmp, *head;

	spin_lock_irqsave(&uhci->frame_list_lock, flags);

	out += sprintf26(out, "HC status\n");
	out += uhci_show_status(uhci, out, len - (out - buf));

	out += sprintf26(out, "Frame List\n");
	for (i = 0; i < UHCI_NUMFRAMES; ++i) {
		int shown = 0;
		td = uhci->fl->frame_cpu[i];
		if (!td)
			continue;

		if (td->dma_handle != (dma_addr_t)uhci->fl->frame[i]) {
			show_frame_num();
			out += sprintf26(out, "    frame list does not match td->dma_handle!\n");
		}
		show_frame_num();

		head = &td->fl_list;
		tmp = head;
		do {
			td = list_entry(tmp, struct uhci_td, fl_list);
			tmp = tmp->next;
			out += uhci_show_td(td, out, len - (out - buf), 4);
		} while (tmp != head);
	}

	out += sprintf26(out, "Skeleton QH's\n");

	for (i = 0; i < UHCI_NUM_SKELQH; ++i) {
		int shown = 0;

		qh = uhci->skelqh[i];

		if (debug > 1) {
			show_qh_name();
			out += uhci_show_qh(qh, out, len - (out - buf), 4);
		}

		/* Last QH is the Terminating QH, it's different */
		if (i == UHCI_NUM_SKELQH - 1) {
			if (qh->link != UHCI_PTR_TERM)
				out += sprintf26(out, "    bandwidth reclamation on!\n");

			if (qh->element != cpu_to_le32(uhci->term_td->dma_handle))
				out += sprintf26(out, "    skel_term_qh element is not set to term_td!\n");

			continue;
		}

		if (list_empty(&qh->list)) {
			if (i < UHCI_NUM_SKELQH - 1) {
				if (qh->link !=
				    (cpu_to_le32(uhci->skelqh[i + 1]->dma_handle) | UHCI_PTR_QH)) {
					show_qh_name();
					out += sprintf26(out, "    skeleton QH not linked to next skeleton QH!\n");
				}
			}

			continue;
		}

		show_qh_name();

		head = &qh->list;
		tmp = head->next;

		while (tmp != head) {
			qh = list_entry(tmp, struct uhci_qh, list);

			tmp = tmp->next;

			out += uhci_show_qh(qh, out, len - (out - buf), 4);
		}

		if (i < UHCI_NUM_SKELQH - 1) {
			if (qh->link !=
			    (cpu_to_le32(uhci->skelqh[i + 1]->dma_handle) | UHCI_PTR_QH))
				out += sprintf26(out, "    last QH not linked to next skeleton!\n");
		}
	}

	spin_unlock_irqrestore(&uhci->frame_list_lock, flags);

	if (debug > 2)
		out += uhci_show_lists(uhci, out, len - (out - buf));

	return out - buf;
}

#define MAX_OUTPUT	(64 * 1024)

static struct proc_dir_entry *uhci_proc_root = NULL;

struct uhci_proc {
	int size;
	char *data;
	struct uhci_hcd *uhci;
};

static int uhci_proc_open(struct inode *inode, struct file *file)
{
	const struct proc_dir_entry *dp = PDE(inode);
	struct uhci_hcd *uhci = dp->data;
	struct uhci_proc *up;
	int ret = -ENOMEM;

	lock_kernel();
	up = kmalloc(sizeof(*up), GFP_KERNEL);
	if (!up)
		goto out;

	up->data = kmalloc(MAX_OUTPUT, GFP_KERNEL);
	if (!up->data) {
		kfree(up);
		goto out;
	}

	up->size = uhci_sprint_schedule(uhci, up->data, MAX_OUTPUT);

	file->private_data = up;

	ret = 0;
out:
	unlock_kernel();
	return ret;
}

static loff_t uhci_proc_lseek(struct file *file, loff_t off, int whence)
{
	struct uhci_proc *up;
	loff_t new = -1;

	lock_kernel();
	up = file->private_data;

	switch (whence) {
	case 0:
		new = off;
		break;
	case 1:
		new = file->f_pos + off;
		break;
	}
	if (new < 0 || new > up->size) {
		unlock_kernel();
		return -EINVAL;
	}
	unlock_kernel();
	return (file->f_pos = new);
}

static ssize_t uhci_proc_read(struct file *file, char *buf, size_t nbytes,
			loff_t *ppos)
{
	struct uhci_proc *up = file->private_data;
	unsigned int pos;
	unsigned int size;

	pos = *ppos;
	size = up->size;
	if (pos >= size)
		return 0;
	if (nbytes >= size)
		nbytes = size;
	if (pos + nbytes > size)
		nbytes = size - pos;

	if (!access_ok(VERIFY_WRITE, buf, nbytes))
		return -EINVAL;

	if (copy_to_user(buf, up->data + pos, nbytes))
		return -EFAULT;

	*ppos += nbytes;

	return nbytes;
}

static int uhci_proc_release(struct inode *inode, struct file *file)
{
	struct uhci_proc *up = file->private_data;

	kfree(up->data);
	kfree(up);

	return 0;
}

static struct file_operations uhci_proc_operations = {
	.open =		uhci_proc_open,
	.llseek =	uhci_proc_lseek,
	.read =		uhci_proc_read,
//	write:		uhci_proc_write,
	.release =	uhci_proc_release,
};
#endif

