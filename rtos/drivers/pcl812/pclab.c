/*==============================================================*/
/*  LIBRERIA DI GESTIONE DELLA SCHEDA PCL-812                   */
/*      Marco Caccamo, 6-2-2000                                 */
/*==============================================================*/

/*--------------------------------------------------------------*/
/* Register bit:    7    6    5    4    3    2    1    0        */
/*--------------------------------------------------------------*/
/* ABUFL        D7   D6   D5   D4   D3   D2   D1   D0           */
/* ABUFH        0    0    0   DRDY  D3   D2   D1   D0           */
/*--------------------------------------------------------------*/
/* SELEC        X    X    X    X    C3   C2   C1   C0           */
/*--------------------------------------------------------------*/
/* CTRLB        X    X    X    X    X    S2   S1   S0           */
/*--------------------------------------------------------------*/
/* TRIGG        X    X    X    X    X    X    X    TR           */



#include <kernel/kern.h>
#define BASE    0x220       /* indirizzo base     BIT       */
#define ABUFL   (BASE + 4)  /* low byte for AD/DA   (0 - 7) */
#define ABUFH   (BASE + 5)  /* high byte for AD/DA  (8 -13) */
#define IBUFL   (BASE + 6)  /* low byte for DIN (0 - 7)     */
#define IBUFH   (BASE + 7)  /* high byte for DIN    (8 -15) */
#define SELEC   (BASE + 10) /* select AD channel    (0 - 3) */
#define CTRLB   (BASE + 11) /* control register     (0 - 2) */
#define TRIGG   (BASE + 12) /* A/D trigger control  (0)     */  
#define OBUFL   (BASE + 13) /* low byte for DOUT    (0 - 7) */
#define OBUFH   (BASE + 14) /* high byte for DOUT   (8 -15) */

/*--------------------------------------------------------------*/
/*  AD_CONV(ch)     ritorna il valore (in volt) convertito      */
/*          dal canale ch dell'ingresso analogico.              */
/*          Volt range: [-10,10], Resolution: 2.4 mV              */
/*--------------------------------------------------------------*/

float   ad_conv(int ch)         /* AD channel [0-15]        */
{
int     lb, hb;                  /* low byte, high byte [0,255]  */
int     n;                       /* converted value [-8192,8191] */
float   v;                       /* value in volt     [-10,10] */

    outp(SELEC, ch);     /* set AD channel   */
    outp(CTRLB, 1);      /* enable software trigger */
    outp(TRIGG, 1);      /* trigger AD converter */

    do {                            /* wait conversion  */
      hb = 0xff & inp(ABUFH); /* read high byte   */
    } while ((hb & 0x10) == 16);    /* loop if (bit 4 == 1) */

    lb = 0xff & inp(ABUFL);         /* read low byte    */

    n = (hb * 256 + lb); // -4096; //- 2048;     /* compose number   */
    //cprintf();
    v = (20. * (float)n ) / 4096 - 10.; //2048.;      /* convert n in volt    */
    return(v);
}

/*--------------------------------------------------------------*/
/*  DA_CONV(float v,int ch) converte in analogico il valore in volt */
/*  passato come parametro sul canale ch.                     */
/*  Volt range: [0,5], Resolution: 1.2 mV                       */
/*  return(-1) se non viene eseguita la conversione             */
/*  altrimenti return(0)                                        */
/*--------------------------------------------------------------*/

int da_conv(float v, int ch)   /* value (in volt) to convert   */
{
int lb, hb;             /* low byte, high byte  */
float   x;              /* value to convert */



    if (v  > 5 || v < 0)
      return(-1);
    else{
      x = ((v / 5.) * 4095.);     /* compose number   */
      
      hb = (int) x / 256;        /* compute high byte    */
      lb = (int) x % 256;        /* compute low byte */
     
      if(ch == 2) { 
	outp(IBUFL, lb);          /* write lb in IBUFL    */
	outp(IBUFH, hb);          /* write hb in IBUFL    */
      }
      else if(ch == 1) {
	outp(ABUFL, lb);          /* write lb in ABUFL    */
	outp(ABUFH, hb);          /* write hb in ABUFH    */
      }
      else return(-1);
      
      return(0);
    }
}

/*--------------------------------------------------------------*/
/*  PAR_IN()        ritorna il valore letto sui 16 bit          */
/*          della porta parallela di ingresso.                  */
/*--------------------------------------------------------------*/

int par_in()
{
int lb, hb;             /* low byte, high byte  */
int n;                  /* value on 16 bit  */

    lb = 0xff & inp(IBUFL);     /* read low byte    */
    hb = 0xff & inp(IBUFH);     /* read high byte   */

    n = hb * 256 + lb;          /* compose number   */
    return(n);
}

/*--------------------------------------------------------------*/
/*  PAR_OUT(n)      scrive il valore n sui 16 bit               */
/*          della porta parallela di uscita.                    */
/*--------------------------------------------------------------*/

void    par_out(n)
int n;                  /* value to write   */
{
int lb, hb;             /* low byte, high byte  */

    hb = n / 256;           /* extract high byte    */
    lb = n % 256;           /* extract low byte */

    outp(OBUFL,lb);         /* write low byte   */
    outp(OBUFH,hb);         /* write high byte  */
}

/*--------------------------------------------------------------*/
