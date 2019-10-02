/* Project:     OSLib
 * Description: The OS Construction Kit
 * Date:                1.6.2000
 * Idea by:             Luca Abeni & Gerardo Lamastra
 *
 * OSLib is an SO project aimed at developing a common, easy-to-use
 * low-level infrastructure for developing OS kernels and Embedded
 * Applications; it partially derives from the HARTIK project but it
 * currently is independently developed.
 *
 * OSLib is distributed under GPL License, and some of its code has
 * been derived from the Linux kernel source; also some important
 * ideas come from studying the DJGPP go32 extender.
 *
 * We acknowledge the Linux Community, Free Software Foundation,
 * D.J. Delorie and all the other developers who believe in the
 * freedom of software and ideas.
 *
 * For legalese, check out the included GPL license.
 */

/*	Xlib initialization code	*/

#include <ll/i386/mem.h>
#include <ll/i386/cons.h>
#include <ll/i386/mb-info.h>
#include <ll/i386/error.h>
#include <ll/i386/pit.h>
#include <ll/i386/pic.h>

#include <ll/i386/tss-ctx.h>
#include <ll/i386/hw-arch.h>

FILE(X-Init);

extern DWORD ll_irq_table[256];

#ifdef __VIRCSW__
int activeInt = 0;
#endif

/* Assembly external routines!      */
/* Setup the TR register of the 80386, to initialize context switch */

extern void init_TR(WORD v);
TSS main_tss;

/* Architecture definition */
LL_ARCH ll_arch;

/* The following stuff is in llCxA.Asm/S    */

static void dummyfun(int i)
{
#if 0
  if (i < 32) {
    cputs("Unhandled Exc occured!!!\n");
  } else {
    cputs("Unhandled Int occured!!!\n");
  }
#else
  message("Unhandled Exc or Int %d occured!!!\n", i);
#endif
  halt();
}

void l1_int_bind(int i, void *f)
{
  ll_irq_table[i] = (DWORD)f;
}

void *l1_init(void)
{
  register int i;
  struct ll_cpuInfo cpuInfo;
  extern unsigned char X86_apic;
  extern unsigned char X86_tsc;
  extern BYTE X86_fpu;
  LIN_ADDR b;
 
  for(i = 0; i < 256; i++) {
    ll_irq_table[i] = (DWORD)dummyfun;
  }
  
  X86_get_CPU(&cpuInfo);
  X86_get_FPU();
  ll_arch.x86.arch = __LL_ARCH__;
  ll_arch.x86.cpu = cpuInfo.X86_cpu;
  ll_arch.x86.fpu = X86_fpu;
  memcpy(&(ll_arch.x86.vendor), &(cpuInfo.X86_vendor_1), 12);
  
  X86_apic = (cpuInfo.X86_StandardFeature>>9) & 1;
  X86_tsc  = (cpuInfo.X86_StandardFeature>>4) & 1;
  
  /* TODO! Need to map featuresXXX & Signature onto ll_arch!  */
  /* TODO! Need to check for CPU bugs!!           */
  
#ifdef __LL_DEBUG__
  message("LL Architecture: %s\n", __LL_ARCH__);
  message("CPU : %u\nFPU : %u\n", cpuInfo.X86_cpu, X86_fpu);
  message("Signature : 0x%lx\nVendor: %s\n", cpuInfo.X86_signature,
	  ll_arch.x86.vendor);
  message("Features #1: 0x%lx\n", cpuInfo.X86_IntelFeature_1);
  message("Features #2: 0x%lx\n", cpuInfo.X86_IntelFeature_2);
  message("Features #3: 0x%lx\n", cpuInfo.X86_StandardFeature);
  message("Has APIC: %s\n", X86_apic);
  message("Has TSC: %s\n", X86_tsc);
#endif /* __LL_DEBUG__ */
  
  IDT_init();

  /* Init coprocessor & assign it to main() */
  /* OK... Now I know the sense of all this... :
     We need a initial value for the FPU context (to be used for creating
     new FPU contexts, as init value)...
     ... And we get it in this strange way!!!!
  */
  reset_fpu();
  init_fpu();
  
  /* Init PIC controllers & unmask timer */
  PIC_init();
  
  /* Set the TR initial value */
  b = (LIN_ADDR)(&main_tss);
  GDT_place(MAIN_SEL, (DWORD)b, sizeof(TSS), FREE_TSS386, GRAN_16);
  init_TR(MAIN_SEL);
  
  return mbi_address();
}


void l1_end(void)
{
  outp(0x21,0xFF);
  outp(0xA1,0xFF);
  /* Back to DOS settings */
  PIC_end();
  /* Reset the timer chip according DOS specification */
  /* Mode: Binary/Mode 3/16 bit Time_const/Counter 0 */
#if 0
  outp(0x43,0x36);
  /* Time_const = 65536; write 0 in CTR */
  outp(0x40,0);
  outp(0x40,0);
#endif
  pit_init(0, TMR_MD3, 0);    /* Timer 0, Mode 3, Time constant 0 */
  if(ll_arch.x86.cpu > 4) {	
    pit_init(1, TMR_MD2, 18);
  } else {
    pit_init(2, TMR_MD0, 0);
    outp(0x61, 0);              /* Stop channel 2 */
  }
}
