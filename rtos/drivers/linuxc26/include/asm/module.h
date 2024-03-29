#ifndef _ASM_I386_MODULE_H
#define _ASM_I386_MODULE_H

#include <linuxcomp.h>

/* x86 is simple */
struct mod_arch_specific
{
};

#define Elf_Shdr Elf32_Shdr
#define Elf_Sym Elf32_Sym
#define Elf_Ehdr Elf32_Ehdr

#ifdef CONFIG_M386
#define MODULE_PROC_FAMILY "386 "
#elif defined CONFIG_M486
#define MODULE_PROC_FAMILY "486 "
#elif defined CONFIG_M586
#define MODULE_PROC_FAMILY "586 "
#elif defined CONFIG_M586TSC
#define MODULE_PROC_FAMILY "586TSC "
#elif defined CONFIG_M586MMX
#define MODULE_PROC_FAMILY "586MMX "
#elif defined CONFIG_M686
#define MODULE_PROC_FAMILY "686 "
#elif defined CONFIG_MPENTIUMII
#define MODULE_PROC_FAMILY "PENTIUMII "
#elif defined CONFIG_MPENTIUMIII
#define MODULE_PROC_FAMILY "PENTIUMIII "
#elif defined CONFIG_MPENTIUM4
#define MODULE_PROC_FAMILY "PENTIUM4 "
#elif defined CONFIG_MK6
#define MODULE_PROC_FAMILY "K6 "
#elif defined CONFIG_MK7
#define MODULE_PROC_FAMILY "K7 "
#elif defined CONFIG_MK8
#define MODULE_PROC_FAMILY "K8 "
#elif defined CONFIG_MELAN
#define MODULE_PROC_FAMILY "ELAN "
#elif defined CONFIG_MCRUSOE
#define MODULE_PROC_FAMILY "CRUSOE "
#elif defined CONFIG_MWINCHIPC6
#define MODULE_PROC_FAMILY "WINCHIPC6 "
#elif defined CONFIG_MWINCHIP2
#define MODULE_PROC_FAMILY "WINCHIP2 "
#elif defined CONFIG_MWINCHIP3D
#define MODULE_PROC_FAMILY "WINCHIP3D "
#elif defined CONFIG_MCYRIXIII
#define MODULE_PROC_FAMILY "CYRIXIII "
#elif CONFIG_MVIAC3_2
#define MODULE_PROC_FAMILY "VIAC3-2 "
#else
#error unknown processor family
#endif

#define MODULE_ARCH_VERMAGIC MODULE_PROC_FAMILY

#endif /* _ASM_I386_MODULE_H */
