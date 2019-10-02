/* File name ......... : pcl833.c
 * Object ............ : Port per SHARK dei driver del encoder PCL833 della Advantech
 *                       scaricati per DOS al sito della Advantech www.advantech.com
 * Author ............ : Cerri Pietro, Fracassi Matteo
 * Language .......... : C
 * Operative system .. : SHARK
 * Creation data ..... : 26/06/2001
 * Last modify ....... : 29/06/2001
 */

#include <ll/ll.h>
#include "drivers/pcl833.h"

//Output Registers and Input Registers

int OutReg[16];
int InReg[16];
int Base;

//All fuctions are called through pcl833()

int pcl833(int func, int option);

//Functions

int vCh_SetInputMode(int ChannelNo, int option);
int vCh_SetInputMode(int ChannelNo, int option);
int vCh_SetInputMode(int ChannelNo, int option);
int vCh_DefineResetValue(int ChannelNo, int option);
int vCh_DefineResetValue(int ChannelNo, int option);
int vCh_DefineResetValue(int ChannelNo, int option);
int vCh_SetLatchSource(int ChannelNo, int option);
int vCh_SetLatchSource(int ChannelNo, int option);
int vCh_SetLatchSource(int ChannelNo, int option);
int vCh_IfResetOnLatch(int ChannelNo, int option);
int vCh_IfResetOnLatch(int ChannelNo, int option);
int vCh_IfResetOnLatch(int ChannelNo, int option);
int vLatchWhenOverflow(int option);
int vCounterReset(int option);
int vChooseSysClock(int option);
int vSetCascadeMode(int option);
int vSet16C54TimeBase(int option);
int vSetDI1orTimerInt(int option);
int vSet16C54Divider(int option);
int vCh_Read(int option);
int vStatus_Read(void);
int vOverflow_Read(void);
int vInitialize833(int option);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:                                                      :
//:   All fuctions are called  through  pcl833()         :
//:                                                      :
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::

