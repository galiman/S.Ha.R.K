
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linuxcomp.h>

#include <linux/config.h>
#include <linux/types.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/signal.h>

#include "media/pwc-ioctl.h"
#include <linux/videodev.h>
#include <drivers/shark_pwc26.h>

/*
* shark lowlevel wrapper
*/

struct PWC_data {
	struct file* file;
	struct pwc_imagesize RealSize;
	struct pwc_coord m_ViewSize, m_RealSize;
	struct pwc_coord m_Offset;
	struct video_capability Capability;
	struct pwc_probe Probe;
	struct video_picture Picture;
	struct video_window Window;
	struct pwc_video_command VideoCmd;
	void *m_pPWCXBuffer, *m_pInputBuffer, *m_pImageBuffer;
	int m_Frames, len, m_UseRaw, m_ImageSize, m_ImageLen;
	int m_Type, m_Bandlength, m_Bayer;
	int m_InputLen;
};


extern int usb_pwc_init();
extern void* malloc(size_t size);

extern int pwc_video_do_ioctl(struct inode *inode, struct file *file, unsigned int cmd, void *arg);
extern int pwc_video_open(struct inode *inode, struct file *file);
extern int pwc_video_close(struct inode *inode, struct file *file);
extern ssize_t pwc_video_read(struct file *file, char *buf, size_t count, loff_t *ppos);
extern int pwc_video_ioctl(struct inode *inode, struct file *file, unsigned int ioctlnr, unsigned long arg);

void shark_PWC_init()
{
	usb_pwc_init();
}

int shark_PWC_open(struct PWC26_DEVICE *pwc26, int width, int height, int fps, int quality, int webcamnr)
{
	int err;
	struct PWC_data *pwc_data;
	struct file *file;
	struct inode *inode;

	pwc_data = malloc(sizeof(struct PWC_data));
	pwc26->private_data = (void*)pwc_data;

	if (!pwc_data)
		return -ENOMEM;

	memset(pwc_data, 0, sizeof(struct PWC_data));
	file =  malloc(sizeof(struct file));
	if (!file)
		return -ENOMEM;

	file->f_dentry= malloc(sizeof(struct dentry));
	if (!file->f_dentry)
		return -ENOMEM;

	file->f_dentry->d_inode = malloc(sizeof(struct inode));
	if (!file->f_dentry->d_inode)
		return -ENOMEM;

	file->f_dentry->d_inode->i_rdev = webcamnr;
	inode = file->f_dentry->d_inode;

	pwc_data->file = file;
	pwc_data->m_ViewSize.x = width;
	pwc_data->m_ViewSize.y = height;
	pwc_data->m_Frames = fps;

	pwc_data->m_RealSize = pwc_data->m_ViewSize;

	pwc_data->m_Offset.x = 0;
	pwc_data->m_Offset.y = 0;
	pwc_data->m_UseRaw = TRUE;
	pwc_data->m_Bayer = FALSE;
	pwc_data->m_pInputBuffer = NULL;
	pwc_data->m_InputLen = 0;
	pwc_data->m_pImageBuffer = NULL;
	pwc_data->m_ImageLen = NULL;
	pwc_data->m_Type = NULL;

	pwc_data->m_pPWCXBuffer = malloc(60000); // large enoug for all cams
	if (!pwc_data->m_pPWCXBuffer)
		return -ENOMEM;

	err=pwc_video_open(inode, file);
	if (err <0)
		return err;

	pwc_video_do_ioctl(inode, file, VIDIOCGCAP, &pwc_data->Capability);

	printk(KERN_INFO "Capability->type= %d\n", pwc_data->Capability.type);
	printk(KERN_INFO "Capability->channels =%d\n",pwc_data->Capability.channels);
	printk(KERN_INFO "Capability->audios=%d\n",pwc_data->Capability.audios);
	printk(KERN_INFO "Capability->minwidth=%d\n",pwc_data->Capability.minwidth);
	printk(KERN_INFO "Capability->minheight=%d\n",pwc_data->Capability.minheight);
	printk(KERN_INFO "Capability->maxwidth=%d\n",pwc_data->Capability.maxwidth);
	printk(KERN_INFO "Capability->maxheight=%d\n",pwc_data->Capability.maxheight);

	memset(&pwc_data->Probe, 0, sizeof(pwc_data->Probe));
	pwc_video_do_ioctl(inode, file, VIDIOCPWCPROBE, &pwc_data->Probe);
	pwc_video_do_ioctl(inode, file, VIDIOCGPICT, &pwc_data->Picture);

	if (pwc_data->m_UseRaw)
		pwc_data->Picture.palette = VIDEO_PALETTE_RAW;
	else
		pwc_data->Picture.palette = VIDEO_PALETTE_YUV420P;

	pwc_video_do_ioctl(inode, file, VIDIOCSPICT, &pwc_data->Picture);
	pwc_video_do_ioctl(inode, file, VIDIOCGWIN, &pwc_data->Window);

	/* unavailable in pwc 10.x */
	if (pwc_data->m_Bayer)
	{
		// special raw Bayer mode
		pwc_data->m_ViewSize.x = 640;
		pwc_data->m_ViewSize.y = 480;
		pwc_data->m_Frames = 5;
		pwc_data->m_RealSize = pwc_data->m_ViewSize;
	}

	pwc_data->Window.width = pwc_data->m_ViewSize.x;
	pwc_data->Window.height = pwc_data->m_ViewSize.y;
	pwc_data->Window.flags = (pwc_data->m_Frames << PWC_FPS_SHIFT);

	if (pwc_data->m_Bayer)
		pwc_data->Window.flags |= PWC_FPS_SNAPSHOT;

	pwc_video_do_ioctl(inode, file, VIDIOCSWIN, &pwc_data->Window);

	if (pwc_data->m_UseRaw)
	{
		pwc_video_do_ioctl(inode, file, VIDIOCPWCGVIDCMD, &pwc_data->VideoCmd);

		pwc_data->m_Type = pwc_data->VideoCmd.type;
		pwc_data->m_InputLen = pwc_data->VideoCmd.frame_size;
		pwc_data->m_Bandlength = pwc_data->VideoCmd.bandlength;

		if (pwc_data->m_Bandlength > 0)
		{
			switch(pwc_data->m_Type)
			{
				case 645:
				case 646:
					pwcx_init_decompress_Nala(pwc_data->m_Type, pwc_data->VideoCmd.release, &pwc_data->VideoCmd.command_buf, pwc_data->m_pPWCXBuffer);
				break;

				case 675:
				case 680:
				case 690:
					pwcx_init_decompress_Timon(pwc_data->m_Type, pwc_data->VideoCmd.release, &pwc_data->VideoCmd.command_buf, pwc_data->m_pPWCXBuffer);
					break;

				case 720:
				case 730:
				case 740:
				case 750:
					pwcx_init_decompress_Kiara(pwc_data->m_Type, pwc_data->VideoCmd.release, &pwc_data->VideoCmd.command_buf, pwc_data->m_pPWCXBuffer);
					break;

				default:
					//           			qDebug("Unknown type of camera (%d)!", VideoCmd.type);
					break;
			} // ..switch
		} // ..m_Bandlength > 0

		if (pwc_video_do_ioctl(inode, file, VIDIOCPWCGREALSIZE, &pwc_data->RealSize) == 0)
		{
			pwc_data->m_Offset.x = (pwc_data->m_ViewSize.x - pwc_data->RealSize.width) / 2;
			pwc_data->m_Offset.y = (pwc_data->m_ViewSize.y - pwc_data->RealSize.height) / 2;
		}
	}

	pwc_data->m_ImageLen = pwc_data->m_ViewSize.x * pwc_data->m_ViewSize.y * 3;
	pwc_data->m_pImageBuffer = malloc (pwc_data->m_ImageLen);

	if (pwc_data->m_UseRaw)
	{
		pwc_data->m_pInputBuffer = malloc(pwc_data->m_InputLen);
	}
	else
	{
		if (pwc_data->m_pImageBuffer != 0)
		{
			pwc_data->m_pInputBuffer = pwc_data->m_pImageBuffer;
			pwc_data->m_InputLen = pwc_data->m_ImageLen;
		}
	}

	pwc26->width=pwc_data->m_ViewSize.x ;
	pwc26->height=pwc_data->m_ViewSize.y ;
	pwc26->imgptr=(BYTE*)pwc_data->m_pImageBuffer;

	return 0;
}

