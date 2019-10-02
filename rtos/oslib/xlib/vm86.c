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

/* File: Vm86.C
 *	   			
 * VM86 mode switch routines!
 * This is basically an alternative way of invoking the
 * BIOS service routines; it is very useful to support
 * native VBE compliant Video card, without writing an explicit driver
 */

#include <ll/i386/hw-data.h>
#include <ll/i386/hw-instr.h>
#include <ll/i386/hw-func.h>
#include <ll/i386/mem.h>
#include <ll/i386/x-bios.h>
#include <ll/i386/x-dosmem.h>
#include <ll/i386/cons.h>
#include <ll/i386/error.h>
#include <ll/i386/apic.h>
#include <ll/i386/advtimer.h>

FILE(VM-86);

/*
#define __LL_DEBUG__
#define __DUMB_CODE__
#define __CHK_IO__
*/

//#define __LL_DEBUG__

#define VM86_STACK_SIZE 8192 

extern DWORD ll_irq_table[256];
extern unsigned char use_apic, use_tsc;

/* TSS optional section */
static BYTE  vm86_stack0[VM86_STACK_SIZE];

static BYTE  init = 0;

static struct {
    TSS t;
    DWORD io_map[2048];
} vm86_TSS;
static LIN_ADDR vm86_stack;
static LIN_ADDR vm86_iretAddress;

struct registers *global_regs;
WORD   VM86_ret_ctx;

#ifdef __DUMB_CODE__
static LIN_ADDR vm86_code;
static BYTE prova86[] = {
	                0x1e,			/* push ds		*/
	                0xb8,0x00,0xb8,         /* mov ax,0xb800	*/
                        0x8e,0xd8,              /* mov ds,ax		*/
                        0xbf,0x9e,0x00,         /* mov di,0x009e (158)	*/
                        0xb0,0x2a,         	/* mov ax,'*'   	*/
                        0x88,0x05,         	/* mov ds:[di],al	*/
                        0x1f,			/* pop ds		*/
			0xcd, 0x40,		/*???*/
#ifdef __CHK_IO__		
			0xb0, 0x00,             /* movb   $0x0,%al*/
			0x66,0xba, 0x80, 0x00, 	/* movw   $0x80,%dx */
			0x66,0xef,   		/* outw %ax, (%dx) */
#endif		
			0xcf,                   /* iret			*/
                        0xf4,                   /* hlt			*/
                        0};    
#endif

static BYTE vm86_retAddr[] = {0xcd, 0x48,       /* int 48h              */
			      0xf4,
			      0};

TSS *vm86_get_tss(void)
{
    return &(vm86_TSS.t);
}

/* Just a debugging function; it dumps the status of the TSS */
void vm86_dump_TSS(void)
{
    BYTE acc,gran;
    DWORD base,lim;
    message("vm86_TSS.t dump\n");
    message("Flag: %lx\n",vm86_TSS.t.eflags);
    message("SS: %hx SP:%lx\n", vm86_TSS.t.ss,vm86_TSS.t.esp);
    message("Stack0: %hx:%lx\n",vm86_TSS.t.ss0,vm86_TSS.t.esp0);
    message("Stack1: %hx:%lx\n",vm86_TSS.t.ss1,vm86_TSS.t.esp1);
    message("Stack2: %hx:%lx\n",vm86_TSS.t.ss2,vm86_TSS.t.esp2);
    message("CS: %hx IP: %lx",vm86_TSS.t.cs, vm86_TSS.t.eip);
    message("DS: %hx\n",vm86_TSS.t.ds);
    base = GDT_read(X_VM86_TSS,&lim,&acc,&gran);
    message("Base : %lx Lim : %lx Acc : %x Gran %x\n",
		    base,lim,(unsigned)(acc),(unsigned)(gran));
}

