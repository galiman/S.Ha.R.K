----------------------------------
Jumping Balls demo

by

Paolo Gai 1999-2001 - pj@sssup.it

----------------------------------

This demo was created to show some S.Ha.R.K. functionalities in the course
of Informatica Industriale , University of Pavia, Italy.

The demo is composed by an application (derived by the template application
distributed on the web site) and two init files.

The demo is composed by:

MAKEFILE     The makefile used to compile the application;
             demo is the rule to compile the application with a CBS scheduler
             demo2 is the rule to compile the application with a RR scheduler
README.TXT   This file
DEMO.H       Some constants used into the demo
ECP.C, ERN.C, RRP.C, RRN.C  Various initfiles
INITFIL2.C   The EDF initfile
INITFILE.C   A makefile that cover either EDF+CBS and RR
BALL.C       The Jumping balls part of the demo
DEMO.C       The main() function and some other utility function
JETCTRL.C    The JET part of the demo

The demo works as follows:
- It works at 640x480 16 bit colors
- on the left, there is the jumping ball arena, on the rigth there is the
  statistics about the tasks into the system.

- the tasks are guaranteed using CBS and EDF. The wcet and mean execution
  time on my portable after a few minutes are (us):

  JetCtrl   7400 max    7500 CBS met
  JetDummy   135 max     200 CBS met
  JetSlide  2100 max    2100 CBS met
  Balls      276 max     380 EDF wcet for hard ball,100 CBS met for soft ones

  The system should go overloaded with 40 soft balls.

- The idea is the following:
  - first, an edf guaranteed ball is created.
  - then, create a set of soft ball using space. since their met is < than the
    real met, they posticipate the deadlines.
  - if they are killed, they remain for some seconds in the zombie state
  - if a set of soft ball arde created (i.e., 10), and after a while all the
    others are created, the bandwidth is fully used, and the task posticipate
    their deadlines. the first set of tasks stops jumping when the bandwidth
    is full utilized by the newest balls until all the tasks have similar
    deadlines.
  - Note on the left the slides that represents the relative ratio
    between the tasks.
  - Note that in overload conditions the EDF task is still guaranteed
  - Note that PI is used with EDF, also if no theory says that it is good:
    - S.Ha.R.K. allows that, it is the user that have to choose if that is
      a non-sense
    - PI is independent from the implemnentation of the scheduling modules
  - if the second init file is used, RR is used instead of EDF+CBS.
  - In overload condition RR perform differently from EDF+CBS, giving to
    each task an equal fraction of bandwidth

  - note also:
    - the redefinition of the standard exception handler
    - the redefinition of the keys and the initialization of the keyboard
    - the myend exit function
    - the main() that terminates
    - the two parts (ball and jet) can be excluded using a #define

If You have any question, please contact the author...

Update (2003/12/19):
Just some more notes...
I usually show the four demos in the following order:

(first of all, tune the demo depending on how fast your notebook is; see demo.h)

ECP - EDF with CBS and PI... the white EDF ball still work also in overload; CBS balls just slow down but they still get their guaranteed bandwidth

RRP - Round Robin with Priority inheritance ... the EDF ball is at the same level of the others, all slow down...

RRN - 2 levels of Round Robin, without Priority inheritance ... when
there is overload, the white ball works ok, but the other have an impredicible
behavior (they execute many instances in one RR period)

ERN - EDF, RR, no priority inheritance - the white ball misses his deadline
just when there is an overload (all the other balls are queued on the same
semaphore, so the blocking time increase, and when there is overload the EDF
task misses also if it has not consumed his bandwidth). Note that it happens
also if the EDF task consume just a few microseconds... Real time does not
means "fast" :-)