int pcl833(int func, int option) {

    switch(func) {
        case Ch1_SetInputMode     : return(vCh_SetInputMode(ch1, option));
        case Ch2_SetInputMode     : return(vCh_SetInputMode(ch2, option));
        case Ch3_SetInputMode     : return(vCh_SetInputMode(ch3, option));
        case Ch1_DefineResetValue : return(vCh_DefineResetValue(ch1, option));
        case Ch2_DefineResetValue : return(vCh_DefineResetValue(ch2, option));
        case Ch3_DefineResetValue : return(vCh_DefineResetValue(ch3, option));
        case Ch1_SetLatchSource   : return(vCh_SetLatchSource(ch1, option));
        case Ch2_SetLatchSource   : return(vCh_SetLatchSource(ch2, option));
        case Ch3_SetLatchSource   : return(vCh_SetLatchSource(ch3, option));
        case Ch1_IfResetOnLatch   : return(vCh_IfResetOnLatch(ch1, option));
        case Ch2_IfResetOnLatch   : return(vCh_IfResetOnLatch(ch2, option));
        case Ch3_IfResetOnLatch   : return(vCh_IfResetOnLatch(ch3, option));
        case LatchWhenOverflow    : return(vLatchWhenOverflow(option));
        case CounterReset         : return(vCounterReset(option));
        case ChooseSysClock       : return(vChooseSysClock(option));
        case SetCascadeMode       : return(vSetCascadeMode(option));
        case Set16C54TimeBase     : return(vSet16C54TimeBase(option));
        case SetDI1orTimerInt     : return(vSetDI1orTimerInt(option));
        case Set16C54Divider      : return(vSet16C54Divider(option));
        case Ch_Read              : return(vCh_Read(option));
        case Status_Read          : return(vStatus_Read());
        case Overflow_Read        : return(vOverflow_Read());
        case Initialize833        : return(vInitialize833(option));
        default                   : return(FUNCTION_NUM_ERR);
    }
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vInitialize833(int option) {

    register int i;

    Base = option;

    for(i = 0; i < 16; i++)
        OutReg[i] = InReg[i] = 0;

    vCh_SetInputMode(ch1, PclDisable);
    vCh_SetInputMode(ch2, PclDisable);
    vCh_SetInputMode(ch3, PclDisable);
    vCh_DefineResetValue(ch1, start);
    vCh_DefineResetValue(ch2, start);
    vCh_DefineResetValue(ch3, start);
    vCh_SetLatchSource(ch1, SwReadLatch);
    vCh_SetLatchSource(ch2, SwReadLatch);
    vCh_SetLatchSource(ch3, SwReadLatch);
    vCh_IfResetOnLatch(ch1, ResetNo);
    vCh_IfResetOnLatch(ch2, ResetNo);
    vCh_IfResetOnLatch(ch3, ResetNo);
    vLatchWhenOverflow(Latch_Ch1);
    vLatchWhenOverflow(Latch_Ch2);
    vLatchWhenOverflow(Latch_Ch3);
    vCounterReset(Reset_Ch1);
    vCounterReset(Reset_Ch2);
    vCounterReset(Reset_Ch3);
    vChooseSysClock(Sys8MHZ);
    vSetCascadeMode(c24bits);  //no cascade
    vSet16C54TimeBase(tPoint1ms);
    vSetDI1orTimerInt(TimerInt);
    vSet16C54Divider(0);     // 0-255 divider

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vCh_SetInputMode(int ChannelNo, int option) {

    int OutputReg, PortAddress, RegIndex;

    switch(ChannelNo) {
        case ch1:  PortAddress = Base + 0;  RegIndex = 0;   break;
        case ch2:  PortAddress = Base + 1;  RegIndex = 1;   break;
        case ch3:  PortAddress = Base + 2;  RegIndex = 2;   break;
        default:   return(CHANNEL_NUM_ERR);
    }

    OutputReg = OutReg[RegIndex] & 0x08;

    switch(option) {
        case x1:
        case x2:
        case x4:
        case PclDisable:
        case TwoPulseIn:
        case OnePulseIn: outp(PortAddress, OutputReg | option);  break;
        default:         return(PARAMETER_ERR);
    }

    OutReg[RegIndex] = OutputReg | option;

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vCh_DefineResetValue(int ChannelNo, int option) {

    int OutputReg, PortAddress, RegIndex;

    switch(ChannelNo) {
        case ch1:  PortAddress = Base + 0;  RegIndex = 0;   break;
        case ch2:  PortAddress = Base + 1;  RegIndex = 1;   break;
        case ch3:  PortAddress = Base + 2;  RegIndex = 2;   break;
        default:   return(CHANNEL_NUM_ERR);
    }

    OutputReg = OutReg[RegIndex] & 0x07;

    switch(option) {
        case start:
        case middle:   outp(PortAddress, OutputReg | option);   break;
        default:       return(PARAMETER_ERR);
    }

    OutReg[RegIndex] = OutputReg | option;

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vCh_SetLatchSource(int ChannelNo, int option) {

    int OutputReg, PortAddress, RegIndex;

    switch(ChannelNo) {
        case ch1:  PortAddress = Base + 3;  RegIndex = 3;   break;
        case ch2:  PortAddress = Base + 4;  RegIndex = 4;   break;
        case ch3:  PortAddress = Base + 5;  RegIndex = 5;   break;
        default:   return(CHANNEL_NUM_ERR);
    }

    OutputReg = OutReg[RegIndex] & 0x08;

    switch(option) {
        case SwReadLatch:
        case IndexInLatch:
        case DI0Latch:
        case DI1Latch:
        case TimerLatch:    outp(PortAddress, OutputReg | option);   break;
        default:            return(PARAMETER_ERR);
    }

    OutReg[RegIndex] = OutputReg | option;

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vCh_IfResetOnLatch(int ChannelNo, int option) {

    int OutputReg, PortAddress, RegIndex;

    switch(ChannelNo) {
        case ch1:  PortAddress= Base + 3;  RegIndex = 3;   break;
        case ch2:  PortAddress= Base + 4;  RegIndex = 4;   break;
        case ch3:  PortAddress= Base + 5;  RegIndex = 5;   break;
        default:   return(CHANNEL_NUM_ERR);
    }

    OutputReg = OutReg[RegIndex] & 0x07;

    switch(option) {
        case ResetNo:
        case ResetYes:   outp(PortAddress, OutputReg | option);   break;
        default:         return(PARAMETER_ERR);
    }

    OutReg[RegIndex] = OutputReg | option;

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vLatchWhenOverflow(int option) {

    switch(option){
        case  Latch_Ch1:   OutReg[6] &= 0x06;   break;
        case  Latch_Ch2:   OutReg[6] &= 0x05;   break;
        case  Latch_Ch3:   OutReg[6] &= 0x03;   break;
        case  FreeAll:     OutReg[6] = 0x07;    break;
        default:           return(PARAMETER_ERR);
    }

    outp(Base + 6, OutReg[6]);

    return(OK);
}


//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vCounterReset(int option) {

    switch(option) {
        case Reset_Ch1:   OutReg[7] &= 0x06;
                          OutReg[7] |= 0x01;
                          break;
        case Reset_Ch2:   OutReg[7] &= 0x05;
                          OutReg[7] |= 0x02;
                          break;
        case Reset_Ch3:   OutReg[7] &= 0x03;
                          OutReg[7] |= 0x04;
                          break;
        case NoneReset:   OutReg[7] = 0;
                          break;
        default:          return(PARAMETER_ERR);
    }

    outp(Base + 7, OutReg[7]);

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vChooseSysClock(option) {

    int OutputReg;

    OutputReg = OutReg[8] & 0x0c;

    switch(option) {
        case Sys8MHZ:
        case Sys4MHZ:
        case Sys2MHZ:  outp(Base + 8, OutputReg | option);   break;
        default:       return(PARAMETER_ERR);
    }

    OutReg[8] = OutputReg | option;

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int  vSetCascadeMode(option) {

    int OutputReg;

    OutputReg = OutReg[8] & 0x03;

    switch(option) {
        case c24bits:   // no cascade
            outp(Base + 8, OutputReg | option);
            break;

        case c48bits:   // ch1 ch2 cascade
            OutReg[1] |= 0x07; // set ch2 cascade mode
            OutReg[0] &= 0x07; // set ch1 reset value '000000'
            outp(Base + 1, OutReg[1]);
            outp(Base, OutReg[0]);
            outp(Base + 8, OutputReg | option);
            vCh_SetInputMode(ch2, cascade);
            break;

        default:
            return(PARAMETER_ERR);
    }

    OutReg[8] = OutputReg | option;

    return(OK);
}


//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vSet16C54TimeBase(option) {

    int OutputReg;

    OutputReg = OutReg[9] & 0x08;

    switch(option) {
        case tPoint1ms:
        case t1ms:
        case t10ms:
        case t100ms:
        case t1s:       outp(Base + 9, OutputReg | option);   break;
        default:        return(PARAMETER_ERR);
    }

    OutReg[9] = OutputReg | option;

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vSetDI1orTimerInt(option) {

    int OutputReg;

    OutputReg = OutReg[9] & 0x07;
    switch(option) {
        case  DI1Int   :
        case  TimerInt : outp(Base + 9, OutputReg | option);   break;
        default:         return(PARAMETER_ERR);
    }

    OutReg[9] = OutputReg | option;

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vSet16C54Divider(option) {

    outp(Base + 10, option);
    OutReg[10] = option;

    return(OK);
}


//:::::::::::::::::::::::::::::::::::::::::::::::::
//:  READ FUNCTION
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vCh_Read(int ChannelNo) {

    switch(ChannelNo) {

        case ch1:
            InReg[2] = inp(Base + 2);
            InReg[0] = inp(Base);
            InReg[1] = inp(Base + 1);
            break;

        case ch2:
            InReg[6] = inp(Base + 6);
            InReg[4] = inp(Base + 4);
            InReg[5] = inp(Base + 5);
            break;

        case ch3:
            InReg[10] = inp(Base + 10);
            InReg[8] = inp(Base + 8);
            InReg[9] = inp(Base + 9);
            break;

        default:
            return(CHANNEL_NUM_ERR);
    }

    return(OK);
}


//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vOverflow_Read(void) {

    InReg[3] = inp(Base + 3);
    InReg[7] = inp(Base + 7);
    InReg[11] = inp(Base + 11);

    return(OK);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::
//:
//:::::::::::::::::::::::::::::::::::::::::::::::::

int vStatus_Read(void) {

    InReg[14] = inp(Base + 14);

    return(OK);
}