void vm86_init(void)
{
    int register i;
    
    if (init != 0) return;
    init = 1;

    /* First of all, we need to setup a GDT entries to
     * allow vm86 task execution. We just need a free 386 TSS, which
     * will be used to store the execution context of the virtual 8086
     * task
     */
    GDT_place(X_VM86_TSS,(DWORD)(&vm86_TSS),
              sizeof(vm86_TSS),FREE_TSS386,GRAN_16);

    /* Return Registers */
    global_regs = DOS_alloc(sizeof(struct registers));

    /* Prepare a real-mode stack, obtaining it from the
     * DOS memory allocator!
     * 8K should be OK! Stack top is vm86_stack + SIZE!
     */
    vm86_stack = DOS_alloc(VM86_STACK_SIZE*2);
    vm86_stack += VM86_STACK_SIZE/2;
    
    vm86_iretAddress = DOS_alloc(sizeof(vm86_retAddr));
    memcpy(vm86_iretAddress,vm86_retAddr,sizeof(vm86_retAddr));
#ifdef __LL_DEBUG__
    message("PM reentry linear address=0x%lx\n", (DWORD)vm86_iretAddress);
#endif
#ifdef __DUMB_CODE__
    vm86_code = DOS_alloc(2048);
    lmemcpy(vm86_code,prova86,sizeof(prova86));
#endif
    /* Zero the PM/Ring[1,2] ss:esp; they're unused! */
    vm86_TSS.t.esp1 = 0;
    vm86_TSS.t.esp2 = 0;
    vm86_TSS.t.ss1 = 0;
    vm86_TSS.t.ss2 = 0;
    /* Use only the GDT */
    vm86_TSS.t.ldt = 0;
    /* No paging activated */
    vm86_TSS.t.cr3 = 0;
    vm86_TSS.t.trap = 0;
    /* Yeah, free access to any I/O port; we trust BIOS anyway! */
    /* Here is the explanation: we have 65536 I/O ports... each bit
     * in the io_map masks/unmasks the exception for the given I/O port
     * If the bit is set, an exception is generated; otherwise, if the bit
     * is clear, everythings works fine...
     * Because of alignment problem, we need to add an extra byte all set
     * to 1, according to Intel manuals
     */
    vm86_TSS.t.io_base = (DWORD)(&(vm86_TSS.io_map)) -
	    		(DWORD)(&(vm86_TSS));
    for (i = 0; i < 2047; i++) vm86_TSS.io_map[i] = 0;
    vm86_TSS.io_map[2047] = 0xFF000000;
}

