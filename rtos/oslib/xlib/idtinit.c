
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

#include <ll/i386/hw-data.h>
#include <ll/i386/hw-func.h>

/* ll hardware interrupt hooks  */
extern void h0(void);
extern void h1(void);
extern void h2(void);
extern void h3(void);
extern void h4(void);
extern void h5(void);
extern void h6(void);
extern void exc7(void);
extern void h8(void);
extern void h9(void);
extern void h10(void);
extern void h11(void);
extern void h12(void);
extern void h13(void);
extern void h14(void);
extern void h15(void);
extern void h16(void);
extern void h17(void);
extern void h18(void);
extern void h19(void);
extern void h20(void);
extern void h21(void);
extern void h22(void);
extern void h23(void);
extern void h24(void);
extern void h25(void);
extern void h26(void);
extern void h27(void);
extern void h28(void);
extern void h29(void);
extern void h30(void);
extern void h31(void);
extern void h32(void);
extern void h33(void);
extern void h34(void);
extern void h35(void);
extern void h36(void);
extern void h37(void);
extern void h38(void);
extern void h39(void);
extern void h40(void);
extern void h41(void);
extern void h42(void);
extern void h43(void);
extern void h44(void);
extern void h45(void);
extern void h46(void);
extern void h47(void);
extern void h48(void);
extern void h49(void);
extern void h50(void);
extern void h51(void);
extern void h52(void);
extern void h53(void);
extern void h54(void);
extern void h55(void);
extern void h56(void);
extern void h57(void);
extern void h58(void);
extern void h59(void);
extern void h60(void);
extern void h61(void);
extern void h62(void);
extern void h63(void);
extern void h64(void);
extern void h65(void);
extern void h66(void);
extern void h67(void);
extern void h68(void);
extern void h69(void);
extern void h70(void);
extern void h71(void);
extern void h72(void);
extern void h73(void);
extern void h74(void);
extern void h75(void);
extern void h76(void);
extern void h77(void);
extern void h78(void);
extern void h79(void);
extern void h80(void);
extern void h81(void);
extern void h82(void);
extern void h83(void);
extern void h84(void);
extern void h85(void);
extern void h86(void);
extern void h87(void);
extern void h88(void);
extern void h89(void);
extern void h90(void);
extern void h91(void);
extern void h92(void);
extern void h93(void);
extern void h94(void);
extern void h95(void);
extern void h96(void);
extern void h97(void);
extern void h98(void);
extern void h99(void);
extern void h100(void);
extern void h101(void);
extern void h102(void);
extern void h103(void);
extern void h104(void);
extern void h105(void);
extern void h106(void);
extern void h107(void);
extern void h108(void);
extern void h109(void);
extern void h110(void);
extern void h111(void);
extern void h112(void);
extern void h113(void);
extern void h114(void);
extern void h115(void);
extern void h116(void);
extern void h117(void);
extern void h118(void);
extern void h119(void);
extern void h120(void);
extern void h121(void);
extern void h122(void);
extern void h123(void);
extern void h124(void);
extern void h125(void);
extern void h126(void);
extern void h127(void);
extern void h128(void);
extern void h129(void);
extern void h130(void);
extern void h131(void);
extern void h132(void);
extern void h133(void);
extern void h134(void);
extern void h135(void);
extern void h136(void);
extern void h137(void);
extern void h138(void);
extern void h139(void);
extern void h140(void);
extern void h141(void);
extern void h142(void);
extern void h143(void);
extern void h144(void);
extern void h145(void);
extern void h146(void);
extern void h147(void);
extern void h148(void);
extern void h149(void);
extern void h150(void);
extern void h151(void);
extern void h152(void);
extern void h153(void);
extern void h154(void);
extern void h155(void);
extern void h156(void);
extern void h157(void);
extern void h158(void);
extern void h159(void);
extern void h160(void);
extern void h161(void);
extern void h162(void);
extern void h163(void);
extern void h164(void);
extern void h165(void);
extern void h166(void);
extern void h167(void);
extern void h168(void);
extern void h169(void);
extern void h170(void);
extern void h171(void);
extern void h172(void);
extern void h173(void);
extern void h174(void);
extern void h175(void);
extern void h176(void);
extern void h177(void);
extern void h178(void);
extern void h179(void);
extern void h180(void);
extern void h181(void);
extern void h182(void);
extern void h183(void);
extern void h184(void);
extern void h185(void);
extern void h186(void);
extern void h187(void);
extern void h188(void);
extern void h189(void);
extern void h190(void);
extern void h191(void);
extern void h192(void);
extern void h193(void);
extern void h194(void);
extern void h195(void);
extern void h196(void);
extern void h197(void);
extern void h198(void);
extern void h199(void);
extern void h200(void);
extern void h201(void);
extern void h202(void);
extern void h203(void);
extern void h204(void);
extern void h205(void);
extern void h206(void);
extern void h207(void);
extern void h208(void);
extern void h209(void);
extern void h210(void);
extern void h211(void);
extern void h212(void);
extern void h213(void);
extern void h214(void);
extern void h215(void);
extern void h216(void);
extern void h217(void);
extern void h218(void);
extern void h219(void);
extern void h220(void);
extern void h221(void);
extern void h222(void);
extern void h223(void);
extern void h224(void);
extern void h225(void);
extern void h226(void);
extern void h227(void);
extern void h228(void);
extern void h229(void);
extern void h230(void);
extern void h231(void);
extern void h232(void);
extern void h233(void);
extern void h234(void);
extern void h235(void);
extern void h236(void);
extern void h237(void);
extern void h238(void);
extern void h239(void);
extern void h240(void);
extern void h241(void);
extern void h242(void);
extern void h243(void);
extern void h244(void);
extern void h245(void);
extern void h246(void);
extern void h247(void);
extern void h248(void);
extern void h249(void);
extern void h250(void);
extern void h251(void);
extern void h252(void);
extern void h253(void);
extern void h254(void);
extern void h255(void);

