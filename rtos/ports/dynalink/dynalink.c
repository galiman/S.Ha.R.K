////////////////////////////////////////////////////////////////////////
//	dynalink.c
//
//  DynaLink for S.H.A.R.K
//  Dynamic ELF object linker.
//  
//  Original code written by Luca Abeni.
//  Adapted by Lex Nahumury 19-7-2006.
//  
//  This is free software; see GPL.txt
//////////////////////////////////////////////////////////////////////// 
 
#include "kernel/kern.h"
#include <kernel/func.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <ll/i386/hw-data.h>
#include <ll/i386/mem.h>
#include <ll/i386/hw-instr.h>
#include <ll/i386/cons.h>
#include <ll/i386/error.h>
#include <ll/i386/mem.h>
#include <ll/ctype.h>
#include <ll/i386/x-bios.h>

#include "format.h"
#include "dynalink.h"
#include "elf.h"

#include <drivers/shark_linuxc26.h>
#include <drivers/shark_pci26.h>
#include <drivers/shark_input26.h>
#include <drivers/shark_keyb26.h>

// some shark pci forward declaration stuff (pci20to26.c)
extern int pci20to26_find_class(unsigned int class_code, int index, BYTE *bus, BYTE *dev);
extern int pci20to26_read_config_byte(unsigned int bus, unsigned int dev, int where, BYTE *val);
extern int pci20to26_read_config_word(unsigned int bus, unsigned int dev, int where, WORD *val);
extern int pci20to26_read_config_dword(unsigned int bus, unsigned int dev, int where, DWORD *val);
extern int pci20to26_write_config_byte(unsigned int bus, unsigned int dev, int where, BYTE val);
extern int pci20to26_write_config_word(unsigned int bus, unsigned int dev, int where, WORD val);
extern int pci20to26_write_config_dword(unsigned int bus, unsigned int dev, int where, DWORD val);


#define __DYNA_DEBUG__
#define NORMAL_PROCESS	0
#define EMPTY_SLOT {0, (DWORD)0xFFFFFFFF}
#define SUBSTITUTE 1
#define ADD 0


static struct file_ops 	kf;
static DWORD start;
static DWORD end;
static DWORD pointer;


//A module can also export symbols by adding them to the kernel symbol table
int add_call(char *name, DWORD address, int mode);

// forward declarations of some custom 'syscalls'...
extern void call_shutdown_task(void *arg);
extern DWORD get_tick();

static struct symbol syscall_table[] = {
	// console & string
	{ "cprintf", (DWORD)cprintf },
	{ "sprintf", (DWORD)sprintf },
	{ "printk", (DWORD)printk },
	{ "printf_xy", (DWORD)printf_xy },
	{ "puts_xy", (DWORD)puts_xy },
	{ "place", (DWORD)place },
	{ "_clear", (DWORD)_clear },	
	{ "clear", (DWORD)clear },	
	{ "strcpy", (DWORD)strcpy },	
	// keyboard
	{ "keyb_getch", (DWORD)keyb_getch },
	{ "keyb_getcode", (DWORD)keyb_getcode },
	// math
	{ "sin", (DWORD)sin },
	{ "cos", (DWORD)cos },
	{ "pow", (DWORD)pow },
	{ "abs", (DWORD)abs },	
	// memory
	{ "malloc",  (DWORD)malloc },
	{ "kern_alloc_aligned",  (DWORD)kern_alloc_aligned },
	{ "free",  (DWORD)free },
	// I/O
	{ "inp",  (DWORD)inp },
	{ "inpw",  (DWORD)inpw },
	{ "inpd",  (DWORD)inpd },
	// pci
	{ "pci20to26_read_config_byte",  (DWORD)pci20to26_read_config_byte },
	{ "pci20to26_read_config_word",  (DWORD)pci20to26_read_config_word },
	{ "pci20to26_read_config_dword",  (DWORD)pci20to26_read_config_dword },
	{ "pci20to26_write_config_byte",  (DWORD)pci20to26_write_config_byte },
	{ "pci20to26_write_config_word",  (DWORD)pci20to26_write_config_word },
	{ "pci20to26_write_config_dword",  (DWORD)pci20to26_write_config_dword },
	// task managment
	{ "task_testcancel",  (DWORD)task_testcancel },
	{ "task_message",  (DWORD)task_message },
	{ "task_createn",  (DWORD)task_createn },
	{ "task_activate",  (DWORD)task_activate },
	{ "handler_set",  (DWORD)handler_set },
	{ "task_nopreempt",  (DWORD)task_nopreempt },
	{ "task_preempt",  (DWORD)task_preempt },	
	{ "group_activate",  (DWORD)group_activate },
	// Time & termination
	{ "sys_gettime",  (DWORD)sys_gettime },
	{ "get_tick",  (DWORD)get_tick },
	{ "perror",  (DWORD)perror },
	{ "exit",  (DWORD)exit },
	{ "call_shutdown_task",  (DWORD)call_shutdown_task },
	{ "sys_shutdown_message",  (DWORD)sys_shutdown_message },	
	{ "add_call", (DWORD)add_call }, //module can add symbol to the kernel symbol table  	
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT, 
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT,
	EMPTY_SLOT, 
	EMPTY_SLOT,  
	{0, 0}
};

