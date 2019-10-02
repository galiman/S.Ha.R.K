/*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 *
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

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Tue May 18 13:56:05 EDT 1999 */

#include <fftw-int.h>
#include <fftw.h>

/* Generated by: ./genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -hc2hc-backward 9 */

/*
 * This function contains 181 FP additions, 120 FP multiplications,
 * (or, 129 additions, 68 multiplications, 52 fused multiply/add),
 * 43 stack variables, and 72 memory accesses
 */
static const fftw_real K663413948 = FFTW_KONST(+0.663413948168938396205421319635891297216863310);
static const fftw_real K556670399 = FFTW_KONST(+0.556670399226419366452912952047023132968291906);
static const fftw_real K852868531 = FFTW_KONST(+0.852868531952443209628250963940074071936020296);
static const fftw_real K296198132 = FFTW_KONST(+0.296198132726023843175338011893050938967728390);
static const fftw_real K150383733 = FFTW_KONST(+0.150383733180435296639271897612501926072238258);
static const fftw_real K813797681 = FFTW_KONST(+0.813797681349373692844693217248393223289101568);
static const fftw_real K939692620 = FFTW_KONST(+0.939692620785908384054109277324731469936208134);
static const fftw_real K342020143 = FFTW_KONST(+0.342020143325668733044099614682259580763083368);
static const fftw_real K984807753 = FFTW_KONST(+0.984807753012208059366743024589523013670643252);
static const fftw_real K173648177 = FFTW_KONST(+0.173648177666930348851716626769314796000375677);
static const fftw_real K300767466 = FFTW_KONST(+0.300767466360870593278543795225003852144476517);
static const fftw_real K1_705737063 = FFTW_KONST(+1.705737063904886419256501927880148143872040591);
static const fftw_real K642787609 = FFTW_KONST(+0.642787609686539326322643409907263432907559884);
static const fftw_real K766044443 = FFTW_KONST(+0.766044443118978035202392650555416673935832457);
static const fftw_real K1_326827896 = FFTW_KONST(+1.326827896337876792410842639271782594433726619);
static const fftw_real K1_113340798 = FFTW_KONST(+1.113340798452838732905825904094046265936583811);
static const fftw_real K500000000 = FFTW_KONST(+0.500000000000000000000000000000000000000000000);
static const fftw_real K866025403 = FFTW_KONST(+0.866025403784438646763723170752936183471402627);
static const fftw_real K2_000000000 = FFTW_KONST(+2.000000000000000000000000000000000000000000000);
static const fftw_real K1_732050807 = FFTW_KONST(+1.732050807568877293527446341505872366942805254);

/*
 * Generator Id's : 
 * $Id: fhb_9.c,v 1.2 2003/03/24 11:14:57 pj Exp $
 * $Id: fhb_9.c,v 1.2 2003/03/24 11:14:57 pj Exp $
 * $Id: fhb_9.c,v 1.2 2003/03/24 11:14:57 pj Exp $
 */

