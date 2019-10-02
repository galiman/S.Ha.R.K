
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

#include <ll/i386/x-dos.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/****************************************************************************/
/*  DEFINIZIONE DEI TIPI                                                    */
/****************************************************************************/

typedef struct {              /* Questa struttura ci serve per definire */
   BYTE red, green, blue;              /* la nostra palette.                     */
} TYPE_PALETTE;

typedef TYPE_PALETTE palette[256];

/*
  Tipo VbeInfoBlock: in esso sono contenute tutte le caratteristiche del
                     display grafico a nostra disposizione.
*/
typedef struct {
   unsigned char      VbeSignature[4];       /* Deve essere 'VBE2' */
   unsigned short int VbeVersion;            /* Versione del driver */
   unsigned short int OemNameOffset;         /* Nome della scheda grafica */
   unsigned short int OemNameSegment;    
   unsigned char      Capabilities[4];       /* Caratt. display grafico */
   unsigned short int SupportedModesOffset;  /* Puntatore alla lista dei modi */
   unsigned short int SupportedModesSegment; /* supportati */
   unsigned short int TotalMemory;           /* Memoria a bordo della scheda*/

   unsigned short int OemSoftwareRev;        /* Livello revisione VBE */
   unsigned short int OemVendorNameOffset;   /* Nome del produttore */
   unsigned short int OemVendorNameSegment;  
   unsigned short int OemProductNameOffset;  /* Nome del prodotto */
   unsigned short int OemProductNameSegment; 
   unsigned short int OemProductRevOffset;   /* Livello revisione display */
   unsigned short int OemProductRevSegment;  

   unsigned char      reserved[222];         /* Riservato */
   unsigned char      OemData[256];          
} VbeInfoBlock;

/*
  Tipo ModeInfoBlock: in esso sono contenute tutte le caratteristiche
                      del modo grafico che vogliamo attivare.
*/
typedef struct {
  unsigned short int ModeAttributes;         /* Specifiche del modo	*/
  unsigned char      WinAAttributes;         /* Caratt. della window A	*/
  unsigned char      WinBAttributes;         /* Caratt. della window B	*/
  unsigned short int WinGranularity;         /* Granularit?-> window	*/
  unsigned short int WinSize;                /* Dimensione  -> window	*/
  unsigned short int WinASegment;            /* Indirizzo window A	*/
  unsigned short int WinBSegment;            /* Indirizzo window B	*/
  void	      (*WPF) (signed long int page); /* Indirizzo funzione	*/
  unsigned short int BytesPerScanLine;

  unsigned short int XResolution;            /* Larghezza in pixel	*/
  unsigned short int YResolution;            /* Altezza in pixel	*/
  unsigned char      XCharSize;              /* Larghezza carattere	*/
  unsigned char      YCharSize;              /* Altezza carattere	*/
  unsigned char      NumberOfPlanes;         /* Numero dei planes disponibili*/
  unsigned char      BitsPerPixel;           /* Num. bit per ogni pixel	*/
  unsigned char      NumberOfBanks;          /* Num. dei banchi presenti*/
  unsigned char      MemoryModel;            /* Tipo di memoria utilizzato*/
  unsigned char      BankSize;               /* Dimensione di ogni banco*/
  unsigned char      NumberOfImagePages;     /* Num. -1 di schermate	*/
  unsigned char      Reserved;               /* Riservato		*/

  unsigned char      RedMaskSize;            /* Maschera per rosso	*/
  unsigned char      RedFieldPosition;       /* Posizione bit rosso	*/
  unsigned char      GreenMaskSize;          /* Maschera per verde	*/
  unsigned char      GreenFieldPosition;     /* Posizione bit verde	*/
  unsigned char      BlueMaskSize;           /* Maschera per blu	*/
  unsigned char      BlueFieldPosition;      /* Posizione bit blu	*/
  unsigned char      RsvdMaskSize;           
  unsigned char      RsvdFieldPosition;      
  unsigned char      DirectColorModeInfo;    /* Caratt. colori modo diretto*/

  unsigned long int  PhysBasePtr;            /* Linear Frame Buffer	*/
  unsigned long int  OffScreenMemoryOffset;  /* Offset mem. "fuori schermo"*/
  unsigned long int  OffScreenMemSize;       /* Mem. disponibile ""  ""*/
  unsigned char      Reserved2 [206];        /* Riservato		*/
} ModeInfoBlock;

/****************************************************************************/
/*  PROTOTYPES                                                              */
/****************************************************************************/

int vbe_getinfo(void);
int vbe_check_id(void);
void vbe_showinfo(void);
DWORD vbe_getmodeinfo(ModeInfoBlock *ModeInfo, WORD Vbe_Mode);
int vbe_setmode (WORD Vbe_Mode);
WORD vbe_getbpr(ModeInfoBlock *ModeInfo);
DWORD vbe_getflb(void);
int vbe_setbank(ModeInfoBlock *ModeInfo, BYTE bank);
void vbe_showmodeinfo (ModeInfoBlock *ModeInfo);
int vbe_checkmode(WORD mode);

DWORD vbe_getmem(void);
__END_DECLS