void *get_syscall_table(void) 
{
  return syscall_table;
};

int add_call(char *name, DWORD address, int mode)
{
  int i;
  int done;
  int res;
  
  i = 0;
  done = 0;
  while ((!done) && (syscall_table[i].address != 0)) {
    if (!strcmp(syscall_table[i].name, name)) {
      done = 1;
    } else {
      i++;
    }
  }

  if (done == 1) {
    if (mode == SUBSTITUTE) {
      res = syscall_table[i].address;
      syscall_table[i].name = name;
      syscall_table[i].address = address;
    } else {
      return -1;
    }
  } else {
    if (mode == SUBSTITUTE) {
      return -1;
    } else {
      i = 0;
      while (syscall_table[i].name != 0) {
        i++;
      }
      
      if (syscall_table[i].address != 0xFFFFFFFF) {
        return -1;
      }
      res = 1;
      syscall_table[i].name = name;
      syscall_table[i].address = address;    
    }
  }
  
  return res;
};



static DWORD load_process(struct file_ops *kf,				
						DWORD *ex_exec_space,
						DWORD *image_base,
						DWORD *ex_size)
{	
  #ifdef __EXEC_DEBUG__
  DWORD offset;
  #endif
  
  DWORD size;
  DWORD exec_space=0;
  int bss_sect, i;
  DWORD dyn_entry;
  struct section_info	*sections;
  struct symbol_info	*symbols = NULL;
  struct table_info		tables;

  // Initialize the table info
  tables.section_names_size = 0;
  tables.local_bss_size = 0;
  tables.private_info = 0;

  // read ELF headers
  dyn_entry = ELF_read_headers(kf, &tables);  

  sections = (struct section_info*)malloc(sizeof(struct section_info) * tables.num_sections);
  if(sections == 0)
  {
    printk("Can't allocate the sections info!\n");    
    return -1;
  }

  // read ELF section headers
  bss_sect = ELF_read_section_headers(kf, &tables, sections);
  
 
  // Load symbols (if it's objects, calculate the local bss size)
  if(tables.num_symbols != 0)
  {
    symbols = (struct symbol_info*)malloc(tables.num_symbols * sizeof (struct symbol_info));
    if (symbols == 0)
	{
      error("Error allocating symbols table\n");
      ELF_free_tables(kf, &tables, symbols, sections);
      return -1;
    }
	
	// read ELF symbols
    ELF_read_symbols(kf, &tables, symbols);
  }

  if(tables.flags & NEED_LOAD_RELOCATABLE)
  {	
  	#ifdef __EXEC_DEBUG__
	printk("[EXEC]Must load_relocatable()!\n");
	#endif
    exec_space = ELF_load_relocatable(kf, &tables, tables.num_sections, sections, &size);
  }
  if(exec_space == 0)
  {
    printk("Error decoding the Executable data\n");
    ELF_free_tables(kf, &tables, symbols, sections);
    return -1;
  }

 
  if(tables.flags & (NEED_SECTION_RELOCATION|NEED_IMAGE_RELOCATION))
  {
    int res;
    void *kernel_symbols;
    int reloc_sections;

    if (tables.flags & NEED_SECTION_RELOCATION)
	{
      if ((bss_sect < 0) || (bss_sect > tables.num_sections))
	  {
        error("Error: strange file --- no BSS section\n");
        /* TODO: Return code... */
        ELF_free_tables(kf, &tables, symbols, sections); 
		free((DWORD*)exec_space);
        return -1;
      }
    }
	#ifdef __EXEC_DEBUG__
    printk("[EXEC] Start of local BSS: 0x%lx\n", tables.private_info);
	#endif

    kernel_symbols = get_syscall_table();
	
    if(tables.flags & NEED_SECTION_RELOCATION)
	{
      reloc_sections = tables.num_sections;
    } else
	{
      reloc_sections = 1;
    }
	
	#ifdef __EXEC_DEBUG__
	printk("[EXEC] reloc_sections = %d\n", reloc_sections);
	#endif
	
	// Relocate sections...
    for (i=0; i<reloc_sections; i++)
	{
      res = ELF_relocate_section(kf,
								exec_space,
								&tables,
								tables.num_sections,
								sections,
								i,
								symbols,
								kernel_symbols);
      if(res < 0)
	  {
        error("Error: relocation failed!!!\n");
        /* TODO: Return code... */
        ELF_free_tables(kf, &tables, symbols, sections); 
		free((DWORD*)exec_space);
        return -1;
      }
    }	
  }
	
  /* The size of the execution space */
  *ex_size = size;
  
  if(tables.flags & NO_ENTRY)
  { 
    int init_sect;
    /* No entry point... We assume that we need dynamic linking */	 
	#ifdef __EXEC_DEBUG__
    printk("[EXEC] Searching for the initialization function...\n");
	#endif
    if(symbols == 0)
	{
      error("Error: no symbols!!!\n");
      /* TODO: Return code... */
      ELF_free_tables(kf, &tables, symbols, sections); 
	  free((DWORD*)exec_space);
      return -1;
    }	

	// Find the main entry point. Function name ends with kf->entry_name...
	//dyn_entry = (DWORD)ELF_import_symbol(kf, tables.num_symbols, symbols, DYN_ENTRY_NAME, &init_sect);
	dyn_entry = (DWORD)ELF_import_symbol(kf, tables.num_symbols, symbols, kf->entry_name, &init_sect);
	
    if(init_sect != -1)
	{
	  #ifdef __EXEC_DEBUG__
      printk("Found: (%d =  0x%x) 0x%lx\n", init_sect,
											init_sect, dyn_entry);
      printk("Section Base: 0x%lx   Exec Space: 0x%lx\n",
										sections[init_sect].base, exec_space);
	  #endif
      dyn_entry += sections[init_sect].base + exec_space;
      *image_base = exec_space;
      *ex_exec_space = 0;
      ELF_free_tables(kf, &tables, symbols, sections);
      return dyn_entry;
    }
	else
	{
      printk("WARNING: Initialization function not found!\n");
      return -1;
    }
  }
 return -1; // error
};



