/*
 * Video capture interface for Linux
 *
 *		A generic video device interface for the LINUX operating system
 *		using a set of device structures/vectors for low level operations.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Author:	Alan Cox, <alan@redhat.com>
 *
 * Fixes:	20000516  Claudio Matsuoka <claudio@conectiva.com>
 *		- Added procfs support
 */

#include <linuxcomp.h>

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/smp_lock.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/devfs_fs_kernel.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/semaphore.h>

#include <linux/videodev.h>

#define VIDEO_NUM_DEVICES	256
#define VIDEO_NAME              "video4linux"

/*
 *	sysfs stuff
 */

static ssize_t show_name(struct class_device *cd, char *buf)
{
	struct video_device *vfd = container_of(cd, struct video_device, class_dev);
	return sprintf26(buf,"%.*s\n",(int)sizeof(vfd->name),vfd->name);
}

static ssize_t show_dev(struct class_device *cd, char *buf)
{
	struct video_device *vfd = container_of(cd, struct video_device, class_dev);
	dev_t dev = MKDEV(VIDEO_MAJOR, vfd->minor);
	return print_dev_t(buf,dev);
}

static CLASS_DEVICE_ATTR(name, S_IRUGO, show_name, NULL);
static CLASS_DEVICE_ATTR(dev,  S_IRUGO, show_dev, NULL);

struct video_device *video_device_alloc(void)
{
	struct video_device *vfd;

	vfd = kmalloc(sizeof(*vfd),GFP_KERNEL);
	if (NULL == vfd)
		return NULL;
	memset(vfd,0,sizeof(*vfd));
	return vfd;
}

void video_device_release(struct video_device *vfd)
{
	kfree(vfd);
}

static void video_release(struct class_device *cd)
{
	struct video_device *vfd = container_of(cd, struct video_device, class_dev);

#if 1 /* needed until all drivers are fixed */
	if (!vfd->release)
		return;
#endif
	vfd->release(vfd);
}

static struct class video_class = {
        .name    = VIDEO_NAME,
	.release = video_release,
};

/*
 *	Active devices 
 */
 
/*static*/ struct video_device *video_device[VIDEO_NUM_DEVICES];
static DECLARE_MUTEX(videodev_lock);

struct video_device* video_devdata(struct file *file)
{
	return video_device[iminor(file->f_dentry->d_inode)];
}

/*
 *	Open a video device.
 */
static int video_open(struct inode *inode, struct file *file)
{
	unsigned int minor = iminor(inode);
	int err = 0;
	struct video_device *vfl;
	struct file_operations *old_fops;
	
	if(minor>=VIDEO_NUM_DEVICES)
		return -ENODEV;
	//down(&videodev_lock);
	vfl=video_device[minor];
	if(vfl==NULL) {
		//up(&videodev_lock);
		//request_module("char-major-%d-%d", VIDEO_MAJOR, minor);
		//down(&videodev_lock);
		vfl=video_device[minor];
		if (vfl==NULL) {
			//up(&videodev_lock);
			return -ENODEV;
		}
	}
	old_fops = file->f_op;
	file->f_op = fops_get(vfl->fops);
	if(file->f_op->open)
		err = file->f_op->open(inode,file);
	if (err) {
		fops_put(file->f_op);
		file->f_op = fops_get(old_fops);
	}
	fops_put(old_fops);
	//up(&videodev_lock);
	return err;
}

/*
 * helper function -- handles userspace copying for ioctl arguments
 */
