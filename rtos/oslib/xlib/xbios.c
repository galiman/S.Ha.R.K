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

/* System calls for X extender 			  	*/
/* BIOS Call from X (Basic Reflection Service)		*/

#include <ll/i386/hw-data.h>
#include <ll/i386/x-bios.h>
#include <ll/i386/mem.h>

#include <ll/i386/advtimer.h>
#include <ll/i386/apic.h>

FILE(X-BIOS);

/* The interface between X (and 32 bit PM) and BIOS (which runs at 16 bits */
/* It works as int86() standard library call				   */

void X_callBIOS(int service,X_REGS16 *in,X_REGS16 *out,X_SREGS16 *s)
{
    /* Assembler gate JMP instruction */
    extern unsigned char use_apic;
    extern void _x_callBIOS(void);
    X_CALLBIOS *xbc = x_bios_address();
    DWORD msr1 = 0,msr2 = 0;

    /* Send interrupt request & register through the X_Info structure */
    xbc->_irqno = service;
    memcpy(&(xbc->_ir),in,sizeof(X_REGS16));
    memcpy(&(xbc->_sr),s,sizeof(X_SREGS16));
    if (use_apic) {
      rdmsr(APIC_BASE_MSR,msr1,msr2);
      disable_APIC_timer();    
    }
    /* Back to RM to execute the BIOS routine */
    _x_callBIOS();

    /* Get the return register values */
    if (use_apic) {
      wrmsr(APIC_BASE_MSR,msr1,msr2);
      enable_APIC_timer();
    }
    memcpy(out,&(xbc->_or),sizeof(X_REGS16));
    memcpy(s,&(xbc->_sr),sizeof(X_SREGS16));
}