// Read an ELF object in memory, and return it's main entry function's address..
static DWORD get_exec_address(	struct file_ops *kf )
{	
	DWORD exec_space;
	DWORD image_base;
	DWORD ex_size;
	DWORD dyn_entry;
	
	dyn_entry = load_process(	kf,						
								&exec_space,
								&image_base,
								&ex_size);

	if(dyn_entry== -1)    return 0;
 return dyn_entry;
};

/*
static char *module_command_line(DWORD mods_addr, int i)
{
  struct mods_struct *curr_mod;
  curr_mod = (struct mods_struct *)mods_addr;
  return curr_mod[i].string;
};

static void module_address(DWORD mods_addr, int i, DWORD *start, DWORD *end)
{
  struct mods_struct *curr_mod;
  curr_mod = (struct mods_struct *)mods_addr;
  *start = (DWORD)curr_mod[i].mod_start;
  *end = (DWORD)curr_mod[i].mod_end;
};
*/

static int modfs_read( void *buff, int size)
{
  int len = size;
  if(pointer + len > end)
  {
    len = end - pointer;
  }
  memcpy(buff, (void *)pointer, len);
  pointer += len;
  return len;
};


static int modfs_seek( int pos, int wence)
{
  int res;

  if(wence == 0)
  {
    res = pos;
  }
  else if(wence == 1)
  {
    res = pointer + pos - start;
  }
  else if(wence == 2)
  {
    if(end - start < pos) res = 0;
	else res = end - start - pos;
  }
  else  res = pointer - start;

  if(start + res > end) res = end - start;
  
  pointer = start + res;
  
  return res;
};

