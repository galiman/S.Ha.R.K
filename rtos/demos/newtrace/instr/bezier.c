
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

#include <kernel/kern.h>
#include <tracer.h>

#define PWCET_Automatic_Ipoint(a) TRACER_LOGEVENT(FTrace_EVT_ipoint,(a),0);

/* here are the bezier curves defined
format:
{startpoint, endpoint, controlpoint1, controlpoint}
.
.
. */
int curveno = 20;
int data[20][4][2] = {
	{{123,321},{ 23,432},{345,120},{123, 98}},
	{{567, 31},{ 23, 24},{ 35,421},{123,398}},
	{{  0, 21},{623,532},{145,323},{153, 68}},
	{{253,321},{ 23,432},{745,525},{423,198}},
	{{123,456},{ 23,482},{  0,123},{123,128}},
	{{322, 21},{223,232},{ 45,224},{123,98}},
	{{423, 32},{123,332},{144,329},{123,98}},
	{{276, 35},{ 23,492},{476,423},{123,98}},
	{{783,121},{723,139},{ 78,528},{123,98}},
	{{723,221},{623, 98},{734,683},{123,98}},
	{{ 60,421},{523,132},{364,753},{123,98}},
	{{100,521},{423,432},{633,623},{123,98}},
	{{ 23,371},{323,599},{343,533},{123,98}},
	{{123,123},{123,  0},{343,403},{123,98}},
	{{253,321},{490, 32},{347,393},{123,98}},
	{{ 68,321},{260,272},{674,283},{123,98}},
	{{500,321},{245,373},{ 98,173},{123,98}},
	{{423,321},{198,532},{306, 63},{123,98}},
	{{197,321},{203,432},{307,443},{123,98}},
	{{143,321},{293,132},{334,393},{123,98}}
};
#define STEPWIDTH 0.01 /* draws 1/STEPWIDTH +1 points between SP and EP */
#define XSIZE 800
#define YSIZE 600

char screen[YSIZE][XSIZE];
int xco[4],yco[4];

int init()
{
	int y,x;

	/*initialize screen*/
	for (x = 0;x < XSIZE;x++) {
		for (y = 0;y < YSIZE;y++)  {
			screen[y][x] = 255;  /*white*/
		}
	}

	return 0;

}

void rand_init()
{
	int i,j,x,y;
	for (i=0;i<20;i++)
		for (j=0;j<4;j++) {
			x=rand()%800;
			y=rand()%600;
			data[i][j][0]=x;
			data[i][j][1]=y;
		}
}

int bezier()
{

  int i,y,x;
  float k;

  init();

	for (i = 0;i < curveno;i++)  {
		xco[3] = data[i][0][0];
		yco[3] = data[i][0][1];
		xco[2] = 3*(data[i][2][0] - data[i][0][0]);
		yco[2] = 3*(data[i][2][1] - data[i][0][1]);
		xco[1] = 3*(data[i][3][0] - data[i][2][0]) - xco[2];
		yco[1] = 3*(data[i][3][1] - data[i][2][1]) - yco[2];
		xco[0] = data[i][1][0] - data[i][0][0] - xco[2]- xco[1];
		yco[0] = data[i][1][1] - data[i][0][1] - yco[2]- yco[1];

		/*scan curve for t = 0 to t = 1 with STEPWIDTH*/
		for (k = 0;k <=1;k+=STEPWIDTH)  {			/* PAN_FIXED_LOOP PAN_VARPATH */
			x = (int)(((float)xco[0]*k*k*k)+((float)xco[1]*k*k)+((float)xco[2]*k)+(float)xco[3]);
			y = (int)(((float)yco[0]*k*k*k)+((float)yco[1]*k*k)+((float)yco[2]*k)+(float)yco[3]);
			if ((x < XSIZE)&&(x > 0)&&(y < YSIZE)&&(y > 0))  {
				/*write dot to screen*/
				screen[y][x] = 0;  /*black*/
			}
		}
	}

	return 0;
}

int instrumented_function() {

	rand_init();

	bezier();

	return 0;
}

