////////////////////////////////////////////////////////////////////////
//	dynalink.h
//
//  DynaLink for S.H.A.R.K
//  Dynamic ELF object linker.
//  
//  Original code written by Luca Abeni.
//  Adapted by Lex Nahumury 19-7-2006.
//  
//  This is free software; see GPL.txt
//////////////////////////////////////////////////////////////////////// 
#ifndef _DYNALINK_H_
#define _DYNALINK_H_

#define MAX_DYNA_MOD	64

struct mods_struct
{
  void *mod_start;
  void *mod_end;
  char *string;
  DWORD reserved;
};


struct app_module_info
{
	unsigned int start;		// the memory used goes from bytes 'mod_start'
	unsigned int end;		// to 'mod_end-1' inclusive
	unsigned int size; 		// size in bytes
	unsigned int dyn_entry;	// main_module_entry() start address
	char* name;				// file name
};

struct dat_module_info
{
	unsigned int start;		// the memory used goes from bytes 'mod_start'
	unsigned int end;		// to 'mod_end-1' inclusive.
	unsigned int size; 		// in bytes
	char *name;				// file name
};

struct dynalink_module_list
{
	int num_apps;								// number of loaded ELF objects
	int num_dats;								// number of loaded Data object
	struct app_module_info	app[MAX_DYNA_MOD];	// aplications in mem
	struct dat_module_info	dat[MAX_DYNA_MOD];	// data file in mem
};


int dynalink_modules(	struct multiboot_info* mb,
						struct dynalink_module_list* dml,
						char* search_string);



#endif //_DYNALINK_H_



