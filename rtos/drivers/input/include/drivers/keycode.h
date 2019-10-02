/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

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

#ifndef __KEYCODE_H__
#define __KEYCODE_H__

#define NUM_OF_KEY		0x90	/* Till the end */
#define TABLE_KEY_SIZE		0x3A	/* Till KEY_SPC */

#define KEY_ESC		0x01
#define KEY_1		0x02
#define KEY_2		0x03
#define KEY_3		0x04
#define KEY_4		0x05
#define KEY_5		0x06
#define KEY_6		0x07
#define KEY_7		0x08
#define KEY_8		0x09
#define KEY_9		0x0A
#define KEY_0		0x0B
#define KEY_SUB		0x0C
#define KEY_PLUS	0x0D
#define KEY_BKS		0x0E
#define KEY_TAB		0x0F
#define KEY_Q		0x10
#define KEY_W		0x11
#define KEY_E		0x12
#define KEY_R		0x13
#define KEY_T		0x14
#define KEY_Y		0x15
#define KEY_U		0x16
#define KEY_I		0x17
#define KEY_O		0x18
#define KEY_P		0x19
#define KEY_BRL		0x1A
#define KEY_BRR		0x1B
#define KEY_ENT		0x1C
#define KEY_CTRLL	0x1D
#define KEY_A		0x1E
#define KEY_S		0x1F
#define KEY_D		0x20
#define KEY_F		0x21
#define KEY_G		0x22
#define KEY_H		0x23
#define KEY_J		0x24
#define KEY_K		0x25
#define KEY_L		0x26
#define KEY_COL		0x27
#define KEY_API		0x28
#define KEY_TIL		0x29
#define KEY_SHL		0x2A
#define KEY_BSL		0x2B
#define KEY_Z		0x2C
#define KEY_X		0x2D
#define KEY_C		0x2E
#define KEY_V		0x2F
#define KEY_B		0x30
#define KEY_N		0x31
#define KEY_M		0x32
#define KEY_LT		0x33
#define KEY_GT		0x34
#define KEY_SLH		0x35
#define KEY_SHR		0x36
#define PAD_AST		0x37
#define KEY_ALTL	0x38
#define KEY_SPC		0x39
#define KEY_CPSLOCK	0x3A
#define KEY_F1		0x3B
#define KEY_F2		0x3C
#define KEY_F3		0x3D
#define KEY_F4		0x3E
#define KEY_F5		0x3F
#define KEY_F6		0x40
#define KEY_F7		0x41
#define KEY_F8		0x42
#define KEY_F9		0x43
#define KEY_F10		0x44
#define PAD_NUMLOCK	0x45
#define EXT_SCRLOCK	0x46
#define PAD_HOME	0x47
#define PAD_UP		0x48
#define PAD_PGUP	0x49
#define PAD_SUB		0x4A
#define PAD_LEFT	0x4B
#define PAD_5		0x4C
#define PAD_RIGHT	0x4D
#define PAD_PLUS	0x4E
#define PAD_END		0x4F
#define PAD_DOWN	0x50
#define PAD_PGDW	0x51
#define PAD_INS		0x52
#define PAD_DEL		0x53

#define KEY_MACRO	0x56	/* ?!? */
#define KEY_F11		0x57
#define KEY_F12		0x58

#define PAD_ENT		0x60
#define KEY_CTRLR	0x61
#define PAD_SLH		0x62
#define EXT_PTRSC	0x63
#define KEY_ALTR	0x64

#define EXT_HOME	0x66
#define EXT_UP		0x67
#define EXT_PGUP	0x68
#define EXT_LEFT	0x69
#define EXT_RIGHT	0x6A
#define EXT_END		0x6B
#define EXT_DOWN	0x6C
#define EXT_PGDW	0x6D
#define EXT_INS		0x6E
#define EXT_DEL		0x6F

#define PWR_POWER	0x74

#define EXT_PAUSE	0x77

#define WIN_L		0x7D
#define WIN_R		0x7E
#define WIN_APPS	0x7F

#define PWR_SLEEP	0x8E
#define PWR_WAKE	0x8F

#endif /* __KEYCODE_H__ */
