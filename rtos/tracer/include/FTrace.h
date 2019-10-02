/*

  Events List File, from York University and F.I.R.S.T project

*/


#ifndef __FTrace__
#define __FTrace__


// List of events, these are 8 bit constants, the 
// the low 4 bits, indicates its class, 
// the high 4 bits indicate the type of event in the class.
// (This allows an easy mechanisms for filtering events)

/* Tool : used for the filter masking*/
#define FTrace_family_mask   0x0F

// general trace events
#define FTrace_EVT_empty	      0x00
#define FTrace_EVT_cycles_per_msec    0x10 // Par1 [empty] Par2 [clk/msec]
#define FTrace_EVT_trace_start        0x20 // Par1 [empty] Par2 [empty]
#define FTrace_EVT_trace_stop         0x30 // Par1 [empty] Par2 [empty]
#define FTrace_EVT_blackout_start     0x40
#define FTrace_EVT_blackout_end       0x50
#define FTrace_EVT_id                 0x60 // Par1 [ctx] Par2 [pid]
#define FTrace_EVT_numeevents         0x70

// lightweight tracing events
#define FTrace_EVT_ipoint             0x01 // Par1 [number] Par2 [empty]

// Task related events
#define FTrace_EVT_task_create        0x02 // Par1 [ctx] Par2 [pid]
#define FTrace_EVT_task_activate      0x12 // Par1 [ctx] Par2 [empty]
#define FTrace_EVT_task_dispatch      0x22
#define FTrace_EVT_task_epilogue      0x32
#define FTrace_EVT_task_end           0x42 // Par1 [ctx] Par2 [pid]
#define FTrace_EVT_task_begin_cycle   0x52
#define FTrace_EVT_task_end_cycle     0x62 // Par1 [ctx] Par2 [level]
#define FTrace_EVT_task_sleep         0x72
#define FTrace_EVT_task_schedule      0x82 // Par1 [exec_shadow.ctx] Par2 [exec.ctx]
#define FTrace_EVT_task_timer         0x92 // Par1 [ctx] Par2 [level]
#define FTrace_EVT_task_disable       0xA2 
#define FTrace_EVT_task_deadline_miss 0xB2 // Par1 [ctx] Par2 [empty]
#define FTrace_EVT_task_wcet_violation 0xC2 // Par1 [ctx] Par2 [empty]

// interrupt events, even more lightweight than ipoints
#define FTrace_EVT_interrupt_start    0x03 // Par1 [int] Par2 [empty]
#define FTrace_EVT_interrupt_end      0x13 // Par1 [int] Par2 [empty]
#define FTrace_EVT_interrupt_hit      0x23 // instant where interrupt was hit (no end)
#define FTrace_EVT_interrupt_count    0x33 // number of interrupts raised since last interrupt_count

// other CPU specific events
#define FTrace_EVT_to_real_mode       0x04
#define FTrace_EVT_to_protected_mode  0x14
#define FTrace_EVT_CLI                0x24
#define FTrace_EVT_STI                0x34

// Changes on task attributes and state
#define FTrace_EVT_set_priority       0x05
#define FTrace_EVT_context_switch     0x15 // Par1 [ctx]
#define FTrace_EVT_inheritance        0x25 // Par1 [exec_shadow.ctx] Par2 [exec.ctx]

// mutex events
#define FTrace_EVT_set_mutex_create   0x06
#define FTrace_EVT_set_mutex_lock     0x16 // Par1 [ctx] Par2 [mutex]
#define FTrace_EVT_set_mutex_inherit  0x26
#define FTrace_EVT_set_mutex_unlock   0x36 // Par1 [ctx] Par2 [mutex]
#define FTrace_EVT_set_mutex_wait     0x46 // Par1 [ctx] Par2 [mutex]
#define FTrace_EVT_set_mutex_post     0x56 // Par1 [ctx] Par2 [mutex]

// signal events
#define FTrace_EVT_signal             0x07
// more here (MarteOS)

// specific server events
#define FTrace_EVT_server_create      0x08 // Par1 [empty] Par2 [server]
#define FTrace_EVT_server_replenish   0x18 // Par1 [empty] Par2 [server]
#define FTrace_EVT_server_exhaust     0x28 // Par1 [empty] Par2 [server]
#define FTrace_EVT_server_reclaiming  0x38 // Par1 [empty] Par2 [server]
#define FTrace_EVT_server_remove      0x48 // Par1 [empty] Par2 [server]
#define FTrace_EVT_server_active      0x58 // Par1 [empty] Par2 [server]
#define FTrace_EVT_server_using_rec   0x68 // Par1 [reclaiming] Par2 [server]

// user defined events
#define FTrace_EVT_user_event_0       0x09
#define FTrace_EVT_user_event_1       0x19
#define FTrace_EVT_user_event_2       0x29
#define FTrace_EVT_user_event_3       0x39
#define FTrace_EVT_user_event_4       0x49
#define FTrace_EVT_user_event_5       0x59
#define FTrace_EVT_user_event_6       0x69
#define FTrace_EVT_user_event_7       0x79
#define FTrace_EVT_user_event_8       0x89
#define FTrace_EVT_user_event_9       0x99
#define FTrace_EVT_user_event_10      0xA9
#define FTrace_EVT_user_event_11      0xB9
#define FTrace_EVT_user_event_12      0xC9
#define FTrace_EVT_user_event_13      0xD9
#define FTrace_EVT_user_event_14      0xE9
//#define FTrace_EVT_user_event_15      0xF9  // Tool: removed

// Timer Events
#define FTrace_EVT_timer_post         0x0B // Par 1 [empty] Par2 [empty]
#define FTrace_EVT_timer_delete       0x1B // Par 1 [empty] Par2 [empty]
#define FTrace_EVT_timer_wakeup_start 0x2B // Par 1 [empty] Par2 [empty]
#define FTrace_EVT_timer_wakeup_end   0x3B // Par 1 [ctx] Par2 [empty]

// generic data events
#define FTrace_EVT_data_pointer       0x1A // holds a pointer of data from 
// previous event

#define FTrace_EVT_next_chunk	      0xFF

// types of the data pointer
#define FTrace_DATA_int		      1
#define FTrace_DATA_char              2

// filter definitions
#define FTrace_filter_trace_Events    0xF0
#define FTrace_filter_ipoint          0xF1
#define FTrace_filter_task            0xF2
#define FTrace_filter_interrupt       0xF3
#define FTrace_filter_CPU             0xF4
#define FTrace_filter_priority        0xF5
#define FTrace_filter_mutex           0xF6
#define FTrace_filter_signal          0xF7
#define FTrace_filter_server          0xF8
#define FTrace_filter_user            0xF9
#define FTrace_filter_data            0xFA
#define FTrace_filter_timer	      0xFB

#endif