void IDT_init(void)
{
  /* Insert the Exceptions handler into IDT */
  IDT_place(0x00, h0);
  IDT_place(0x01, h1);
  IDT_place(0x02, h2);
  IDT_place(0x03, h3);
  IDT_place(0x04, h4);
  IDT_place(0x05, h5);
  IDT_place(0x06, h6);
  IDT_place(0x07, exc7);
  IDT_place(0x08, h8);
  IDT_place(0x09, h9);
  IDT_place(0x0A, h10);
  IDT_place(0x0B, h11);
  IDT_place(0x0C, h12);
  IDT_place(0x0D, h13);
  IDT_place(0x0E, h14);
  IDT_place(0x0F, h15);
  IDT_place(0x10, h16);
  IDT_place(0x11, h17);
  IDT_place(0x12, h18);
  IDT_place(0x13, h19);
  IDT_place(0x14, h20);
  IDT_place(0x15, h21);
  IDT_place(0x16, h22);
  IDT_place(0x17, h23);
  IDT_place(0x18, h24);
  IDT_place(0x19, h25);
  IDT_place(0x1A, h26);
  IDT_place(0x1B, h27);
  IDT_place(0x1C, h28);
  IDT_place(0x1D, h29);
  IDT_place(0x1E, h30);
  IDT_place(0x1F, h31);
  
  IDT_place(0x20, h32);
  IDT_place(0x21, h33);
  IDT_place(0x22, h34);
  IDT_place(0x23, h35);
  IDT_place(0x24, h36);
  IDT_place(0x25, h37);
  IDT_place(0x26, h38);
  IDT_place(0x27, h39);
  IDT_place(0x28, h40);
  IDT_place(0x29, h41);
  IDT_place(0x2A, h42);
  IDT_place(0x2B, h43);
  IDT_place(0x2C, h44);
  IDT_place(0x2D, h45);
  IDT_place(0x2E, h46);
  IDT_place(0x2F, h47);
  IDT_place(0x30, h48);
  IDT_place(0x31, h49);
  IDT_place(0x32, h50);
  IDT_place(0x33, h51);
  IDT_place(0x34, h52);
  IDT_place(0x35, h53);
  IDT_place(0x36, h54);
  IDT_place(0x37, h55);
  IDT_place(0x38, h56);
  IDT_place(0x39, h57);
  IDT_place(0x3A, h58);
  IDT_place(0x3B, h59);
  IDT_place(0x3C, h60);
  IDT_place(0x3D, h61);
  IDT_place(0x3E, h62);
  IDT_place(0x3F, h63);
 
  IDT_place(0x40, h64);
  IDT_place(0x41, h65);
  IDT_place(0x42, h66);
  IDT_place(0x43, h67);
  IDT_place(0x44, h68);
  IDT_place(0x45, h69);
  IDT_place(0x46, h70);
  IDT_place(0x47, h71);

  IDT_place(0x48, h72);
  IDT_place(0x49, h73);
  IDT_place(0x4A, h74);
  IDT_place(0x4B, h75);
  IDT_place(0x4C, h76);
  IDT_place(0x4D, h77);
  IDT_place(0x4E, h78);
  IDT_place(0x4F, h79);
  IDT_place(0x50, h80);
  IDT_place(0x51, h81);
  IDT_place(0x52, h82);
  IDT_place(0x53, h83);
  IDT_place(0x54, h84);
  IDT_place(0x55, h85);
  IDT_place(0x56, h86);
  IDT_place(0x57, h87);
  IDT_place(0x58, h88);
  IDT_place(0x59, h89);
  IDT_place(0x5A, h90);
  IDT_place(0x5B, h91);
  IDT_place(0x5C, h92);
  IDT_place(0x5D, h93);
  IDT_place(0x5E, h94);
  IDT_place(0x5F, h95);
  IDT_place(0x60, h96);
  IDT_place(0x61, h97);
  IDT_place(0x62, h98);
  IDT_place(0x63, h99);
  IDT_place(0x64, h100);
  IDT_place(0x65, h101);
  IDT_place(0x66, h102);
  IDT_place(0x67, h103);
  IDT_place(0x68, h104);
  IDT_place(0x69, h105);
  IDT_place(0x6A, h106);
  IDT_place(0x6B, h107);
  IDT_place(0x6C, h108);
  IDT_place(0x6D, h109);
  IDT_place(0x6E, h110);
  IDT_place(0x6F, h111);

  IDT_place(0x70, h112);
  IDT_place(0x71, h113);
  IDT_place(0x72, h114);
  IDT_place(0x73, h115);
  IDT_place(0x74, h116);
  IDT_place(0x75, h117);
  IDT_place(0x76, h118);
  IDT_place(0x77, h119);
  
  IDT_place(0x78, h120);
  IDT_place(0x79, h121);
  IDT_place(0x7A, h122);
  IDT_place(0x7B, h123);
  IDT_place(0x7C, h124);
  IDT_place(0x7D, h125);
  IDT_place(0x7E, h127);
  IDT_place(0x7F, h127);
  IDT_place(0x80, h128);
  IDT_place(0x81, h129);
  IDT_place(0x82, h130);
  IDT_place(0x83, h131);
  IDT_place(0x84, h132);
  IDT_place(0x85, h133);
  IDT_place(0x86, h134);
  IDT_place(0x87, h135);
  IDT_place(0x88, h136);
  IDT_place(0x89, h137);
  IDT_place(0x8A, h138);
  IDT_place(0x8B, h139);
  IDT_place(0x8C, h140);
  IDT_place(0x8D, h141);
  IDT_place(0x8E, h142);
  IDT_place(0x8F, h143);
  IDT_place(0x90, h144);
  IDT_place(0x91, h145);
  IDT_place(0x92, h146);
  IDT_place(0x93, h147);
  IDT_place(0x94, h148);
  IDT_place(0x95, h149);
  IDT_place(0x96, h150);
  IDT_place(0x97, h151);
  IDT_place(0x98, h152);
  IDT_place(0x99, h153);
  IDT_place(0x9A, h154);
  IDT_place(0x9B, h155);
  IDT_place(0x9C, h156);
  IDT_place(0x9D, h157);
  IDT_place(0x9E, h158);
  IDT_place(0x9F, h159);
  IDT_place(0xA0, h160);
  IDT_place(0xA1, h161);
  IDT_place(0xA2, h162);
  IDT_place(0xA3, h163);
  IDT_place(0xA4, h164);
  IDT_place(0xA5, h165);
  IDT_place(0xA6, h166);
  IDT_place(0xA7, h167);
  IDT_place(0xA8, h168);
  IDT_place(0xA9, h169);
  IDT_place(0xAA, h170);
  IDT_place(0xAB, h171);
  IDT_place(0xAC, h172);
  IDT_place(0xAD, h173);
  IDT_place(0xAE, h174);
  IDT_place(0xAF, h175);
  IDT_place(0xB0, h176);
  IDT_place(0xB1, h177);
  IDT_place(0xB2, h178);
  IDT_place(0xB3, h179);
  IDT_place(0xB4, h180);
  IDT_place(0xB5, h181);
  IDT_place(0xB6, h182);
  IDT_place(0xB7, h183);
  IDT_place(0xB8, h184);
  IDT_place(0xB9, h185);
  IDT_place(0xBA, h186);
  IDT_place(0xBB, h187);
  IDT_place(0xBC, h188);
  IDT_place(0xBD, h189);
  IDT_place(0xBE, h190);
  IDT_place(0xBF, h191);
  IDT_place(0xC0, h192);
  IDT_place(0xC1, h193);
  IDT_place(0xC2, h194);
  IDT_place(0xC3, h195);
  IDT_place(0xC4, h196);
  IDT_place(0xC5, h197);
  IDT_place(0xC6, h198);
  IDT_place(0xC7, h199);
  IDT_place(0xC8, h200);
  IDT_place(0xC9, h201);
  IDT_place(0xCA, h202);
  IDT_place(0xCB, h203);
  IDT_place(0xCC, h204);
  IDT_place(0xCD, h205);
  IDT_place(0xCE, h206);
  IDT_place(0xCF, h207);
  IDT_place(0xD0, h208);
  IDT_place(0xD1, h209);
  IDT_place(0xD2, h210);
  IDT_place(0xD3, h211);
  IDT_place(0xD4, h212);
  IDT_place(0xD5, h213);
  IDT_place(0xD6, h214);
  IDT_place(0xD7, h215);
  IDT_place(0xD8, h216);
  IDT_place(0xD9, h217);
  IDT_place(0xDA, h218);
  IDT_place(0xDB, h219);
  IDT_place(0xDC, h220);
  IDT_place(0xDD, h221);
  IDT_place(0xDE, h222);
  IDT_place(0xDF, h223);
  IDT_place(0xE0, h224);
  IDT_place(0xE1, h225);
  IDT_place(0xE2, h226);
  IDT_place(0xE3, h227);
  IDT_place(0xE4, h228);
  IDT_place(0xE5, h229);
  IDT_place(0xE6, h230);
  IDT_place(0xE7, h231);
  IDT_place(0xE8, h232);
  IDT_place(0xE9, h233);
  IDT_place(0xEA, h234);
  IDT_place(0xEB, h235);
  IDT_place(0xEC, h236);
  IDT_place(0xED, h237);
  IDT_place(0xEE, h238);
  IDT_place(0xEF, h239);
  IDT_place(0xF0, h240);
  IDT_place(0xF1, h241);
  IDT_place(0xF2, h242);
  IDT_place(0xF3, h243);
  IDT_place(0xF4, h244);
  IDT_place(0xF5, h245);
  IDT_place(0xF6, h246);
  IDT_place(0xF7, h247);
  IDT_place(0xF8, h248);
  IDT_place(0xF9, h249);
  IDT_place(0xFA, h250);
  IDT_place(0xFB, h251);
  IDT_place(0xFC, h252);
  IDT_place(0xFD, h253);
  IDT_place(0xFE, h254);
  IDT_place(0xFF, h255);
} 
