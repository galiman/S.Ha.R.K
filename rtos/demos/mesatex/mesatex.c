/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
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

#include <GL/osmesa.h>
#include <GL/glut.h>

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <kernel/log.h>
#include <kernel/kern.h>

#include <drivers/shark_fb26.h>
#include <drivers/shark_keyb26.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define WIDTH 640
#define HEIGHT 430
#define BYTES_PP 2 //BytesPerPixel

OSMesaContext ctx;

static GLuint TexObj[2];
static GLfloat Angle = 0.0f;
static GLboolean UseObj = GL_FALSE;

extern void *video_memory;

#if defined(GL_VERSION_1_1) || defined(GL_VERSION_1_2)
#  define TEXTURE_OBJECT 1
#elif defined(GL_EXT_texture_object)
#  define TEXTURE_OBJECT 1
#  define glBindTexture(A,B)     glBindTextureEXT(A,B)
#  define glGenTextures(A,B)     glGenTexturesEXT(A,B)
#  define glDeleteTextures(A,B)  glDeleteTexturesEXT(A,B)
#endif

unsigned char *rgb_565_buf = NULL; //RGB 16 bpp Buffer
unsigned char *video_buf = NULL; //Video Buffer

unsigned long int RGB565MEM = WIDTH * HEIGHT * BYTES_PP; // Total video mem

unsigned long int PERIOD_REFRESH = 30000;
unsigned long int PERIOD_DISEGNA = 30000;

unsigned long int WCET_REFRESH, WCET_DISEGNA;

TASK refesh(void);
TASK disegna(void);

PID refresh_PID, disegna_PID;

static void draw( void )
{
   glDepthFunc(GL_EQUAL);
   /*   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );*/
   glClear( GL_COLOR_BUFFER_BIT );

   glColor3f( 1.0, 1.0, 1.0 );

   /* draw first polygon */
   glPushMatrix();
   glTranslatef( -1.0, 0.0, 0.0 );
   glRotatef( Angle, 0.0, 0.0, 1.0 );
   if (UseObj) {
#ifdef TEXTURE_OBJECT
      glBindTexture( GL_TEXTURE_2D, TexObj[0] );
#endif
   }
   else {
      glCallList( TexObj[0] );
   }
   glBegin( GL_POLYGON );
   glTexCoord2f( 0.0, 0.0 );   glVertex2f( -1.0, -1.0 );
   glTexCoord2f( 1.0, 0.0 );   glVertex2f(  1.0, -1.0 );
   glTexCoord2f( 1.0, 1.0 );   glVertex2f(  1.0,  1.0 );
   glTexCoord2f( 0.0, 1.0 );   glVertex2f( -1.0,  1.0 );
   glEnd();
   glPopMatrix();

   /* draw second polygon */
   glPushMatrix();
   glTranslatef( 1.0, 0.0, 0.0 );
   glRotatef( Angle-90.0, 0.0, 1.0, 0.0 );
   if (UseObj) {
#ifdef TEXTURE_OBJECT
      glBindTexture( GL_TEXTURE_2D, TexObj[1] );
#endif
   }
   else {
      glCallList( TexObj[1] );
   }
   glBegin( GL_POLYGON );
   glTexCoord2f( 0.0, 0.0 );   glVertex2f( -1.0, -1.0 );
   glTexCoord2f( 1.0, 0.0 );   glVertex2f(  1.0, -1.0 );
   glTexCoord2f( 1.0, 1.0 );   glVertex2f(  1.0,  1.0 );
   glTexCoord2f( 0.0, 1.0 );   glVertex2f( -1.0,  1.0 );
   glEnd();
   glPopMatrix();

}