void fftw_hc2hc_backward_9(fftw_real *A, const fftw_complex *W, int iostride, int m, int dist)
{
     int i;
     fftw_real *X;
     fftw_real *Y;
     X = A;
     Y = A + (9 * iostride);
     {
	  fftw_real tmp155;
	  fftw_real tmp183;
	  fftw_real tmp177;
	  fftw_real tmp162;
	  fftw_real tmp165;
	  fftw_real tmp160;
	  fftw_real tmp169;
	  fftw_real tmp180;
	  fftw_real tmp172;
	  fftw_real tmp181;
	  fftw_real tmp161;
	  fftw_real tmp166;
	  ASSERT_ALIGNED_DOUBLE();
	  {
	       fftw_real tmp176;
	       fftw_real tmp153;
	       fftw_real tmp154;
	       fftw_real tmp174;
	       fftw_real tmp175;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp175 = Y[-3 * iostride];
	       tmp176 = K1_732050807 * tmp175;
	       tmp153 = X[0];
	       tmp154 = X[3 * iostride];
	       tmp174 = tmp154 - tmp153;
	       tmp155 = tmp153 + (K2_000000000 * tmp154);
	       tmp183 = tmp176 - tmp174;
	       tmp177 = tmp174 + tmp176;
	  }
	  {
	       fftw_real tmp156;
	       fftw_real tmp159;
	       fftw_real tmp170;
	       fftw_real tmp168;
	       fftw_real tmp167;
	       fftw_real tmp171;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp156 = X[iostride];
	       tmp162 = Y[-iostride];
	       {
		    fftw_real tmp157;
		    fftw_real tmp158;
		    fftw_real tmp163;
		    fftw_real tmp164;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp157 = X[4 * iostride];
		    tmp158 = X[2 * iostride];
		    tmp159 = tmp157 + tmp158;
		    tmp170 = K866025403 * (tmp157 - tmp158);
		    tmp163 = Y[-2 * iostride];
		    tmp164 = Y[-4 * iostride];
		    tmp165 = tmp163 - tmp164;
		    tmp168 = K866025403 * (tmp164 + tmp163);
	       }
	       tmp160 = tmp156 + tmp159;
	       tmp167 = tmp156 - (K500000000 * tmp159);
	       tmp169 = tmp167 - tmp168;
	       tmp180 = tmp167 + tmp168;
	       tmp171 = (K500000000 * tmp165) + tmp162;
	       tmp172 = tmp170 + tmp171;
	       tmp181 = tmp171 - tmp170;
	  }
	  X[0] = tmp155 + (K2_000000000 * tmp160);
	  tmp161 = tmp155 - tmp160;
	  tmp166 = K1_732050807 * (tmp162 - tmp165);
	  X[6 * iostride] = tmp161 + tmp166;
	  X[3 * iostride] = tmp161 - tmp166;
	  {
	       fftw_real tmp178;
	       fftw_real tmp173;
	       fftw_real tmp179;
	       fftw_real tmp185;
	       fftw_real tmp182;
	       fftw_real tmp184;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp178 = (K1_113340798 * tmp169) + (K1_326827896 * tmp172);
	       tmp173 = (K766044443 * tmp169) - (K642787609 * tmp172);
	       tmp179 = tmp173 + tmp177;
	       X[iostride] = (K2_000000000 * tmp173) - tmp177;
	       X[7 * iostride] = tmp178 - tmp179;
	       X[4 * iostride] = -(tmp178 + tmp179);
	       tmp185 = (K1_705737063 * tmp180) + (K300767466 * tmp181);
	       tmp182 = (K173648177 * tmp180) - (K984807753 * tmp181);
	       tmp184 = tmp183 - tmp182;
	       X[2 * iostride] = (K2_000000000 * tmp182) + tmp183;
	       X[8 * iostride] = tmp185 + tmp184;
	       X[5 * iostride] = tmp184 - tmp185;
	  }
     }
     X = X + dist;
     Y = Y - dist;
     for (i = 2; i < m; i = i + 2, X = X + dist, Y = Y - dist, W = W + 8) {
	  fftw_real tmp45;
	  fftw_real tmp88;
	  fftw_real tmp136;
	  fftw_real tmp61;
	  fftw_real tmp108;
	  fftw_real tmp126;
	  fftw_real tmp50;
	  fftw_real tmp55;
	  fftw_real tmp56;
	  fftw_real tmp102;
	  fftw_real tmp110;
	  fftw_real tmp132;
	  fftw_real tmp138;
	  fftw_real tmp129;
	  fftw_real tmp137;
	  fftw_real tmp95;
	  fftw_real tmp109;
	  fftw_real tmp66;
	  fftw_real tmp71;
	  fftw_real tmp72;
	  ASSERT_ALIGNED_DOUBLE();
	  {
	       fftw_real tmp41;
	       fftw_real tmp57;
	       fftw_real tmp44;
	       fftw_real tmp106;
	       fftw_real tmp60;
	       fftw_real tmp87;
	       fftw_real tmp86;
	       fftw_real tmp107;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp41 = X[0];
	       tmp57 = Y[0];
	       {
		    fftw_real tmp42;
		    fftw_real tmp43;
		    fftw_real tmp58;
		    fftw_real tmp59;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp42 = X[3 * iostride];
		    tmp43 = Y[-6 * iostride];
		    tmp44 = tmp42 + tmp43;
		    tmp106 = K866025403 * (tmp42 - tmp43);
		    tmp58 = Y[-3 * iostride];
		    tmp59 = X[6 * iostride];
		    tmp60 = tmp58 - tmp59;
		    tmp87 = K866025403 * (tmp58 + tmp59);
	       }
	       tmp45 = tmp41 + tmp44;
	       tmp86 = tmp41 - (K500000000 * tmp44);
	       tmp88 = tmp86 - tmp87;
	       tmp136 = tmp86 + tmp87;
	       tmp61 = tmp57 + tmp60;
	       tmp107 = tmp57 - (K500000000 * tmp60);
	       tmp108 = tmp106 + tmp107;
	       tmp126 = tmp107 - tmp106;
	  }
	  {
	       fftw_real tmp46;
	       fftw_real tmp49;
	       fftw_real tmp89;
	       fftw_real tmp92;
	       fftw_real tmp62;
	       fftw_real tmp65;
	       fftw_real tmp90;
	       fftw_real tmp93;
	       fftw_real tmp51;
	       fftw_real tmp54;
	       fftw_real tmp96;
	       fftw_real tmp99;
	       fftw_real tmp67;
	       fftw_real tmp70;
	       fftw_real tmp97;
	       fftw_real tmp100;
	       ASSERT_ALIGNED_DOUBLE();
	       {
		    fftw_real tmp47;
		    fftw_real tmp48;
		    fftw_real tmp63;
		    fftw_real tmp64;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp46 = X[iostride];
		    tmp47 = X[4 * iostride];
		    tmp48 = Y[-7 * iostride];
		    tmp49 = tmp47 + tmp48;
		    tmp89 = tmp46 - (K500000000 * tmp49);
		    tmp92 = K866025403 * (tmp47 - tmp48);
		    tmp62 = Y[-iostride];
		    tmp63 = Y[-4 * iostride];
		    tmp64 = X[7 * iostride];
		    tmp65 = tmp63 - tmp64;
		    tmp90 = K866025403 * (tmp63 + tmp64);
		    tmp93 = tmp62 - (K500000000 * tmp65);
	       }
	       {
		    fftw_real tmp52;
		    fftw_real tmp53;
		    fftw_real tmp68;
		    fftw_real tmp69;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp51 = X[2 * iostride];
		    tmp52 = Y[-5 * iostride];
		    tmp53 = Y[-8 * iostride];
		    tmp54 = tmp52 + tmp53;
		    tmp96 = tmp51 - (K500000000 * tmp54);
		    tmp99 = K866025403 * (tmp52 - tmp53);
		    tmp67 = Y[-2 * iostride];
		    tmp68 = X[5 * iostride];
		    tmp69 = X[8 * iostride];
		    tmp70 = tmp68 + tmp69;
		    tmp97 = K866025403 * (tmp68 - tmp69);
		    tmp100 = tmp67 + (K500000000 * tmp70);
	       }
	       tmp50 = tmp46 + tmp49;
	       tmp55 = tmp51 + tmp54;
	       tmp56 = tmp50 + tmp55;
	       {
		    fftw_real tmp98;
		    fftw_real tmp101;
		    fftw_real tmp130;
		    fftw_real tmp131;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp98 = tmp96 + tmp97;
		    tmp101 = tmp99 + tmp100;
		    tmp102 = (K173648177 * tmp98) - (K984807753 * tmp101);
		    tmp110 = (K984807753 * tmp98) + (K173648177 * tmp101);
		    tmp130 = tmp96 - tmp97;
		    tmp131 = tmp100 - tmp99;
		    tmp132 = (K342020143 * tmp130) - (K939692620 * tmp131);
		    tmp138 = (K939692620 * tmp130) + (K342020143 * tmp131);
	       }
	       {
		    fftw_real tmp127;
		    fftw_real tmp128;
		    fftw_real tmp91;
		    fftw_real tmp94;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp127 = tmp93 - tmp92;
		    tmp128 = tmp89 + tmp90;
		    tmp129 = (K173648177 * tmp127) + (K984807753 * tmp128);
		    tmp137 = (K173648177 * tmp128) - (K984807753 * tmp127);
		    tmp91 = tmp89 - tmp90;
		    tmp94 = tmp92 + tmp93;
		    tmp95 = (K766044443 * tmp91) - (K642787609 * tmp94);
		    tmp109 = (K766044443 * tmp94) + (K642787609 * tmp91);
		    tmp66 = tmp62 + tmp65;
		    tmp71 = tmp67 - tmp70;
		    tmp72 = tmp66 + tmp71;
	       }
	  }
	  X[0] = tmp45 + tmp56;
	  {
	       fftw_real tmp76;
	       fftw_real tmp82;
	       fftw_real tmp80;
	       fftw_real tmp84;
	       ASSERT_ALIGNED_DOUBLE();
	       {
		    fftw_real tmp74;
		    fftw_real tmp75;
		    fftw_real tmp78;
		    fftw_real tmp79;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp74 = tmp61 - (K500000000 * tmp72);
		    tmp75 = K866025403 * (tmp50 - tmp55);
		    tmp76 = tmp74 - tmp75;
		    tmp82 = tmp75 + tmp74;
		    tmp78 = tmp45 - (K500000000 * tmp56);
		    tmp79 = K866025403 * (tmp71 - tmp66);
		    tmp80 = tmp78 - tmp79;
		    tmp84 = tmp78 + tmp79;
	       }
	       {
		    fftw_real tmp73;
		    fftw_real tmp77;
		    fftw_real tmp81;
		    fftw_real tmp83;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp73 = c_re(W[5]);
		    tmp77 = c_im(W[5]);
		    Y[-2 * iostride] = (tmp73 * tmp76) - (tmp77 * tmp80);
		    X[6 * iostride] = (tmp77 * tmp76) + (tmp73 * tmp80);
		    tmp81 = c_re(W[2]);
		    tmp83 = c_im(W[2]);
		    Y[-5 * iostride] = (tmp81 * tmp82) - (tmp83 * tmp84);
		    X[3 * iostride] = (tmp83 * tmp82) + (tmp81 * tmp84);
	       }
	  }
	  Y[-8 * iostride] = tmp61 + tmp72;
	  {
	       fftw_real tmp115;
	       fftw_real tmp104;
	       fftw_real tmp118;
	       fftw_real tmp119;
	       fftw_real tmp112;
	       fftw_real tmp114;
	       fftw_real tmp103;
	       fftw_real tmp111;
	       fftw_real tmp85;
	       fftw_real tmp105;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp115 = K866025403 * (tmp95 - tmp102);
	       tmp103 = tmp95 + tmp102;
	       tmp104 = tmp88 + tmp103;
	       tmp118 = tmp88 - (K500000000 * tmp103);
	       tmp119 = K866025403 * (tmp110 - tmp109);
	       tmp111 = tmp109 + tmp110;
	       tmp112 = tmp108 + tmp111;
	       tmp114 = tmp108 - (K500000000 * tmp111);
	       tmp85 = c_re(W[0]);
	       tmp105 = c_im(W[0]);
	       X[iostride] = (tmp85 * tmp104) + (tmp105 * tmp112);
	       Y[-7 * iostride] = (tmp85 * tmp112) - (tmp105 * tmp104);
	       {
		    fftw_real tmp122;
		    fftw_real tmp124;
		    fftw_real tmp121;
		    fftw_real tmp123;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp122 = tmp115 + tmp114;
		    tmp124 = tmp118 + tmp119;
		    tmp121 = c_re(W[3]);
		    tmp123 = c_im(W[3]);
		    Y[-4 * iostride] = (tmp121 * tmp122) - (tmp123 * tmp124);
		    X[4 * iostride] = (tmp123 * tmp122) + (tmp121 * tmp124);
	       }
	       {
		    fftw_real tmp116;
		    fftw_real tmp120;
		    fftw_real tmp113;
		    fftw_real tmp117;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp116 = tmp114 - tmp115;
		    tmp120 = tmp118 - tmp119;
		    tmp113 = c_re(W[6]);
		    tmp117 = c_im(W[6]);
		    Y[-iostride] = (tmp113 * tmp116) - (tmp117 * tmp120);
		    X[7 * iostride] = (tmp117 * tmp116) + (tmp113 * tmp120);
	       }
	  }
	  {
	       fftw_real tmp143;
	       fftw_real tmp134;
	       fftw_real tmp146;
	       fftw_real tmp147;
	       fftw_real tmp140;
	       fftw_real tmp142;
	       fftw_real tmp133;
	       fftw_real tmp139;
	       fftw_real tmp125;
	       fftw_real tmp135;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp143 = K866025403 * (tmp132 - tmp129);
	       tmp133 = tmp129 + tmp132;
	       tmp134 = tmp126 + tmp133;
	       tmp146 = tmp126 - (K500000000 * tmp133);
	       tmp147 = K866025403 * (tmp137 + tmp138);
	       tmp139 = tmp137 - tmp138;
	       tmp140 = tmp136 + tmp139;
	       tmp142 = tmp136 - (K500000000 * tmp139);
	       tmp125 = c_re(W[1]);
	       tmp135 = c_im(W[1]);
	       Y[-6 * iostride] = (tmp125 * tmp134) - (tmp135 * tmp140);
	       X[2 * iostride] = (tmp135 * tmp134) + (tmp125 * tmp140);
	       {
		    fftw_real tmp150;
		    fftw_real tmp152;
		    fftw_real tmp149;
		    fftw_real tmp151;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp150 = tmp143 + tmp142;
		    tmp152 = tmp146 + tmp147;
		    tmp149 = c_re(W[4]);
		    tmp151 = c_im(W[4]);
		    X[5 * iostride] = (tmp149 * tmp150) + (tmp151 * tmp152);
		    Y[-3 * iostride] = (tmp149 * tmp152) - (tmp151 * tmp150);
	       }
	       {
		    fftw_real tmp144;
		    fftw_real tmp148;
		    fftw_real tmp141;
		    fftw_real tmp145;
		    ASSERT_ALIGNED_DOUBLE();
		    tmp144 = tmp142 - tmp143;
		    tmp148 = tmp146 - tmp147;
		    tmp141 = c_re(W[7]);
		    tmp145 = c_im(W[7]);
		    X[8 * iostride] = (tmp141 * tmp144) + (tmp145 * tmp148);
		    Y[0] = (tmp141 * tmp148) - (tmp145 * tmp144);
	       }
	  }
     }
     if (i == m) {
	  fftw_real tmp3;
	  fftw_real tmp33;
	  fftw_real tmp17;
	  fftw_real tmp12;
	  fftw_real tmp11;
	  fftw_real tmp20;
	  fftw_real tmp34;
	  fftw_real tmp7;
	  fftw_real tmp36;
	  fftw_real tmp35;
	  fftw_real tmp23;
	  fftw_real tmp29;
	  fftw_real tmp26;
	  fftw_real tmp37;
	  fftw_real tmp8;
	  fftw_real tmp13;
	  ASSERT_ALIGNED_DOUBLE();
	  {
	       fftw_real tmp16;
	       fftw_real tmp1;
	       fftw_real tmp2;
	       fftw_real tmp14;
	       fftw_real tmp15;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp15 = Y[-iostride];
	       tmp16 = K1_732050807 * tmp15;
	       tmp1 = X[4 * iostride];
	       tmp2 = X[iostride];
	       tmp14 = tmp2 - tmp1;
	       tmp3 = tmp1 + (K2_000000000 * tmp2);
	       tmp33 = tmp14 - tmp16;
	       tmp17 = tmp14 + tmp16;
	  }
	  {
	       fftw_real tmp4;
	       fftw_real tmp6;
	       fftw_real tmp5;
	       fftw_real tmp24;
	       fftw_real tmp22;
	       fftw_real tmp27;
	       fftw_real tmp18;
	       fftw_real tmp10;
	       fftw_real tmp21;
	       fftw_real tmp19;
	       fftw_real tmp28;
	       fftw_real tmp9;
	       fftw_real tmp25;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp4 = X[3 * iostride];
	       tmp6 = X[2 * iostride];
	       tmp5 = X[0];
	       tmp24 = K866025403 * (tmp4 - tmp5);
	       tmp22 = (K500000000 * (tmp4 + tmp5)) - tmp6;
	       tmp27 = K866025403 * (tmp6 - tmp5);
	       tmp18 = (K500000000 * (tmp5 + tmp6)) - tmp4;
	       tmp9 = Y[0];
	       tmp12 = Y[-2 * iostride];
	       tmp10 = Y[-3 * iostride];
	       tmp11 = tmp9 + tmp10;
	       tmp21 = K866025403 * (tmp9 - tmp10);
	       tmp19 = K866025403 * (tmp9 + tmp12);
	       tmp28 = K500000000 * (tmp12 - tmp9);
	       tmp20 = tmp18 + tmp19;
	       tmp34 = tmp18 - tmp19;
	       tmp7 = tmp4 + tmp5 + tmp6;
	       tmp36 = tmp27 + tmp28 + tmp10;
	       tmp35 = tmp22 + tmp21;
	       tmp23 = tmp21 - tmp22;
	       tmp29 = tmp27 - (tmp28 + tmp10);
	       tmp25 = K500000000 * tmp11;
	       tmp26 = tmp24 - tmp25 - tmp12;
	       tmp37 = (tmp24 + tmp12) + tmp25;
	  }
	  X[0] = tmp3 + (K2_000000000 * tmp7);
	  tmp8 = tmp7 - tmp3;
	  tmp13 = K1_732050807 * (tmp11 - tmp12);
	  X[3 * iostride] = tmp8 - tmp13;
	  X[6 * iostride] = -(tmp8 + tmp13);
	  {
	       fftw_real tmp32;
	       fftw_real tmp30;
	       fftw_real tmp31;
	       fftw_real tmp40;
	       fftw_real tmp38;
	       fftw_real tmp39;
	       ASSERT_ALIGNED_DOUBLE();
	       tmp32 = (K813797681 * tmp26) + (K150383733 * tmp29) - (K296198132 * tmp23) - (K852868531 * tmp20);
	       tmp30 = (K173648177 * tmp20) + (K939692620 * tmp23) + (K342020143 * tmp26) + (K984807753 * tmp29);
	       tmp31 = tmp17 - (K500000000 * tmp30);
	       X[2 * iostride] = -(tmp17 + tmp30);
	       X[8 * iostride] = tmp32 - tmp31;
	       X[5 * iostride] = tmp31 + tmp32;
	       tmp40 = (K556670399 * tmp34) + (K663413948 * tmp36) - (K150383733 * tmp37) - (K852868531 * tmp35);
	       tmp38 = (K766044443 * tmp34) + (K173648177 * tmp35) - (K642787609 * tmp36) - (K984807753 * tmp37);
	       tmp39 = (K500000000 * tmp38) - tmp33;
	       X[iostride] = tmp33 + tmp38;
	       X[7 * iostride] = tmp40 - tmp39;
	       X[4 * iostride] = tmp39 + tmp40;
	  }
     }
}

static const int twiddle_order[] =
{1, 2, 3, 4, 5, 6, 7, 8};
fftw_codelet_desc fftw_hc2hc_backward_9_desc =
{
     "fftw_hc2hc_backward_9",
     (void (*)()) fftw_hc2hc_backward_9,
     9,
     FFTW_BACKWARD,
     FFTW_HC2HC,
     212,
     8,
     twiddle_order,
};