int
video_usercopy(struct inode *inode, struct file *file,
	       unsigned int cmd, unsigned long arg,
	       int (*func)(struct inode *inode, struct file *file,
			   unsigned int cmd, void *arg))
{
	char	sbuf[128];
	void    *mbuf = NULL;
	void	*parg = NULL;
	int	err  = -EINVAL;

	/*  Copy arguments into temp kernel buffer  */
	switch (_IOC_DIR(cmd)) {
	case _IOC_NONE:
		parg = (void *)arg;
		break;
	case _IOC_READ: /* some v4l ioctls are marked wrong ... */
	case _IOC_WRITE:
	case (_IOC_WRITE | _IOC_READ):
		if (_IOC_SIZE(cmd) <= sizeof(sbuf)) {
			parg = sbuf;
		} else {
			/* too big to allocate from stack */
			mbuf = kmalloc(_IOC_SIZE(cmd),GFP_KERNEL);
			if (NULL == mbuf)
				return -ENOMEM;
			parg = mbuf;
		}
		
		err = -EFAULT;
		if (copy_from_user(parg, (void *)arg, _IOC_SIZE(cmd)))
			goto out;
		break;
	}

	/* call driver */
	err = func(inode, file, cmd, parg);
	if (err == -ENOIOCTLCMD)
		err = -EINVAL;
	if (err < 0)
		goto out;

	/*  Copy results into user buffer  */
	switch (_IOC_DIR(cmd))
	{
	case _IOC_READ:
	case (_IOC_WRITE | _IOC_READ):
		if (copy_to_user((void *)arg, parg, _IOC_SIZE(cmd)))
			err = -EFAULT;
		break;
	}

out:
	if (mbuf)
		kfree(mbuf);
	return err;
}

/*
 * open/release helper functions -- handle exclusive opens
 */
extern int video_exclusive_open(struct inode *inode, struct file *file)
{
	struct  video_device *vfl = video_devdata(file);
	int retval = 0;

	//down(&vfl->lock);
	if (vfl->users) {
		retval = -EBUSY;
	} else {
		vfl->users++;
	}
	//up(&vfl->lock);
	return retval;
}

extern int video_exclusive_release(struct inode *inode, struct file *file)
{
	struct  video_device *vfl = video_devdata(file);
	
	vfl->users--;
	return 0;
}

extern struct file_operations video_fops;

/**
 *	video_register_device - register video4linux devices
 *	@vfd:  video device structure we want to register
 *	@type: type of device to register
 *	@nr:   which device number (0 == /dev/video0, 1 == /dev/video1, ...
 *             -1 == first free)
 *	
 *	The registration code assigns minor numbers based on the type
 *	requested. -ENFILE is returned in all the device slots for this
 *	category are full. If not then the minor field is set and the
 *	driver initialize function is called (if non %NULL).
 *
 *	Zero is returned on success.
 *
 *	Valid types are
 *
 *	%VFL_TYPE_GRABBER - A frame grabber
 *
 *	%VFL_TYPE_VTX - A teletext device
 *
 *	%VFL_TYPE_VBI - Vertical blank data (undecoded)
 *
 *	%VFL_TYPE_RADIO - A radio card	
 */

int video_register_device(struct video_device *vfd, int type, int nr)
{
	int i=0;
	int base;
	int end;
	char *name_base;
	
	switch(type)
	{
		case VFL_TYPE_GRABBER:
			base=0;
			end=64;
			name_base = "video";
			break;
		case VFL_TYPE_VTX:
			base=192;
			end=224;
			name_base = "vtx";
			break;
		case VFL_TYPE_VBI:
			base=224;
			end=240;
			name_base = "vbi";
			break;
		case VFL_TYPE_RADIO:
			base=64;
			end=128;
			name_base = "radio";
			break;
		default:
			return -1;
	}

	/* pick a minor number */
	//down(&videodev_lock);
	if (-1 == nr) {
		/* use first free */
		for(i=base;i<end;i++)
			if (NULL == video_device[i])
				break;
		if (i == end) {
			//up(&videodev_lock);
			return -ENFILE;
		}
	} else {
		/* use the one the driver asked for */
		i = base+nr;
		if (NULL != video_device[i]) {
			//up(&videodev_lock);
			return -ENFILE;
		}
	}
	video_device[i]=vfd;
	vfd->minor=i;
	//up(&videodev_lock);

	sprintf26(vfd->devfs_name, "v4l/%s%d", name_base, i - base);
	devfs_mk_cdev(MKDEV(VIDEO_MAJOR, vfd->minor),
			S_IFCHR | S_IRUSR | S_IWUSR, vfd->devfs_name);
	//init_MUTEX(&vfd->lock);

	/* sysfs class */
        memset(&vfd->class_dev, 0x00, sizeof(vfd->class_dev));
	if (vfd->dev)
		vfd->class_dev.dev = vfd->dev;
	vfd->class_dev.class       = &video_class;
	strncpy(vfd->class_dev.class_id, vfd->devfs_name + 4, BUS_ID_SIZE);
	class_device_register(&vfd->class_dev);
	class_device_create_file(&vfd->class_dev,
				 &class_device_attr_name);
	class_device_create_file(&vfd->class_dev,
				 &class_device_attr_dev);

#if 1 /* needed until all drivers are fixed */
	if (!vfd->release)
		printk(KERN_WARNING "videodev: \"%s\" has no release callback. "
		       "Please fix your driver for proper sysfs support, see "
		       "http://lwn.net/Articles/36850/\n", vfd->name);
#endif
	return 0;
}

