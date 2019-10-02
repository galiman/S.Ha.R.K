////////////////////////////////////////////////////////////////////////
//	format.h
//
//  DynaLink for S.H.A.R.K
//  Dynamic ELF object linker.
//  
//  Original code written by Luca Abeni.
//  Adapted by Lex Nahumury 19-7-2006.
//  
//  This is free software; see GPL.txt
//////////////////////////////////////////////////////////////////////// 

#ifndef __FORMAT_H__
#define __FORMAT_H__

#define EXTERN_SYMBOL 0xFF00
#define COMMON_SYMBOL 0xFF01
#define NULL_SYMBOL   0xFFFF

#define REL_TYPE_ELF_ABSOLUTE   1
#define REL_TYPE_RELATIVE       4

#define NEED_IMAGE_RELOCATION	1
#define NEED_SECTION_RELOCATION	2
#define NEED_LOAD_RELOCATABLE	4
#define NO_ENTRY				16
#define DLL_WITH_STDCALL		32


struct reloc_info {
  DWORD offset;
  int symbol;
  int type;
};


struct symbol_info {
  char *name;
  DWORD offset;
  WORD section;
};

struct table_info {
  DWORD section_header;
  WORD num_sections;
  WORD section_header_size;
  WORD flags;
  DWORD symbol;
  DWORD num_symbols;
  DWORD symbol_size;
  DWORD string;
  DWORD string_size;
  DWORD string_buffer;
  char *section_names;
  DWORD section_names_size;  
  DWORD image_base;  
  DWORD local_bss;
  DWORD local_bss_size;
  /* In PEI, it's a pei extra info */
  DWORD private_info;
};

struct section_info {
  DWORD base;
  DWORD size;
  DWORD fileptr;
  DWORD filesize;
  int num_reloc;
  struct reloc_info *reloc;
};

/* Only used for ``import'' symbols... */
struct symbol {
  char *name;
  DWORD address;
};


struct file_ops
{
    int (*file_read)( void *buffer, int len);
    int (*file_seek)( int position, int wence);	
    int seek_set;
    int seek_cur;
    int file_offset;
	char* entry_name;
};


#endif
