# CPU support from linux 2.6.0

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = cpu

OBJS_PATH     = $(BASE)/drivers/cpu

OBJS = common.o	amd.o cyrix.o intel.o\
	cpufreq/cpufreq.o cpufreq/freq_table.o\
	cpufreq/powernow-k6.o cpufreq/powernow-k7.o cpufreq/powernow-k8.o\
	cpufreq/gx-suspmod.o cpufreq/p4-clockmod.o\
	cpufreq/speedstep-lib.o cpufreq/speedstep-centrino.o\
	cpufreq/speedstep-ich.o\
	shark/shark_cpu.o
#	cpufreq/speedstep-smi.o 

OTHERINCL += -I$(BASE)/drivers/linuxc26/include

C_OPT += -D__KERNEL__

include $(BASE)/config/lib.mk
