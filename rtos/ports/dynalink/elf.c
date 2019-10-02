////////////////////////////////////////////////////////////////////////
//	elf.c
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
#include "elf.h"


//#define __ELF_DEBUG__

char elf_signature[] = "\177ELF";

DWORD ELF_read_headers(	struct file_ops *kf,
						struct table_info *tables)
{
	//printk("ELF_read_headers\n");
  int res;

  DWORD entry;
  struct elf_header header;
  struct elf_section_header h;

  kf->file_seek(kf->file_offset + 0, kf->seek_set);
  kf->file_read(&header, sizeof(struct elf_header));
  
  if (memcmp(header.e_ident, elf_signature, 4)) {
    printk("Not an ELF file\n");
    printk("Wrong signature: 0x%lx)!!!\n",
	*(DWORD *)header.e_ident);
    return 0;
  }	
	//printk("ELF signature OK\n");

  tables->flags = 0;
  if (header.e_ident[4] != ELFCLASS32) {
    printk("Wrong ELF class\n");
    printk("Class: 0x%x!!!\n", header.e_ident[4]);
    return 0;
  }
	//printk("ELF Class OK\n");

  if (header.e_ident[5] != ELFDATA2LSB) {
    printk("Wrong data ordering (not LSB)\n");
    printk("Ordering: 0x%x!!!\n", header.e_ident[5]);
    return 0;
  }
	//printk("ELF data ordering OK\n");

#ifdef __ELF_DEBUG__
  if(header.e_machine != EM_386)
  {
    printk("Warning: machine = 0x%x!!!\n", header.e_machine);
  }  
#endif
	//printk("ELF Type: 0x%x\n", header.e_type);

  if(header.e_shoff != 0)
  {
#ifdef __ELF_DEBUG__
    printk("Section headers @ %ld\n", header.e_shoff);
    printk("Number of sections: %d\n", header.e_shnum);
    printk("Section header size: %d (0x%x)\n",
    		header.e_shentsize, header.e_shentsize);
#endif
    tables->section_header = header.e_shoff;
    tables->section_header_size = header.e_shentsize;
    tables->num_sections = header.e_shnum;
  }

#ifdef __ELF_DEBUG__
  if (header.e_phoff != 0) {
    printk("Program header table @ %ld\n", header.e_phoff);
    printk("Number of segments: %d\n", header.e_phnum);
    printk("Segment header size: %d (0x%x)\n",
    		header.e_phentsize, header.e_phentsize);
  }


#ifdef __ELF_DEBUG__
  /* Flags... */
  /* RELOCATION */
  if (header.f_flags & F_RELFLG) {
    printk("No relocation info!\n");
  }
  
  if (header.f_flags & F_EXEC) {
    printk("Executable file (no unresolved symbols)\n");
  }

  if (header.f_flags & F_LNNO) {
    printk("No line numbers!\n");
  }

  if (header.f_flags & F_LSYMS) {
    printk("No local symbols!\n");
  }

  if (header.f_flags & F_AR32WR) {
    printk("32-bit little endian!\n");
  } else {
    printk("File type?\n");
  }
#endif

  printk("Section Name String Table is section number %d\n",
  		header.e_shstrndx);
#endif
  if (header.e_shstrndx > tables->num_sections)
  {
    printk("Error: SNST number > section number...\n");
    return 0;
  }
  
  res = kf->file_seek( kf->file_offset + tables->section_header +
		  header.e_shstrndx * tables->section_header_size,
		  kf->seek_set);
  if (res < 0) {
    printk("Cannot seek");
    return 0;
  }
	//printk("ELF file seek OK\n");
	
  res = kf->file_read( &h, sizeof(struct elf_section_header));
  if (res < 0) {
    printk("Cannot read");
    return 0;
  }
	//printk("ELF file read OK\n");

  tables->section_names = 0;
  if(h.sh_size != 0)
  {
    //printk("ELF Loading Section Names...\n");

	tables->section_names = (void *)malloc(h.sh_size);
    if (tables->section_names == NULL)
	{
      printk("Failed to allocate space for section names...\n");
      return 0;
    }

    res = kf->file_seek( kf->file_offset + h.sh_offset, kf->seek_set);
    if (res < 0) {
      printk("Cannot seek");
      return 0;
    }
    res = kf->file_read( tables->section_names, h.sh_size);
    if (res < 0) {
      printk("Cannot read");
      return 0;
    }
    tables->section_names_size = h.sh_size;
  }
  else
  {
    printk("0 size?\n");
    tables->section_names_size = 0;
  }