int shark_PWC_read(struct PWC26_DEVICE *pwc26)
{
	int len;
	struct PWC_data *pwc_data = (struct PWC_data*)pwc26->private_data;

	if (pwc_data->m_pInputBuffer == 0 || pwc_data->m_pImageBuffer == 0)
		return -666;

	len = pwc_video_read(pwc_data->file, pwc_data->m_pInputBuffer, pwc_data->m_InputLen, 0);

	if (len > 0 && pwc_data->m_Bandlength > 0)
	{
		switch(pwc_data->m_Type)
		{
			case 645:
			case 646:
				pwcx_decompress_Nala(&pwc_data->m_RealSize, &pwc_data->m_ViewSize, &pwc_data->m_Offset, \
				pwc_data->m_pInputBuffer, pwc_data->m_pImageBuffer, \
				PWCX_FLAG_PLANAR, \
				pwc_data->m_pPWCXBuffer, pwc_data->m_Bandlength);
				break;

			case 675:
			case 680:
			case 690:
				pwcx_decompress_Timon(&pwc_data->m_RealSize, &pwc_data->m_ViewSize, &pwc_data->m_Offset, \
				pwc_data->m_pInputBuffer, pwc_data->m_pImageBuffer, \
				PWCX_FLAG_PLANAR, \
				pwc_data->m_pPWCXBuffer, pwc_data->m_Bandlength);
				break;

			case 720:
			case 730:
			case 740:
			case 750:
				pwcx_decompress_Kiara(&pwc_data->m_RealSize, &pwc_data->m_ViewSize, &pwc_data->m_Offset, \
				pwc_data->m_pInputBuffer, pwc_data->m_pImageBuffer, \
				PWCX_FLAG_PLANAR | (pwc_data->m_Bayer ? PWCX_FLAG_BAYER : 0), \
				pwc_data->m_pPWCXBuffer, pwc_data->m_Bandlength);
				break;
		}
	}
	return len;
}

void shark_PWC_close(struct PWC26_DEVICE *pwc26)
{
	struct file *file;
	struct inode *inode;
	struct PWC_data *pwc_data = (struct PWC_data*)pwc26->private_data;

	if (!pwc_data)
		return;

	free(pwc_data->m_pPWCXBuffer);

	file = pwc_data->file;
	inode = file->f_dentry->d_inode;
	//  pwc_video_close(inode, file);

	free(inode);
	free(file->f_dentry);
	free(file);
	free(pwc_data);
}
