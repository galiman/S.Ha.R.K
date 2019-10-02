
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

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  char *symbol[100];
  int i, j, found, size;
  FILE *f;
  char buff[20], *profile = "Profile:", *str, c;
  
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(-1);
  }

  f = fopen(argv[1], "r");
  if (f == NULL) {
    printf("Input file: %s\n", argv[1]);
    perror("error opening file");
    exit(-1);
  }
  
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  printf("File Name: %s\n", argv[1]);
  printf("File Size: %d\n", size);
  found = 0;
  for (i = 0; i < size; i++) {
    fseek(f, i, SEEK_SET);
    fread(buff, 8, 1, f);
    if (memcmp(buff, profile, 8) == 0) {
      /* Found!!! */
      str = malloc(20);
      symbol[found++] = str;
      j = 0;
      do {
        c = fgetc(f);
        if ((j != 0) || (c != 0)) {
	  str[j] = c;
	  j++;
	} else {
	  c = 1;
	}
      } while (c != 0);
    }
    i++;
  }
  fclose(f);

  printf("%d symbols found:\n", found);
  for(i = 0; i < found; i++) {
    printf("\t%s\n", symbol[i]);
  }
  exit(0);
}