/**
 *	video_unregister_device - unregister a video4linux device
 *	@vfd: the device to unregister
 *
 *	This unregisters the passed device and deassigns the minor
 *	number. Future open calls will be met with errors.
 */
 
void video_unregister_device(struct video_device *vfd)
{
	//down(&videodev_lock);
	if(video_device[vfd->minor]!=vfd)
		panic("videodev: bad unregister");

	devfs_remove(vfd->devfs_name);
	video_device[vfd->minor]=NULL;
	class_device_unregister(&vfd->class_dev);
	//up(&videodev_lock);
}

// removed "static" for gcc 4x compatibility 
struct file_operations video_fops=
{
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.open		= video_open,
};

/*
 *	Initialise video for linux
 */
 
int __init videodev_init(void)
{
	printk(KERN_INFO "Linux video capture interface: v1.00\n");
	if (register_chrdev(VIDEO_MAJOR,VIDEO_NAME, &video_fops)) {
		printk("video_dev: unable to get major %d\n", VIDEO_MAJOR);
		return -EIO;
	}
	class_register(&video_class);
	return 0;
}

void __exit videodev_exit(void)
{
	class_unregister(&video_class);
	unregister_chrdev(VIDEO_MAJOR, VIDEO_NAME);
}

extern int linuxcomp_setfd(struct inode *i, int i_rdev);
                       
void *file = NULL;

int bttv_open(struct inode *inode, struct file *file);
int bttv_release(struct inode *inode, struct file *file); 
                                                                                                   
/* Shark Inode emulation - Open */
int videodev_open_inode(int num) {
                                                                                                                             
  struct inode *i;
                                                                                                                             
  i = (struct inode *)kmalloc(sizeof(struct inode),GFP_KERNEL);
  if (file == NULL) file = (void *)kmalloc(50000,GFP_KERNEL);           
                                                                                                                  
  linuxcomp_setfd(i,num);
                                                                                                                             
  if (bttv_open(i,file)) {
    kfree(i);
    return -1;
  }
                                                                                                                             
  kfree(i);
  return 0;
                                                                                                                             
}

extern int bttv_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg);

int videodev_ioctl_inode(int num,unsigned int cmd,unsigned long arg) {

  struct inode *i;
  int res;
                                                                                                                             
  i = (struct inode *)kmalloc(sizeof(struct inode),GFP_KERNEL);
                                                                                                                             
  linuxcomp_setfd(i,num);
  
  res = bttv_ioctl(i,file,cmd,arg);
 
  kfree(i);
  return res;
                                                                                                                             
}

module_init(videodev_init)
module_exit(videodev_exit)

EXPORT_SYMBOL(video_register_device);
EXPORT_SYMBOL(video_unregister_device);
EXPORT_SYMBOL(video_devdata);
EXPORT_SYMBOL(video_usercopy);
EXPORT_SYMBOL(video_exclusive_open);
EXPORT_SYMBOL(video_exclusive_release);
EXPORT_SYMBOL(video_device_alloc);
EXPORT_SYMBOL(video_device_release);

MODULE_AUTHOR("Alan Cox");
MODULE_DESCRIPTION("Device registrar for Video4Linux drivers");
MODULE_LICENSE("GPL");


/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