static void gl_init()
{

   static int twidth=8, theight=8;
   static GLubyte tex1[] = {
     0, 0, 0, 1, 1, 1, 0, 0,
     0, 0, 1, 0, 0, 0, 0, 0,
     0, 0, 1, 0, 0, 0, 0, 0,
     0, 0, 1, 0, 0, 0, 0, 0,
     0, 0, 1, 0, 0, 0, 0, 0,
     0, 0, 1, 0, 0, 0, 0, 0,
     0, 0, 1, 0, 0, 0, 0, 0,
     0, 0, 0, 1, 1, 1, 0, 0 };

   static GLubyte tex2[] = {
     0, 0, 0, 2, 2, 2, 0, 0,
     0, 0, 2, 0, 0, 0, 0, 0,
     0, 0, 2, 0, 0, 0, 0, 0,
     0, 0, 0, 2, 0, 0, 0, 0,
     0, 0, 0, 0, 2, 0, 0, 0,
     0, 0, 0, 0, 0, 2, 0, 0,
     0, 0, 0, 0, 0, 2, 0, 0,
     0, 0, 2, 2, 2, 0, 0, 0 };

   GLubyte tex[64][3];
   GLint i, j;

    //Create the OSMesa Context
   ctx = OSMesaCreateContext(OSMESA_RGB_565, NULL);

   //Make Current Context
   OSMesaMakeCurrent(ctx, rgb_565_buf, GL_UNSIGNED_SHORT_5_6_5, WIDTH, HEIGHT);

   UseObj = GL_TRUE;

   glDisable( GL_DITHER );

   /* Setup texturing */
   glEnable( GL_TEXTURE_2D );
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
   glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );

   /* generate texture object IDs */
   if (UseObj) {
#ifdef TEXTURE_OBJECT
      glGenTextures( 2, TexObj );
#endif
   }
   else {
      TexObj[0] = glGenLists(2);
      TexObj[1] = TexObj[0]+1;
   }

   /* setup first texture object */
   if (UseObj) {
#ifdef TEXTURE_OBJECT
      glBindTexture( GL_TEXTURE_2D, TexObj[0] );
      assert(glIsTexture(TexObj[0]));
#endif
   }
   else {
      glNewList( TexObj[0], GL_COMPILE );
   }
   /* red on white */
   for (i=0;i<theight;i++) {
      for (j=0;j<twidth;j++) {
         int p = i*twidth+j;
         if (tex1[(theight-i-1)*twidth+j]) {
            tex[p][0] = 255;   tex[p][1] = 0;     tex[p][2] = 0;
         }
         else {
            tex[p][0] = 255;   tex[p][1] = 255;   tex[p][2] = 255;
         }
      }
   }

   glTexImage2D( GL_TEXTURE_2D, 0, 3, twidth, theight, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, tex );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
   if (!UseObj) {
      glEndList();
   }
   /* end of texture object */

   /* setup second texture object */
   if (UseObj) {
#ifdef TEXTURE_OBJECT
      glBindTexture( GL_TEXTURE_2D, TexObj[1] );
      assert(glIsTexture(TexObj[1]));
#endif
      assert(!glIsTexture(TexObj[1] + 999));
   }
   else {
      glNewList( TexObj[1], GL_COMPILE );
   }
   /* green on blue */
   for (i=0;i<theight;i++) {
      for (j=0;j<twidth;j++) {
         int p = i*twidth+j;
         if (tex2[(theight-i-1)*twidth+j]) {
            tex[p][0] = 0;     tex[p][1] = 255;   tex[p][2] = 0;
         }
         else {
            tex[p][0] = 0;     tex[p][1] = 0;     tex[p][2] = 255;
         }
      }
   }
   glTexImage2D( GL_TEXTURE_2D, 0, 3, twidth, theight, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, tex );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
   if (!UseObj) {
      glEndList();
   }
   /* end texture object */

   glViewport(0, 0, (GLint)WIDTH, (GLint)HEIGHT);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   /*   glOrtho( -3.0, 3.0, -3.0, 3.0, -10.0, 10.0 );*/
   glFrustum( -2.0, 2.0, 2.0, -2.0, 6.0, 20.0 );
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef( 0.0, 0.0, -8.0 );

}

void program_end(void *arg)
{
  
  OSMesaDestroyContext(ctx);
  free(rgb_565_buf);

  exit(0);

}

void program_key_end(KEY_EVT *k)
{

  exit(0);

}

TASK refresh(void)
{

  while(1) {

    memcpy((video_buf+40*WIDTH*2), rgb_565_buf, RGB565MEM);
    task_endcycle();

  }

  exit(0);

}


TASK disegna(void)
{

  char text[100];
  TIME disegna_TIME, refresh_TIME;
  
  while(1) {
   
    jet_gettable(refresh_PID, &refresh_TIME, 1);
    jet_gettable(disegna_PID, &disegna_TIME, 1);

    Angle += 2.0;

    draw();
 
    sprintf(text,"Hard Task Refresh PER:%6d us EX:%6d us",(int)PERIOD_REFRESH,(int)refresh_TIME);
    grx_text(text,10,5,rgb16(0,0,255),0);    
    sprintf(text,"Hard Task Draw    PER:%6d us EX:%6d us",(int)PERIOD_DISEGNA,(int)disegna_TIME);
    grx_text(text,10,15,rgb16(0,0,255),0);

    task_endcycle();

  }

  exit(0);

}

int main (int argc, char *argv[])
{
    
    HARD_TASK_MODEL ht_refresh, ht_disegna;

    clear();
    
    WCET_REFRESH =((long int) PERIOD_REFRESH * (0.45));
    WCET_DISEGNA =((long int) PERIOD_DISEGNA * (0.45));

    hard_task_default_model(ht_refresh);
    hard_task_def_wcet(ht_refresh,WCET_REFRESH);
    hard_task_def_mit(ht_refresh,PERIOD_REFRESH);
    hard_task_def_usemath(ht_refresh);
    hard_task_def_group(ht_refresh,1);
    hard_task_def_ctrl_jet(ht_refresh);

    refresh_PID = task_create("refresh", refresh, &ht_refresh, NULL);
    if (refresh_PID == -1) {
      exit(4);
    }

    hard_task_default_model(ht_disegna);
    hard_task_def_mit(ht_disegna,PERIOD_DISEGNA);
    hard_task_def_wcet(ht_disegna,WCET_DISEGNA);
    hard_task_def_group(ht_disegna,1);
    hard_task_def_ctrl_jet(ht_disegna);
    hard_task_def_usemath(ht_disegna);
    hard_task_def_stack(ht_disegna,30000);
    
    disegna_PID = task_create("disegna", disegna, &ht_disegna, NULL);
    if (disegna_PID == -1) {
      exit(4);
    }

    {
			KEY_EVT k;
			k.flag = CNTL_BIT;
			k.scan = KEY_C;
			k.ascii = 'c';
			k.status = KEY_PRESSED;
			keyb_hook(k, program_key_end, FALSE);
		
			k.flag = CNTR_BIT;
			k.status = KEY_PRESSED;
			keyb_hook(k, program_key_end, FALSE);
    }

    rgb_565_buf = malloc(RGB565MEM);
        
    gl_init();
    
    video_buf = (unsigned char *)video_memory;
    //video_buf = (unsigned char *)malloc(640*480*2);

    memset(rgb_565_buf, 0, RGB565MEM);

    group_activate(1);

    return 0;
        
}
