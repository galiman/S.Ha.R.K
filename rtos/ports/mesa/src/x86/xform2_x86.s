/* $Id: xform2_x86.s,v 1.1 2003/04/24 13:36:03 giacomo Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.5
 *
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * NOTE: Avoid using spaces in between '(' ')' and arguments, especially
 * with macros like CONST, LLBL that expand to CONCAT(...).  Putting spaces
 * in there will break the build on some platforms.
 */

#include "matypes.h"
#include "xform_args.h"

	SEG_TEXT

#define FP_ONE		1065353216
#define FP_ZERO		0

#define SRC0		REGOFF(0, ESI)
#define SRC1		REGOFF(4, ESI)
#define SRC2		REGOFF(8, ESI)
#define SRC3		REGOFF(12, ESI)
#define DST0		REGOFF(0, EDI)
#define DST1		REGOFF(4, EDI)
#define DST2		REGOFF(8, EDI)
#define DST3		REGOFF(12, EDI)
#define MAT0		REGOFF(0, EDX)
#define MAT1		REGOFF(4, EDX)
#define MAT2		REGOFF(8, EDX)
#define MAT3		REGOFF(12, EDX)
#define MAT4		REGOFF(16, EDX)
#define MAT5		REGOFF(20, EDX)
#define MAT6		REGOFF(24, EDX)
#define MAT7		REGOFF(28, EDX)
#define MAT8		REGOFF(32, EDX)
#define MAT9		REGOFF(36, EDX)
#define MAT10		REGOFF(40, EDX)
#define MAT11		REGOFF(44, EDX)
#define MAT12		REGOFF(48, EDX)
#define MAT13		REGOFF(52, EDX)
#define MAT14		REGOFF(56, EDX)
#define MAT15		REGOFF(60, EDX)


ALIGNTEXT16
GLOBL GLNAME( _mesa_x86_transform_points2_general )
GLNAME( _mesa_x86_transform_points2_general ):

#define FRAME_OFFSET 8
	PUSH_L( ESI )
	PUSH_L( EDI )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )

	TEST_L( ECX, ECX )
	JZ( LLBL(x86_p2_gr_done) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(4), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

ALIGNTEXT16
LLBL(x86_p2_gr_loop):

	FLD_S( SRC0 )			/* F4 */
	FMUL_S( MAT0 )
	FLD_S( SRC0 )			/* F5 F4 */
	FMUL_S( MAT1 )
	FLD_S( SRC0 )			/* F6 F5 F4 */
	FMUL_S( MAT2 )
	FLD_S( SRC0 )			/* F7 F6 F5 F4 */
	FMUL_S( MAT3 )

	FLD_S( SRC1 )			/* F0 F7 F6 F5 F4 */
	FMUL_S( MAT4 )
	FLD_S( SRC1 )			/* F1 F0 F7 F6 F5 F4 */
	FMUL_S( MAT5 )
	FLD_S( SRC1 )			/* F2 F1 F0 F7 F6 F5 F4 */
	FMUL_S( MAT6 )
	FLD_S( SRC1 )			/* F3 F2 F1 F0 F7 F6 F5 F4 */
	FMUL_S( MAT7 )

	FXCH( ST(3) )			/* F0 F2 F1 F3 F7 F6 F5 F4 */
	FADDP( ST0, ST(7) )		/* F2 F1 F3 F7 F6 F5 F4 */
	FXCH( ST(1) )			/* F1 F2 F3 F7 F6 F5 F4 */
	FADDP( ST0, ST(5) )		/* F2 F3 F7 F6 F5 F4 */
	FADDP( ST0, ST(3) )		/* F3 F7 F6 F5 F4 */
	FADDP( ST0, ST(1) )		/* F7 F6 F5 F4 */

	FXCH( ST(3) )			/* F4 F6 F5 F7 */
	FADD_S( MAT12 )
	FXCH( ST(2) )			/* F5 F6 F4 F7 */
	FADD_S( MAT13 )
	FXCH( ST(1) )			/* F6 F5 F4 F7 */
	FADD_S( MAT14 )
	FXCH( ST(3) )			/* F7 F5 F4 F6 */
	FADD_S( MAT15 )

	FXCH( ST(2) )			/* F4 F5 F7 F6 */
	FSTP_S( DST0 )			/* F5 F7 F6 */
	FSTP_S( DST1 )			/* F7 F6 */
	FXCH( ST(1) )			/* F6 F7 */
	FSTP_S( DST2 )			/* F7 */
	FSTP_S( DST3 )			/* */

LLBL(x86_p2_gr_skip):

	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(x86_p2_gr_loop) )

LLBL(x86_p2_gr_done):

	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET




