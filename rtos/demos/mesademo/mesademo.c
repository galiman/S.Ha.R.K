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

#include <kernel/log.h>
#include <kernel/kern.h>
#include <math.h>
#include <stdlib.h>

#include <drivers/shark_fb26.h>
#include <drivers/shark_keyb26.h>
                                                                                                                             
#include <GL/osmesa.h>
#include <GL/glut.h>
                                                                                                                             
#ifndef M_PI
#define M_PI 3.14159265
#endif

#define WIDTH 640
#define HEIGHT 430
#define BYTES_PP 2 //BytesPerPixel

OSMesaContext ctx;

extern void *video_memory;

unsigned char *rgb_565_buf = NULL; //RGB 16 bpp Buffer
unsigned char *video_buf = NULL; //Video Buffer

unsigned long int VMEMLONG = WIDTH * HEIGHT * BYTES_PP / 4; // Used by copy_videomem_16to16
unsigned long int RGB565MEM = WIDTH * HEIGHT * BYTES_PP; // Total video mem

unsigned long int PERIOD_REFRESH = 100000;
unsigned long int PERIOD_DISEGNA = 100000;

unsigned long int WCET_REFRESH, WCET_DISEGNA;

TASK refesh(void);
TASK disegna(void);

PID refresh_PID, disegna_PID;

static void
gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
  GLint teeth, GLfloat tooth_depth)
{
  GLint i;
  GLfloat r0, r1, r2;
  GLfloat angle, da;
  GLfloat u, v, len;

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2.0;
  r2 = outer_radius + tooth_depth / 2.0;

  da = 2.0 * M_PI / teeth / 4.0;

  glShadeModel(GL_FLAT);

  glNormal3f(0.0, 0.0, 1.0);

  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    if (i < teeth) {
      glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
      glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    }
  }
  glEnd();

  glBegin(GL_QUADS);
  da = 2.0 * M_PI / teeth / 4.0;
  for (i = 0; i < teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;

    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
  }
  glEnd();

  glNormal3f(0.0, 0.0, -1.0);

  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    if (i < teeth) {
      glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
      glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    }
  }
  glEnd();

  glBegin(GL_QUADS);
  da = 2.0 * M_PI / teeth / 4.0;
  for (i = 0; i < teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;

    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
  }
  glEnd();

  glBegin(GL_QUAD_STRIP);
  for (i = 0; i < teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;

    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    u = r2 * cos(angle + da) - r1 * cos(angle);
    v = r2 * sin(angle + da) - r1 * sin(angle);
    len = sqrt(u * u + v * v);
    u /= len;
    v /= len;
    glNormal3f(v, -u, 0.0);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
    glNormal3f(cos(angle), sin(angle), 0.0);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
    u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
    v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
    glNormal3f(v, -u, 0.0);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
    glNormal3f(cos(angle), sin(angle), 0.0);
  }

  glVertex3f(r1 * cos(0), r1 * sin(0), width * 0.5);
  glVertex3f(r1 * cos(0), r1 * sin(0), -width * 0.5);

  glEnd();

  glShadeModel(GL_SMOOTH);

  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = i * 2.0 * M_PI / teeth;
    glNormal3f(-cos(angle), -sin(angle), 0.0);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
  }
  glEnd();

}

static GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 0.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;

static void draw(void)
{ 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
    glRotatef(view_rotx, 1.0, 0.0, 0.0);
    glRotatef(view_roty, 0.0, 1.0, 0.0);
    glRotatef(view_rotz, 0.0, 0.0, 1.0);

    glPushMatrix();
      glTranslatef(-3.0, -2.0, 0.0);
      glRotatef(angle, 0.0, 0.0, 1.0);
      glCallList(gear1);
    glPopMatrix();

    glPushMatrix();
      glTranslatef(3.1, -2.0, 0.0);
      glRotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
      glCallList(gear2);
    glPopMatrix();

    glPushMatrix();
      glTranslatef(-3.1, 4.2, 0.0);
      glRotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
      glCallList(gear3);
    glPopMatrix();

  glPopMatrix();

  glFinish();

}