  entry = header.e_entry;
  if (entry == 0) {
    tables->flags |= NO_ENTRY;
    tables->flags |= NEED_LOAD_RELOCATABLE;
    tables->flags |= NEED_SECTION_RELOCATION;
  }
	//printk("ELF Read headers Done!\n");
  return entry;
};


int ELF_read_section_headers(struct file_ops *kf,
							struct table_info *tables,
							struct section_info *scndata)
{
	
  int bss = -1;
  int i, j;
  struct elf_section_header h;
  int header_size;
  int stringtable = -1;
  struct elf_rel_info r;

  header_size = tables->section_header_size;
  if(header_size > sizeof(struct elf_section_header))
  {
    printk("Section header size (%d) > sizeof(struct section_header) (%d)\n",
    		header_size, (int)(sizeof(struct elf_section_header))  );
			
    header_size = sizeof(struct elf_section_header);
  }
  
  for (i = 0; i < tables->num_sections; i++)
  {
    kf->file_seek( kf->file_offset + tables->section_header +
					i * tables->section_header_size, kf->seek_set);
    kf->file_read( &h, sizeof(struct elf_section_header));

#ifdef __ELF_DEBUG__
    printk("Section %d: ", i);
    printk("Flags 0x%x: \n", h.sh_flags);
#endif
    /*
       Set this stuff to 0...
       If size == 0 the section must not be loaded
     */
    scndata[i].num_reloc = 0;
    scndata[i].base = 0;
    scndata[i].size = 0;
    scndata[i].fileptr = 0;
    scndata[i].filesize = 0;

    /* If this is a NULL section, skip it!!! */
    if (h.sh_type != SHT_NULL) {
      if (tables->section_names != 0) {
#ifdef __ELF_DEBUG__
        printk("[%s]", tables->section_names + h.sh_name);
#endif
        if (strcmp(tables->section_names + h.sh_name, ".bss") == 0) {
          bss = i;
        }
      }
#ifdef __ELF_DEBUG__
      printk("    <0x%lx:0x%lx> (0x%lx)\n",
	  h.sh_addr,
	  h.sh_addr + h.sh_size,
	  h.sh_offset);
#endif

      if (h.sh_type == SHT_REL) {
#ifdef __ELF_DEBUG__
        printk("\t\tSection %d: relocation info!!!\n", i);
        printk("\t\tSymbol table: section number %lu\n", h.sh_link);
        printk("\t\tSection to modify: %lu\n", h.sh_info);
        printk("\t\tNumber of relocation entries: %lu\n",
      			h.sh_size / h.sh_entsize);
#endif
        if (scndata[h.sh_info].num_reloc != 0) {
          printk("Double relocation for section\n");
	  printk("%lu?\n", h.sh_info);
	  return 0;
        }
	/* So, ...let's load it!!! */
        scndata[h.sh_info].num_reloc = h.sh_size / h.sh_entsize;
        //scndata[h.sh_info].reloc = (void *)kf->mem_alloc((h.sh_size / h.sh_entsize) * sizeof(struct reloc_info));
		scndata[h.sh_info].reloc = (void *)malloc((h.sh_size / h.sh_entsize) * sizeof(struct reloc_info));
        if (scndata[h.sh_info].reloc == NULL) {
          printk("Failed to allocate space for relocation info...\n");
          return 0;
        }
			//printk("ELF Allocate space for relocation info OK!\n");
		
	for (j = 0; j < h.sh_size / h.sh_entsize; j++) {
          kf->file_seek( kf->file_offset + h.sh_offset + j * h.sh_entsize,
			  kf->seek_set);
          kf->file_read( &r, sizeof(struct elf_rel_info));
	  scndata[h.sh_info].reloc[j].offset = r.r_offset;
	  scndata[h.sh_info].reloc[j].symbol = r.r_info >> 8;
/* HACKME!!! Unify the relocation types... */
	  scndata[h.sh_info].reloc[j].type = (BYTE)r.r_info;
	  if ((BYTE)r.r_info == R_386_32){
            scndata[h.sh_info].reloc[j].type = REL_TYPE_ELF_ABSOLUTE;
          } else if ((BYTE)r.r_info == R_386_PC32) {
            scndata[h.sh_info].reloc[j].type = REL_TYPE_RELATIVE;
          }
	}
      } else if (h.sh_type == SHT_RELA) {
        printk("Error: unsupported relocation section!!!\n");

	return 0;
      } else if ((h.sh_type == SHT_SYMTAB) || (h.sh_type == SHT_DYNSYM)) {
#ifdef __ELF_DEBUG__
        printk("\t\tSection %d: symbol table!!!\n", i);
        printk("\t\tString table: section number %lu\n", h.sh_link);
        printk("\t\tLast local Symbol + 1: %lu\n", h.sh_info);
#endif
	tables->symbol = h.sh_offset;
	tables->num_symbols = h.sh_size / h.sh_entsize;
	tables->symbol_size = h.sh_size;
	if (stringtable != -1) {
	  printk("Error: double string table!!!\n");
	  return 0;
	}
	stringtable = h.sh_link;
	if (stringtable < i) {
	  printk("Strange... ");
	  printk("String table (%d) < Symbol Table\n", stringtable);
	  return 0;
	}
      } else if (i == stringtable) {
#ifdef __ELF_DEBUG__
        printk("\t\t Section %d: string table!!!\n", i);
#endif
	tables->string = h.sh_offset;
	tables->string_size = h.sh_size;
	stringtable = -1;
      } else {
        scndata[i].base = h.sh_addr;
        scndata[i].size = h.sh_size;
        scndata[i].fileptr = h.sh_offset;
        if (h.sh_type != SHT_NOBITS) {
#ifdef __ELF_DEBUG__
	  printk("BSS?\n");
#endif
          scndata[i].filesize = h.sh_size;
        }
      }
    } else {
#ifdef __ELF_DEBUG__
      printk("NULL Section\n");
#endif
    }

#if 0
    if (h.s_flags & SECT_TEXT) {
      printk("Executable section\n");
    }
    if (h.s_flags & SECT_INIT_DATA) {
      printk("Data section\n");
    }
    if (h.s_flags & SECT_UNINIT_DATA) {
      printk("BSS section\n");
      scndata[i].filesize = 0;
    }
#endif
  }
  tables->image_base = scndata[0].base; 
  return bss;
};


