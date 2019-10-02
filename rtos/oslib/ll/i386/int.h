
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

#ifndef __LL_I386_INT__
#define __LL_I386_INT__

#include <ll/i386/linkage.h>

#define INT(n) \
.globl SYMBOL_NAME(h##n) ; \
SYMBOL_NAME_LABEL(h##n)  ; \
	call debug_info  ; \
        pushal           ; \
	movl $##n, %eax	 ; \
	jmp ll_handler

#define INT_1(n) \
.globl SYMBOL_NAME(h##n) ; \
SYMBOL_NAME_LABEL(h##n)  ; \
        pushal           ; \
	movl $##n, %eax	 ; \
	jmp ll_handler_master_pic

#define INT_2(n) \
.globl SYMBOL_NAME(h##n) ; \
SYMBOL_NAME_LABEL(h##n)  ; \
        pushal           ; \
	movl $##n, %eax	 ; \
	jmp ll_handler_slave_pic

#define VM86(n) \
.globl SYMBOL_NAME(h##n) ; \
SYMBOL_NAME_LABEL(h##n)  ; \
        pushal           ; \
        jmp ll_handler_vm86

#define EXC(n) \
.globl SYMBOL_NAME(exc##n) ; \
SYMBOL_NAME_LABEL(exc##n)  ; \
	movl	$##n, %eax ; \
	jmp	ll_handler2
#endif

#define NONE(n) \
.globl SYMBOL_NAME(h##n)   ; \
SYMBOL_NAME_LABEL(h##n)    ; \
        iret               ; \
#endif