// This will process all modules and fill out a dynalink_module_list.
// We only support valid ELF objects.
// Everything else is treated as DATA.
int dynalink_modules(struct multiboot_info* mb,
					struct dynalink_module_list* dml,
					char* search_string)
{
	int i;
	DWORD dyn_entry;
	
	int mods_addr	= mb->mods_addr;
	int mods_count	= mb->mods_count;	
	
	// Initialize the Modules Pseudo-FS...	
	kf.file_seek = modfs_seek;
	kf.file_read = modfs_read;
	kf.entry_name = search_string;
	kf.seek_set = 0;
	kf.seek_cur = 1;
	
	// check all modules..
	for(i = 0; i < mods_count; i++)
	{
		#ifdef __DYNA_DEBUG__
		printk("[MOD] Process Module #%d:", i);
		#endif
		
		kf.file_offset = 0;
		
		// Pseudo-FS open		
		struct mods_struct* curr_mod = (struct mods_struct*)mods_addr;
		start = (DWORD)curr_mod[i].mod_start;
		end = (DWORD)curr_mod[i].mod_end;		
		pointer = start;

		if( Elf_check(&kf) )
		{	
			// Try to Link the ELF module and get it's main_entry address
			dyn_entry = get_exec_address( &kf );			
			if(dyn_entry)
			{
				int j = dml->num_apps;
				dml->app[j].dyn_entry	= dyn_entry;
				dml->app[j].start	 	= start;
				dml->app[j].end		 	= end;
				dml->app[j].size		= end - start;
				dml->app[j].name		= curr_mod[i].string;
				#ifdef __DYNA_DEBUG__	
				printk(" ELF object :%s [%d bytes]\n",
						dml->app[j].name, dml->app[j].size);
				#endif
				++dml->num_apps;
			}
			else // add it as 'Data' module
			{
				int j = dml->num_dats;			
				dml->dat[j].start	= start;
				dml->dat[j].end		= end;
				dml->dat[j].size	= end - start;
				dml->dat[j].name	= curr_mod[i].string;
				#ifdef __DYNA_DEBUG__	
				printk(" Data object :%s [%d bytes]\n",
						dml->dat[j].name, dml->dat[j].size);
				#endif				
				++dml->num_dats;
			}
		}
		else // it is a 'Data' module
		{
			int j = dml->num_dats;		
			dml->dat[j].start	= start;
			dml->dat[j].end		= end;
			dml->dat[j].size	= end - start;
			dml->dat[j].name	= curr_mod[i].string;
			#ifdef __DYNA_DEBUG__	
			printk(" Data object :%s [%d bytes]\n",
					dml->dat[j].name, dml->dat[j].size);
			#endif					
			++dml->num_dats;
		}
		
		//TODO: ? mem_release_module(mods_addr, i, mods_count);
	}
return 0;
};



/*
// TODO: haven't finished this yet for SHARK [Lex]
void mem_release_module(DWORD mstart, int m, int mnum)
{
  DWORD ms, nms, me, ms1, dummy;
  int i;

  module_address(mstart, m, &ms, &me);
  nms = 0;
  for (i = 0; i < mnum; i++)
  {
    module_address(mstart, i, &ms1, &dummy);
    if(nms == 0)
	{
      if(ms1 >= me) nms = ms1;
    }
	else
	{
      if((ms1 >= me) && (ms1 < nms))  nms = ms1;  
    }
  }
  
  if (nms == 0)  nms = me;
  
#ifdef __MEM_DEBUG__
  printk("releasing %lx - %lx\n", ms, nms);
#endif

  //pmm_add_region(&high_mem_pool, ms, nms - ms);
  

};
*/