int ELF_read_symbols(struct file_ops *kf,
					struct table_info *tables,
					struct symbol_info *syms)							
{
	//printk("ELF read_symbols\n");
	
  int i;
  int entsize;
  struct elf_symbol_info symbol;
  char *s;

  s = (void *)malloc(tables->string_size);
  if(s == NULL)
  {
    printk("Failed to allocate space for string table...\n");
    return 0;
  }
  
  tables->string_buffer = (DWORD)s;
  kf->file_seek( kf->file_offset + tables->string, kf->seek_set);
  kf->file_read( s, tables->string_size);
   
  entsize = tables->symbol_size / tables->num_symbols;

  for (i = 0; i < tables->num_symbols; i++)
  {
    kf->file_seek( kf->file_offset + tables->symbol + i * entsize,
						kf->seek_set);
    kf->file_read( &symbol, sizeof(struct elf_symbol_info));
    syms[i].name = s + symbol.st_name;
    syms[i].section = symbol.st_shndx;
    syms[i].offset = symbol.st_value;

    if (syms[i].section == SHN_UNDEF) {
      /* extern symbol */
      if (symbol.st_name != 0)
        syms[i].section = EXTERN_SYMBOL;
      else /* Mark the empty entry, external symbol with no name is not used :-) */
        syms[i].section = NULL_SYMBOL;
    }
    if (syms[i].section == SHN_COMMON) {
      /* extern symbol */
      syms[i].section = COMMON_SYMBOL;
      syms[i].offset = symbol.st_size;
      /* calculate the local_bss_size */
      tables->local_bss_size += syms[i].offset;
    }
  }

  return 1;
};

int Elf_check(struct file_ops *kf)
{
  char signature[4];

  kf->file_offset = kf->file_seek( 0, kf->seek_cur);
  kf->file_read(signature, 4);
  kf->file_seek( kf->file_offset + 0, kf->seek_set);

  if(memcmp(signature, elf_signature, 4))
  {
    return 0;
  }
 
  return 1;
};