ALIGNTEXT16
GLOBL GLNAME( _mesa_x86_transform_points2_perspective )
GLNAME( _mesa_x86_transform_points2_perspective ):

#define FRAME_OFFSET 12
	PUSH_L( ESI )
	PUSH_L( EDI )
	PUSH_L( EBX )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )

	TEST_L( ECX, ECX )
	JZ( LLBL(x86_p2_pr_done) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_4), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(4), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

	MOV_L( MAT14, EBX )

ALIGNTEXT16
LLBL(x86_p2_pr_loop):

	FLD_S( SRC0 )			/* F4 */
	FMUL_S( MAT0 )

	FLD_S( SRC1 )			/* F1 F4 */
	FMUL_S( MAT5 )

	FXCH( ST(1) )			/* F4 F1 */
	FSTP_S( DST0   )		/* F1 */
	FSTP_S( DST1   )		/* */
	MOV_L( EBX, DST2 )
	MOV_L( CONST(FP_ZERO), DST3 )

LLBL(x86_p2_pr_skip):

	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(x86_p2_pr_loop) )

LLBL(x86_p2_pr_done):

	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET




ALIGNTEXT16
GLOBL GLNAME( _mesa_x86_transform_points2_3d )
GLNAME( _mesa_x86_transform_points2_3d ):

#define FRAME_OFFSET 8
	PUSH_L( ESI )
	PUSH_L( EDI )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )

	TEST_L( ECX, ECX )
	JZ( LLBL(x86_p2_3dr_done) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_3), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(3), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

ALIGNTEXT16
LLBL(x86_p2_3dr_loop):

	FLD_S( SRC0 )			/* F4 */
	FMUL_S( MAT0 )
	FLD_S( SRC0 )			/* F5 F4 */
	FMUL_S( MAT1 )
	FLD_S( SRC0 )			/* F6 F5 F4 */
	FMUL_S( MAT2 )

	FLD_S( SRC1 )			/* F0 F6 F5 F4 */
	FMUL_S( MAT4 )
	FLD_S( SRC1 )			/* F1 F0 F6 F5 F4 */
	FMUL_S( MAT5 )
	FLD_S( SRC1 )			/* F2 F1 F0 F6 F5 F4 */
	FMUL_S( MAT6 )

	FXCH( ST(2) )			/* F0 F1 F2 F6 F5 F4 */
	FADDP( ST0, ST(5) )		/* F1 F2 F6 F5 F4 */
	FADDP( ST0, ST(3) )		/* F2 F6 F5 F4 */
	FADDP( ST0, ST(1) )		/* F6 F5 F4 */

	FXCH( ST(2) )			/* F4 F5 F6 */
	FADD_S( MAT12 )
	FXCH( ST(1) )			/* F5 F4 F6 */
	FADD_S( MAT13 )
	FXCH( ST(2) )			/* F6 F4 F5 */
	FADD_S( MAT14 )

	FXCH( ST(1) )			/* F4 F6 F5 */
	FSTP_S( DST0 )			/* F6 F5 */
	FXCH( ST(1) )			/* F5 F6 */
	FSTP_S( DST1 )			/* F6 */
	FSTP_S( DST2 )			/* */

LLBL(x86_p2_3dr_skip):

	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(x86_p2_3dr_loop) )

LLBL(x86_p2_3dr_done):

	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET




ALIGNTEXT16
GLOBL GLNAME( _mesa_x86_transform_points2_3d_no_rot )
GLNAME( _mesa_x86_transform_points2_3d_no_rot ):

#define FRAME_OFFSET 12
	PUSH_L( ESI )
	PUSH_L( EDI )
	PUSH_L( EBX )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )

	TEST_L( ECX, ECX )
	JZ( LLBL(x86_p2_3dnrr_done) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_3), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(3), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

	MOV_L( MAT14, EBX )

ALIGNTEXT16
LLBL(x86_p2_3dnrr_loop):

	FLD_S( SRC0 )			/* F4 */
	FMUL_S( MAT0 )

	FLD_S( SRC1 )			/* F1 F4 */
	FMUL_S( MAT5 )

	FXCH( ST(1) )			/* F4 F1 */
	FADD_S( MAT12 )
	FLD_S( MAT13 )		/* F5 F4 F1 */
	FXCH( ST(2) )			/* F1 F4 F5 */
	FADDP( ST0, ST(2) )		/* F4 F5 */

	FSTP_S( DST0 )		/* F5 */
	FSTP_S( DST1 )		/* */
	MOV_L( EBX, DST2 )

LLBL(x86_p2_3dnrr_skip):

	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(x86_p2_3dnrr_loop) )

LLBL(x86_p2_3dnrr_done):

	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET




