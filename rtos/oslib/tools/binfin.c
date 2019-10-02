
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

#include <malloc.h>
#include <stdio.h>
#include "../ll/i386/mb-hdr.h"

/*
 * Warning!!! This program does not work properly...
 * FIXIT!
 */
void main(int argc, char *argv[])
{
    FILE *in;
    char *buf,*p;
    register scan;
    unsigned long multiboot_sign = MULTIBOOT_MAGIC, size;
    unsigned long *search;
    
    if (argc != 2) {
	printf("Usage: binfin <file>\n");
	exit(0);
    }
    in = fopen(argv[1],"rb");
    if (in == NULL) {
	printf("Error! File %s not found\n",argv[1]);
	exit(-1);
    }
    fseek(in, 0, SEEK_END);
    size = ftell(in);
    buf = malloc(size);
    if (buf == NULL) {
	printf("Malloc error! Requested : %lu\n",size);
	exit(-2);
    }
    fread(buf,1,size,in);
    fclose(in);
    for (scan = 0, p = buf; scan < size; scan++,p++) {
	search = (unsigned long *)(p);
	if (*search == multiboot_sign) {
	    printf("Gotta! Offset = %lx(%lu)\nAligned = %s",scan,scan, scan == (scan & ~0x00000003) ? "Yes" : "No" );
	    free(buf);
	    exit(1);	    
	}
    }
    printf("Not found!\n");
    exit(1);
}