int ELF_relocate_section(struct file_ops *kf,
							DWORD base,
							struct table_info *tables,
							int n,
							struct section_info *s,
							int sect,
							struct symbol_info *syms,
							struct symbol *import)
{
	
	int i, idx;
	DWORD address, destination;
	int j, done;
	DWORD local_bss = tables->local_bss;
	struct reloc_info *rel = s[sect].reloc;

	/* Setup the common space-uninitialized symbols at the first section relocation
	* Pre-calculate the local BSS size (in read_symbols)
	* then allocate for each symbol (in load_relocatable)
	*/
	
  if(sect == 0)
  {
    for(i=0; i<tables->num_symbols; i++)
	{
      if(syms[i].section == COMMON_SYMBOL)
	  {
        j = syms[i].offset;
        syms[i].offset = local_bss;
        local_bss += j;
      }
	  else if(syms[i].section == EXTERN_SYMBOL)
	  {
		#ifdef __ELF_DEBUG__
        printk("Searching for symbol %s\n", syms[i].name);
		#endif
        /* Pre-set the external symbol at the same time */
        for(j=0, done=0; import[j].name != 0; j++)
		{
          if(strcmp(import[j].name, syms[i].name) == 0)
		  {
            syms[i].offset = import[j].address;
            done = 1;
            break;
          }
		}
        if(done == 0)
		{
            printk("Symbol %s not found\n", syms[i].name);
            return -1;
        }
		
      }
    }
  }

  //printk("[COMMON] s[sect].num_reloc = %d\n", s[sect].num_reloc);
  
  for(i=0; i < s[sect].num_reloc; i++)
  {
	#ifdef __COFF_DEBUG__
    printk("Relocate 0x%lx (index 0x%x): mode %d ",
			rel[i].offset, rel[i].symbol, rel[i].type);
	#endif
    idx = rel[i].symbol;

	#ifdef __COFF_DEBUG__
    printk("%s --> 0x%lx (section %d)\n", syms[idx].name,
			syms[idx].offset, syms[idx].section);
	#endif

    switch (rel[i].type)
    {
      case REL_TYPE_ELF_ABSOLUTE:
        destination = s[sect].base + rel[i].offset + base;
        // Initial address
        address = *((DWORD*)destination);
        break;
      case REL_TYPE_RELATIVE:
        destination = s[sect].base + rel[i].offset + base;
        address = 0;
        break;
      default:
      	// (Non-)external symbols: only REL32 is supported
        printk("Unsupported relocation!\n");
        printk("Relocation Type: %d\n", rel[i].type);
        return -1;
    }
	
	if(syms[idx].section == COMMON_SYMBOL || syms[idx].section == EXTERN_SYMBOL)
	{ 
		if(rel[i].type == REL_TYPE_ELF_ABSOLUTE)
			address += syms[idx].offset;
			
		else if(rel[i].type == REL_TYPE_RELATIVE)
			address = syms[idx].offset - destination - 4;
	}
	else if(syms[idx].section >= n)
	{
		// Check if the section exists ... 
		printk("Unsupported relocation section\n");
		printk("Section %d > %d\n", syms[idx].section, n);
		printk("Value 0x%lx\n", syms[idx].offset);
		return -1;
	}
	else
	{ 
		if(rel[i].type == REL_TYPE_ELF_ABSOLUTE)
		{
			address += base + s[syms[idx].section].base + syms[idx].offset;
		}
		else if(rel[i].type == REL_TYPE_RELATIVE)
		{
			address = (s[syms[idx].section].base + syms[idx].offset) - (s[sect].base + rel[i].offset) - 4;
			#ifdef __COFF_DEBUG__
			printk("Reloc: 0x%lx + 0x%lx - 0x%lx = 0x%lx   ",
				syms[idx].offset,
				s[syms[idx].section].base,
				rel[i].offset , address);
			#endif
		}
	}
	
	#ifdef __COFF_DEBUG__
    printk("0x%lx <--- 0x%lx\n", destination, address);
	#endif
    *((DWORD*)destination) = address;
  }

  return 1;
};