ALIGNTEXT16
GLOBL GLNAME( _mesa_x86_transform_points2_2d )
GLNAME( _mesa_x86_transform_points2_2d ):

#define FRAME_OFFSET 8
	PUSH_L( ESI )
	PUSH_L( EDI )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )

	TEST_L( ECX, ECX )
	JZ( LLBL(x86_p2_2dr_done) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(2), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

ALIGNTEXT16
LLBL(x86_p2_2dr_loop):

	FLD_S( SRC0 )			/* F4 */
	FMUL_S( MAT0 )
	FLD_S( SRC0 )			/* F5 F4 */
	FMUL_S( MAT1 )

	FLD_S( SRC1 )			/* F0 F5 F4 */
	FMUL_S( MAT4 )
	FLD_S( SRC1 )			/* F1 F0 F5 F4 */
	FMUL_S( MAT5 )

	FXCH( ST(1) )			/* F0 F1 F5 F4 */
	FADDP( ST0, ST(3) )		/* F1 F5 F4 */
	FADDP( ST0, ST(1) )		/* F5 F4 */

	FXCH( ST(1) )			/* F4 F5 */
	FADD_S( MAT12 )
	FXCH( ST(1) )			/* F5 F4 */
	FADD_S( MAT13 )

	FXCH( ST(1) )			/* F4 F5 */
	FSTP_S( DST0 )		/* F5 */
	FSTP_S( DST1 )		/* */

LLBL(x86_p2_2dr_skip):

	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(x86_p2_2dr_loop) )

LLBL(x86_p2_2dr_done):

	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET




ALIGNTEXT4
GLOBL GLNAME( _mesa_x86_transform_points2_2d_no_rot )
GLNAME( _mesa_x86_transform_points2_2d_no_rot ):

#define FRAME_OFFSET 8
	PUSH_L( ESI )
	PUSH_L( EDI )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )

	TEST_L( ECX, ECX )
	JZ( LLBL(x86_p2_2dnrr_done) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(2), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

ALIGNTEXT16
LLBL(x86_p2_2dnrr_loop):

	FLD_S( SRC0 )			/* F4 */
	FMUL_S( MAT0 )

	FLD_S( SRC1 )			/* F1 F4 */
	FMUL_S( MAT5 )

	FXCH( ST(1) )			/* F4 F1 */
	FADD_S( MAT12 )
	FLD_S( MAT13 )		/* F5 F4 F1 */
	FXCH( ST(2) )			/* F1 F4 F5 */
	FADDP( ST0, ST(2) )		/* F4 F5 */

	FSTP_S( DST0   )		/* F5 */
	FSTP_S( DST1   )		/* */

LLBL(x86_p2_2dnrr_skip):

	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(x86_p2_2dnrr_loop) )

LLBL(x86_p2_2dnrr_done):

	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET




ALIGNTEXT16
GLOBL GLNAME( _mesa_x86_transform_points2_identity )
GLNAME( _mesa_x86_transform_points2_identity ):

#define FRAME_OFFSET 12
	PUSH_L( ESI )
	PUSH_L( EDI )
	PUSH_L( EBX )

	MOV_L( ARG_SOURCE, ESI )
	MOV_L( ARG_DEST, EDI )

	MOV_L( ARG_MATRIX, EDX )
	MOV_L( REGOFF(V4F_COUNT, ESI), ECX )

	TEST_L( ECX, ECX )
	JZ( LLBL(x86_p2_ir_done) )

	MOV_L( REGOFF(V4F_STRIDE, ESI), EAX )
	OR_L( CONST(VEC_SIZE_2), REGOFF(V4F_FLAGS, EDI) )

	MOV_L( ECX, REGOFF(V4F_COUNT, EDI) )
	MOV_L( CONST(2), REGOFF(V4F_SIZE, EDI) )

	SHL_L( CONST(4), ECX )
	MOV_L( REGOFF(V4F_START, ESI), ESI )

	MOV_L( REGOFF(V4F_START, EDI), EDI )
	ADD_L( EDI, ECX )

	CMP_L( ESI, EDI )
	JE( LLBL(x86_p2_ir_done) )

ALIGNTEXT16
LLBL(x86_p2_ir_loop):

	MOV_L( SRC0, EBX )
	MOV_L( SRC1, EDX )

	MOV_L( EBX, DST0 )
	MOV_L( EDX, DST1 )

LLBL(x86_p2_ir_skip):

	ADD_L( CONST(16), EDI )
	ADD_L( EAX, ESI )
	CMP_L( ECX, EDI )
	JNE( LLBL(x86_p2_ir_loop) )

LLBL(x86_p2_ir_done):

	POP_L( EBX )
	POP_L( EDI )
	POP_L( ESI )
	RET
#undef FRAME_OFFSET
