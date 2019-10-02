/* File name ......... : 833drive.h
 * Object ............ : driver del frame grabber PCL833 della Advantech
 *                       scaricati al sito della Advantech www.advantech.com
 * Language .......... : C
 * Operative system .. : DOS/SHARK
 */

#ifndef __833DRIVE_H
#define __833DRIVE_H

// Define Error Code

#define ERR              -6
#define PARAMETER_ERR    -5
#define COUNT_LATCH_ERR  -4
#define COUNT_MODE_ERR   -3
#define CHANNEL_NUM_ERR  -2
#define FUNCTION_NUM_ERR -1
#define FUNCTION_SUCCESS  0
#define OK                0

// Define Basic Function Number

#define  Ch1_SetInputMode      1000
#define  Ch2_SetInputMode      1001
#define  Ch3_SetInputMode      1002
#define  Ch1_DefineResetValue  1003
#define  Ch2_DefineResetValue  1004
#define  Ch3_DefineResetValue  1005
#define  Ch1_SetLatchSource    1006
#define  Ch2_SetLatchSource    1007
#define  Ch3_SetLatchSource    1008
#define  Ch1_IfResetOnLatch    1009
#define  Ch2_IfResetOnLatch    1010
#define  Ch3_IfResetOnLatch    1011
#define  LatchWhenOverflow     1012
#define  CounterReset          1013
#define  ChooseSysClock        1014
#define  SetCascadeMode        1015
#define  Set16C54TimeBase      1016
#define  SetDI1orTimerInt      1017
#define  Set16C54Divider       1018
#define  Ch_Read               1019
#define  Overflow_Read         1020
#define  Status_Read           1021
#define  Latch_Ch1             1022
#define  Latch_Ch2             1023
#define  Latch_Ch3             1024
#define  FreeAll               1025
#define  NoneReset             1026
#define  Reset_Ch1             1027
#define  Reset_Ch2             1028
#define  Reset_Ch3             1029

// Define Miscellaneous Function Number

#define  Initialize833         2000

// Define Values

#define  ch1              1
#define  ch2              2
#define  ch3              3
#define  NA               4

// vCh_SetInputMode options
#define  PclDisable       0
#define  x1               1
#define  x2               2
#define  x4               3
#define  TwoPulseIn       4
#define  OnePulseIn       5
// vCh_DefineResetValue options
#define  start            0
#define  middle           8
// vCh_SetLatchSource options
#define  SwReadLatch      0
#define  IndexInLatch     1
#define  DI0Latch         2
#define  DI1Latch         3
#define  TimerLatch       4
// vCh_IfResetOnLatch options
#define  ResetNo          0
#define  ResetYes      0x08
// vChooseSysClock options
#define  Sys8MHZ          0
#define  Sys4MHZ          1
#define  Sys2MHZ          2
// vSetCascadeMode options
#define  c24bits          0
#define  c48bits          4
#define  c72bits          8
#define  cascade          7
// vSet16C54TimeBase options
#define  tPoint1ms        0
#define  t1ms             1
#define  t10ms            2
#define  t100ms           3
#define  t1s              4
// vSetDI1orTimerInt options
#define  DI1Int           0
#define  TimerInt         8

//Output Registers and Input Registers

extern int OutReg[16];
extern int InReg[16];

//All fuctions are called through pcl833()

int pcl833(int func, int option);

#endif