/* Import symbol with suffix `name'
 * NOTE: Any symbol with prefix `_' won't be found and be regarded as internal and hidden
 */
DWORD ELF_import_symbol(struct file_ops *kf,
							int n,
							struct symbol_info *syms,
							char *name,
							int *sect)
{
	//printk("[COMMON] import_symbol...\n");
	
  int i;
  int len = strlen(name);

  for(i = 0; i < n ; i++)
  {
	#ifdef __COFF_DEBUG__
    printk("Checking symbol %d [%d] --- Sect %d\n", i, n, syms[i].section);
	#endif
    if ((syms[i].section != EXTERN_SYMBOL) && (syms[i].section != COMMON_SYMBOL) && (syms[i].section != NULL_SYMBOL))
	{
	  #ifdef __COFF_DEBUG__
      printk("Compare %s, %s\n", syms[i].name, name);
	  #endif
      if(syms[i].name[0] != '_')
	  {
        int sym_len = strlen(syms[i].name);
        if(sym_len >= len && strcmp(syms[i].name+sym_len-len, name) == 0)
		{
		  #ifdef __COFF_DEBUG__
          printk("Found: %s --- 0x%x : 0x%lx\n",
				syms[i].name, syms[i].section, syms[i].offset);
		  #endif
          break;
        }
		#ifdef __COFF_DEBUG__
        else
		{
          printk("Cmp failed --- Going to %d\n", i);
        }
		#endif
      }
    }
	#ifdef __COFF_DEBUG__
    else
	{
      printk("Skipped symbol --- Going to %d\n", i);
    }
	#endif
  }

  if(i < n)
  { /* Symbol found */
    *sect = syms[i].section;
    return syms[i].offset;
  }
  else
  {
    *sect = -1;
    printk("Symbol not found!!!\n");
    return 0;
  }
};


						
DWORD ELF_load_relocatable(	struct file_ops *kf,
								struct table_info *tables,
								int n,
								struct section_info *s,
								DWORD *size)								
{
	int i;
	DWORD needed_mem = 0;
	DWORD local_offset = 0;
	BYTE *mem_space, *where_to_place;

	// Allocate for the local bss at the mean time 
	for(i=0; i<n; i++)
	{
		needed_mem += s[i].size;
	}
	needed_mem += tables->local_bss_size;
	mem_space = (BYTE *)malloc(needed_mem);
	if(mem_space == NULL)
	{
		printk("Unable to allocate memory for the program image\n");
		return 0;
	}
	memset(mem_space, 0, needed_mem); 

	#ifdef __ELF_DEBUG__
	printk("Loading relocatable @%p; size 0x%lx\n", mem_space, needed_mem);
	#endif

  
	if(tables->local_bss_size != 0)
		tables->local_bss = (DWORD)mem_space + needed_mem - tables->local_bss_size;
	else
		tables->local_bss = 0;


	for(i=0; i<n; i++)
	{
		#ifdef __ELF_DEBUG__
		printk("Section %d\t", i);
		#endif
		if(s[i].size != 0)
		{
			#ifdef __ELF_DEBUG__
			printk("Loading @ 0x%lx (0x%lx + 0x%lx)...\n",
					(DWORD)mem_space + (DWORD)local_offset,
					(DWORD)mem_space, local_offset);
			#endif
			
			where_to_place = mem_space + local_offset;
			s[i].base = local_offset;
			local_offset += s[i].size;
			kf->file_seek( kf->file_offset + s[i].fileptr, kf->seek_set);
			if(s[i].filesize > 0)
			{
				kf->file_read( where_to_place, s[i].filesize);
			}
		}
		else
		{
			#ifdef __ELF_DEBUG__
			printk("Not to be loaded\n");
			#endif
		}
	}

	*size = needed_mem;
 return (DWORD)mem_space;
};


void ELF_free_tables(struct file_ops *kf,
						struct table_info *tables,
						struct symbol_info *syms,
						struct section_info *scndata)
{
  int i;

  for(i = 0; i < tables->num_sections; i++)
    if (scndata[i].num_reloc != 0)
	  free( scndata[i].reloc);

  free(scndata);  
  if (syms != NULL)	free(syms);
  if (tables->string_size != 0)   free((DWORD*)tables->string_buffer);
  if (tables->section_names_size != 0)    free(tables->section_names);  
};

