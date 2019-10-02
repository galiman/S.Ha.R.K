/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *    Giacomo Guidi      <giacomo@gandalf.sssup.it>
 *    Tullio Facchinetti <tullio.facchinetti@unipv.it>
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 * http://robot.unipv.it
 */

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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define READ_BUFFER 2000
#define DELTA_BUFFER 100

int main(int argc, char *argv[])
{

  char buffer[READ_BUFFER+DELTA_BUFFER]; 
  void *p, *last;
  int n,delta,size;
  
  unsigned long long tsc_value;

  unsigned long long ev = 0;

  FILE *input_file; 
  
  if (argc < 2) {
    printf("%s: Enter the input file name [%s filename]\n",argv[0],argv[0]);
    exit(1);
  }

  input_file = fopen(argv[1],"rb");

  last = buffer + READ_BUFFER;

  while(!feof(input_file)) {
 
    //move remaining byte
    delta = (unsigned int)(buffer) + READ_BUFFER - (unsigned int)(last);
    if (delta > 0) memcpy(buffer,last,delta);    

    n = fread(buffer+delta,1,READ_BUFFER+10,input_file);
    fseek(input_file,-(delta+10),SEEK_CUR);

    p = buffer;

    while ((unsigned int)(p) + 16 <= (unsigned int)(buffer + READ_BUFFER) && 
	   (unsigned int)(p) + 16 <= (unsigned int)(buffer + n + delta)) {

      printf("%08d Type = %02x ",(unsigned int)ev,*(unsigned short int *)(p));
 
      tsc_value = (unsigned long long)(*(unsigned int *)(p+4)) << 32;
      tsc_value += (unsigned long long)(*(unsigned int *)(p+8));
      
      printf("TSC = %llu (%08x:%08x)",tsc_value, *(unsigned int *)(p+4),*(unsigned int *)(p+8));

      size = 16;

      printf(" Par1 = %11d",*(unsigned short int *)(p+2));
      printf(" Par2 = %11d\n",*(unsigned int *)(p+12));

      ev++;
      
      p += 16;

      if ((unsigned int)(p) + 10 > (unsigned int)(buffer + n + delta)) break; 

      last = p;
 
    }

    if ((unsigned int)(p) + 10 > (unsigned int)(buffer + n + delta)) break;
      
  }

  fclose(input_file);

  return 0;

}