static void gl_init()
{

  static GLfloat red[4] = {1.0, 0.0, 0.0, 1.0};
  static GLfloat green[4] = {0.0, 0.8, 0.2, 1.0};
  static GLfloat blue[4] = {0.2, 0.2, 1.0, 1.0};
  static GLfloat pos[4] = {5.0, 5.0, 10.0, 1.0};
  static GLfloat h = (GLfloat) HEIGHT / (GLfloat) WIDTH;

  //Create the OSMesa Context
  ctx = OSMesaCreateContext(OSMESA_RGB_565, NULL);

  //Make Current Context
  OSMesaMakeCurrent(ctx, rgb_565_buf, GL_UNSIGNED_SHORT_5_6_5, WIDTH, HEIGHT);
  
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
    
  /* make the gears */
  gear1 = glGenLists(1);
  glNewList(gear1, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  gear(1.0, 4.0, 1.0, 20, 0.7);
  glEndList();

  gear2 = glGenLists(1);
  glNewList(gear2, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
  gear(0.5, 2.0, 2.0, 10, 0.7);
  glEndList();

  gear3 = glGenLists(1);
  glNewList(gear3, GL_COMPILE);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
  gear(1.3, 2.0, 0.5, 10, 0.7);
  glEndList();
  
  glEnable(GL_NORMALIZE);

  glViewport(0, 0, (GLint) WIDTH, (GLint) HEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -40.0);

}

void program_key_end(KEY_EVT *k)
{

  exit(0);

}

TASK refresh(void)
{

  while(1) {

    task_testcancel();
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

    task_testcancel();
   
    jet_gettable(refresh_PID, &refresh_TIME, 1);
    jet_gettable(disegna_PID, &disegna_TIME, 1);

    angle += 1.0;

    draw();
 
    sprintf(text,"Hard Task Refresh PER:%6d us EX:%6d us",(int)PERIOD_REFRESH,(int)refresh_TIME);
    grx_text(text,10,5,color16(0,0,255),0);    
    sprintf(text,"Hard Task Draw    PER:%6d us EX:%6d us",(int)PERIOD_DISEGNA,(int)disegna_TIME);
    grx_text(text,10,15,color16(0,0,255),0);

    task_endcycle();

  }

  exit(0);

}

int main (int argc, char *argv[])
{
    
    HARD_TASK_MODEL ht_refresh, ht_disegna;

    WCET_REFRESH =((long int) PERIOD_REFRESH * (0.30));
    WCET_DISEGNA =((long int) PERIOD_DISEGNA * (0.55));

    hard_task_default_model(ht_refresh);
    hard_task_def_wcet(ht_refresh,WCET_REFRESH);
    hard_task_def_mit(ht_refresh,PERIOD_REFRESH);
    hard_task_def_usemath(ht_refresh);
    hard_task_def_group(ht_refresh,1);
    hard_task_def_ctrl_jet(ht_refresh);

    refresh_PID = task_create("refresh", refresh, &ht_refresh, NULL);
    if (refresh_PID == -1) {
      exit(0);
    }

    hard_task_default_model(ht_disegna);
    hard_task_def_mit(ht_disegna,PERIOD_DISEGNA);
    hard_task_def_wcet(ht_disegna,WCET_DISEGNA);
    hard_task_def_group(ht_disegna,1);
    hard_task_def_ctrl_jet(ht_disegna);
    hard_task_def_usemath(ht_disegna);
    hard_task_def_stack(ht_disegna,30000); //VERY IMPORTANT FOR glCallList !!

    disegna_PID = task_create("disegna", disegna, &ht_disegna, NULL);
    if (disegna_PID == -1) {
      exit(0);
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
    video_buf = (unsigned char *)video_memory;

    gl_init();
    
    memset(rgb_565_buf, 0, RGB565MEM);

    group_activate(1);

    return 0;
        
}