int vm86_callBIOS(int service,X_REGS16 *in,X_REGS16 *out,X_SREGS16 *s)
{
    DWORD vm86_tmpAddr;
    DWORD vm86_flags, vm86_cs,vm86_ip;
    LIN_ADDR vm86_stackPtr;
    DWORD *IRQTable_entry;
    BYTE p1,p2;
    DWORD msr1 = 0,msr2 = 0;

    SYS_FLAGS f;

    if (service < 0x10 || in == NULL) return -1;
    
    f = ll_fsave();

    /* Setup the stack frame */
    vm86_tmpAddr = (DWORD)(vm86_stack);
    vm86_TSS.t.ss = (vm86_tmpAddr & 0xFF000) >> 4;
    vm86_TSS.t.ebp = vm86_TSS.t.esp = (vm86_tmpAddr & 0x0FFF)
	    	+ VM86_STACK_SIZE - 6;
    /* Build an iret stack frame which returns to vm86_iretAddress */
    vm86_tmpAddr = (DWORD)(vm86_iretAddress);
    vm86_cs = (vm86_tmpAddr & 0xFF000) >> 4;
    vm86_ip = (vm86_tmpAddr & 0xFFF);
    vm86_flags = 0;
    vm86_stackPtr = vm86_stack + VM86_STACK_SIZE;
    lmempokew(vm86_stackPtr-6,vm86_ip);
    lmempokew(vm86_stackPtr-4,vm86_cs);
    lmempokew(vm86_stackPtr-2,vm86_flags);
#ifdef __LL_DEBUG__
    message("Stack: %lx SS: %lx SP: %lx\n",
	vm86_tmpAddr + VM86_STACK_SIZE,(DWORD)vm86_TSS.t.ss,vm86_TSS.t.esp);
#endif
    /* Wanted VM86 mode + IOPL = 3! */
    vm86_TSS.t.eflags = CPU_FLAG_VM + CPU_FLAG_IOPL;
    /* Preload some standard values into the registers */
    vm86_TSS.t.ss0 = X_FLATDATA_SEL;
    vm86_TSS.t.esp0 = (DWORD)&(vm86_stack0[VM86_STACK_SIZE-1]); 
    
#ifdef __DUMB_CODE__
    vm86_TSS.t.cs = ((DWORD)(vm86_code) & 0xFFFF0) >> 4;
    vm86_TSS.t.eip = ((DWORD)(vm86_code) & 0x000F);
#ifdef __LL_DEBUG_
    message("(DUMB CODE) CS:%x IP:%x/%x\n",
        (DWORD)vm86_TSS.t.cs,vm86_TSS.t.eip,&prova86);
    message("(DUMB CODE) Go...\n");
#endif

    p1 = inp(0x21);
    p2 = inp(0xA1);
    outp(0x21,0xFF);
    outp(0xA1,0xFF);

    if (use_apic) {
      rdmsr(APIC_BASE_MSR,msr1,msr2);
      disable_APIC_timer();
    }

    vm86_TSS.t.back_link = ll_context_save();
    VM86_ret_ctx = vm86_TSS.t.back_link
    sti();
    ll_context_load(X_VM86_TSS);
    cli();

    if (use_apic) {
      wrmsr(APIC_BASE_MSR,msr1,msr2);
      enable_APIC_timer();
    }

    outp(0x21,p1);
    outp(0xA1,p2);

#ifdef __LL_DEBUG_
    message("(DUMB CODE) I am back...\n");
#endif
#else
    /* Copy the parms from the X_*REGS structures in the vm86 TSS */
    vm86_TSS.t.eax = (DWORD)in->x.ax;
    vm86_TSS.t.ebx = (DWORD)in->x.bx;
    vm86_TSS.t.ecx = (DWORD)in->x.cx;
    vm86_TSS.t.edx = (DWORD)in->x.dx;
    vm86_TSS.t.esi = (DWORD)in->x.si;
    vm86_TSS.t.edi = (DWORD)in->x.di;
    /* IF Segment registers are required, copy them... */
    if (s != NULL) {
        vm86_TSS.t.es = (WORD)s->es;
        vm86_TSS.t.ds = (WORD)s->ds;
    } else {
        vm86_TSS.t.ds = vm86_TSS.t.ss;
        vm86_TSS.t.es = vm86_TSS.t.ss; 
    }
    vm86_TSS.t.gs = vm86_TSS.t.ss; 
    vm86_TSS.t.fs = vm86_TSS.t.ss; 
    /* Execute the BIOS call, fetching the CS:IP of the real interrupt
     * handler from 0:0 (DOS irq table!)
     */
    IRQTable_entry = (void *)(0L);
    vm86_TSS.t.cs= ((IRQTable_entry[service]) & 0xFFFF0000) >> 16;
    vm86_TSS.t.eip = ((IRQTable_entry[service]) & 0x0000FFFF);
#ifdef __LL_DEBUG__    
    message("CS:%x IP:%lx\n", vm86_TSS.t.cs, vm86_TSS.t.eip);
#endif
    /* Let's use the ll standard call... */

    p1 = inp(0x21);
    p2 = inp(0xA1);
    outp(0x21,0xFF);
    outp(0xA1,0xFF);

    if (use_apic) {
      rdmsr(APIC_BASE_MSR,msr1,msr2);
      disable_APIC_timer();
    }

    vm86_TSS.t.back_link = ll_context_save();
    VM86_ret_ctx = vm86_TSS.t.back_link;     
    sti();
    ll_context_load(X_VM86_TSS);
    cli();

    if (use_apic) {
      wrmsr(APIC_BASE_MSR,msr1,msr2);
      enable_APIC_timer();
    }

    outp(0x21,p1);
    outp(0xA1,p2);

#ifdef __LL_DEBUG__    
    message("I am back...\n");
    message("TSS CS=%hx IP=%lx\n", vm86_TSS.t.cs, vm86_TSS.t.eip);
#endif
    /* Send back in the X_*REGS structure the value obtained with
     * the real-mode interrupt call
     */
    if (out != NULL) {
      
      out->x.ax = global_regs->eax;
      out->x.bx = global_regs->ebx;
      out->x.cx = global_regs->ecx;
      out->x.dx = global_regs->edx;
      out->x.si = global_regs->esi;
      out->x.di = global_regs->edi;
      out->x.cflag = global_regs->flags;
 
      //message("ax = %d bx = %d cx = %d dx = %d\n",out->x.ax,out->x.bx,out->x.cx,out->x.dx);
      //message("si = %d di = %d\n",out->x.si,out->x.di);

    }
    if (s != NULL) {
      s->es = vm86_TSS.t.es;
      s->ds = vm86_TSS.t.ds;
    }
#endif

    ll_frestore(f);

    return 1;

}
