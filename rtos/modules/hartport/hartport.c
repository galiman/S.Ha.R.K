/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: hartport.c,v 1.1 2005/02/25 10:53:41 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:53:41 $
 ------------

 This file contains the Hartik 3.3.1 Port functions

 Author:      Giuseppe Lipari
 Date:        2/7/96

 File: 	Port.C (renamed to hartport.c)
 Revision:    1.4

**/

/*
 * Copyright (C) 2000 Paolo Gai
 *
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


#include <hartport/hartport/hartport.h>
#include <kernel/config.h>
#include <ll/ll.h>
#include <ll/string.h>
#include <ll/stdio.h>
#include <kernel/model.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/descr.h>
#include <errno.h>
#include <kernel/var.h>
#include <kernel/func.h>


static sem_t hash_mutex;

#define __PORT_DBG__

struct hash_port {
	char	name[MAX_PORT_NAME];
	PID	port_index;
	BYTE 	blocked;
	sem_t	sem;
	BYTE	valid;
	int	port_int;
};

struct port_ker {
	BYTE	valid;
	int 	next;
	WORD 	dim_block;
	BYTE	*mem_block;
	BYTE	*head;
	BYTE	*tail;
	BYTE	type;
	sem_t	mutex;
	sem_t	pieno;
	sem_t	vuoto;
};

struct port_com {
	BYTE	valid;
	int 	next;
	PID 	proc_id;
	BYTE	access;
	int	port_index;
	WORD	dim_mes;
	int	h_index;
};
	
struct hash_port	htable[MAX_HASH_ENTRY];
struct port_ker		port_des[MAX_PORT];
struct port_com		port_int[MAX_PORT_INT];
int	freeportdes;
int	freeportint;

static int port_installed = 0;

 
/*----------------------------------------------------------------------*/
/* port_init() : inizializza le strutture delle porte; da chiamare 	*/
/* dentro __hartik_register_levels__().	                                */
/*----------------------------------------------------------------------*/
void HARTPORT_init(void)
{
    int i;

    if (port_installed)
      return;
    port_installed = 1;

    /* Init hash table			*/
    for (i = 0; i < MAX_HASH_ENTRY; i++) {
	htable[i].valid = FALSE;
	htable[i].port_int = NIL;
	htable[i].blocked = 0;
    }

    /* mutex sem on the hash table		*/
    sem_init(&hash_mutex, 0, 1);

    /* init the port descriptor table 	*/
    for (i = 0; i < MAX_PORT - 1; i++) {
	port_des[i].next = i+1;
	port_des[i].valid = FALSE;
    }
    port_des[MAX_PORT - 1].next = NIL;
    port_des[MAX_PORT - 1].valid = FALSE;
    freeportdes = 0;

    /* Init the port interface table	*/
    for (i = 0; i < MAX_PORT_INT - 1; i++) {
	port_int[i].next = i+1;
	port_int[i].valid = FALSE;
    }
    port_int[MAX_PORT_INT - 1].next = NIL;
    port_int[MAX_PORT_INT - 1].valid = FALSE;
//    for (i = PORT_NO_MORE_DESCR; i <= PORT_UNVALID_DESCR; i++)
//        exc_set(i,port_exception);
    freeportint = 0;
}


/*----------------------------------------------------------------------*/
/* hash_fun() : address hash table				 	*/
/*----------------------------------------------------------------------*/
static int hash_fun(char *name)
{
    return (*name % MAX_HASH_ENTRY);
}


/*----------------------------------------------------------------------*/
/* getfreedes : restituisce l'indice di un descrittore di porta libero	*/
/*----------------------------------------------------------------------*/
static int getfreedes(void)
{
    int p;
    SYS_FLAGS f;

    f = kern_fsave();
    if (freeportdes == NIL) {
	errno = EPORT_NO_MORE_DESCR;
	kern_frestore(f);
	return -1;
    }
    p = freeportdes;
    freeportdes = port_des[p].next;
    kern_frestore(f);
    return(p);
}	  

/*----------------------------------------------------------------------*/
/* ungetdes() : mette il descrittore tra quelli disponibile	 	*/
/*----------------------------------------------------------------------*/
static void ungetdes(int pd)
{
    SYS_FLAGS f;

    f = kern_fsave();
    port_des[pd].next = freeportdes;
    freeportdes = pd;
    kern_frestore(f);
}

/*----------------------------------------------------------------------*/
/* get freeint : restituisce una interfaccia di porta libera	 	*/
/*----------------------------------------------------------------------*/
static int getfreeint(void)
{
    int p;
    SYS_FLAGS f;                                                                                                                       
    f = kern_fsave();
    if (freeportint == NIL) {
	errno = EPORT_NO_MORE_INTERF;
	kern_frestore(f);
	return -1;
    }
    p = freeportint;
    freeportint = port_int[p].next;
    kern_frestore(f);
    return(p);
}	

/*----------------------------------------------------------------------*/
/* ungetint : rende disponibile l'interfaccia di porta specificata	*/
/*----------------------------------------------------------------------*/
static void ungetint(int pi)
{
    SYS_FLAGS f;

    f = kern_fsave();
    port_int[pi].next = freeportint;
    freeportint = pi;
    kern_frestore(f);
}

/*----------------------------------------------------------------------*/
/* port_create(): Apre la porta specificata dalla stringa, effettuando	*/
/* automaticamente il collegamento					*/
/* WARNING : La funzione e' bloccante per la mutua esclusione sulla 	*/
/* hash table	 						*/
/*----------------------------------------------------------------------*/
PORT port_create(char *name, int dim_mes, int num_mes, BYTE type, BYTE access)
{
    int i, pd, pi;
    WORD letti = 0;
    BYTE flag = FALSE;
    SYS_FLAGS f;

/*
   Prendo un descrittore di porta.
*/
    #ifdef __PORT_DBG__
	if ((type == MAILBOX) && (access == WRITE)) {
	   errno = EPORT_INCOMPAT_MESSAGE;
	   return -1;
	}
	if ((type == STICK  ) && (access == READ )) {
	   errno = EPORT_INCOMPAT_MESSAGE;
	   return -1;
	}
    #endif
    pd = getfreedes();
    if (pd == -1) return -1;
/* 
   Devo andare in mutua esclusione sulla tabella di hash ! Poiche' questa
   viene acceduta dalle openport e dalle connect sono sicuro che una
   connect non puo' interrompere una openport.
*/
    sem_wait(&hash_mutex);
/*
   Scorro la tabella di hash fino a trovare un'entrata libera, oppure 
   una entrata occupata in precedenza da una connect che ci si e' bloccata
   sopra (blocked > 0). Se ne trovo una gia' aperta da un'altra openport
   esco con errore. Lo stesso se sono state occupate tutte le entrate della
   tabella (tramite la var. letti);
*/
    i = hash_fun(name);
    while (!flag) {
	if (htable[i].valid == FALSE) flag = TRUE;
	else {
	    if (strcmp(htable[i].name,name) == 0) {
		if (htable[i].blocked == 0) {
		    errno = EPORT_ALREADY_OPEN;
		    sem_post(&hash_mutex);
		    return -1;
		}
		else flag = TRUE;
	    }
	    else {
		i = (i+1) % MAX_HASH_ENTRY;
		letti++;
	    }
	}
	if (letti > MAX_HASH_ENTRY-1) {
	    errno = EPORT_NO_MORE_HASHENTRY;
	    sem_post(&hash_mutex);
	    return -1;
	}

    }
    htable[i].valid = TRUE;
    strcpy(htable[i].name, name);

    htable[i].port_index = pd;
/* 
   A questo punto inizializzo tutta la struttura del descrittore di porta
*/
    if (type == STICK) 	port_des[pd].dim_block = dim_mes;
    else port_des[pd].dim_block = dim_mes * num_mes;

    f = kern_fsave();
    port_des[pd].mem_block = kern_alloc(port_des[pd].dim_block);
    kern_frestore(f);
    if (port_des[pd].mem_block == NULL) {
	errno = EPORT_2_CONNECT;
	sem_post(&hash_mutex);
	return -1;
    }

    port_des[pd].head = port_des[pd].tail = port_des[pd].mem_block;

    sem_init(&port_des[pd].mutex,0,1);
    sem_init(&port_des[pd].pieno,0,port_des[pd].dim_block);
    sem_init(&port_des[pd].vuoto,0,0);
    port_des[pd].type = type;
/*
   Prendo e inizializzo la struttura dell'interfaccia di porta verso il
   processo (e' la varibile pi quella che restituisco)
*/
    pi = getfreeint();
    if (pi == -1) {
	sem_post(&hash_mutex);
	return -1;
    }
	
/*  port_int[pi].proc_id = exec_shadow; */
    port_int[pi].access = access;
    port_int[pi].port_index = pd;
    port_int[pi].dim_mes = dim_mes;
    port_int[pi].next = NIL;
    port_int[pi].h_index = i;
    port_des[pd].valid = TRUE;
    port_int[pi].valid = TRUE;
/* 
   Sblocco eventuali processi che avevano fatto la connect nella coda
   semaforica che sta nell'entrata relativa della hash table !
*/
    if (htable[i].blocked > 0) {
	sem_xpost(&htable[i].sem, htable[i].blocked);
	htable[i].blocked = 0;
	sem_destroy(&htable[i].sem);
    }
/*
   Infine libero la mutua esclusione.
*/
    sem_post(&hash_mutex);
    return (pi);
}

/*----------------------------------------------------------------------*/
/* port_connect(): collega la porta specificata dalla stringa.		*/
/* WARNING : La funzione e' bloccante per la mutua esclusione sulle 	*/
/* strutture delle porte						*/
/*----------------------------------------------------------------------*/
PORT port_connect(char *name, int dim_mes, BYTE type, BYTE access)
{
    int i, pi, pd, pn,letti = 0;
    BYTE flag = FALSE, create = FALSE;

    #ifdef __PORT_DBG__
	if ((type == MAILBOX) && (access == READ )) {
	    errno = EPORT_INCOMPAT_MESSAGE;
	    return -1;
	}
	if ((type == STICK  ) && (access == WRITE)) {
	   errno = EPORT_INCOMPAT_MESSAGE;
	   return -1;
	}
    #endif
/*
   Per prima cosa mi prendo una interfaccia di porta e la riempio 
   parzialmente.	
*/
    pi = getfreeint();
    if (pi == -1) return -1;
/*  port_int[pi].proc_id = exec_shadow; */
    port_int[pi].access = access;
    port_int[pi].dim_mes = dim_mes;
    port_int[pi].next = NIL;
/*
   Mutua esclusione sulla tabella di hash
*/
    sem_wait(&hash_mutex);
/*
   Cerco il descrittore appropriato : Se la porta e' gia' stata aperta
   allora esco dal ciclo con flag = TRUE, create = FALSE, l'indice i che 
   punta all'entrata della tabella di hash, e con htable[i].blocked = 0;
   In tutti gli altri casi significa che la porta non e' stata aperta e 
   quindi devo bloccarmi.
*/
    i = hash_fun(name);
    while (!flag) {
	/* Devo crearla */
	if (htable[i].valid == FALSE) {
	    flag = TRUE;
	    create = TRUE;
	}
	/* l'ho trovata (ma non so ancora se e' stata aperta) */
	else if (strcmp(htable[i].name, name) == 0) flag = TRUE;
	/* scandisco l'entrata successiva */
	else {
	    i = (i+1) % MAX_HASH_ENTRY;
	    letti ++;
	}
	#ifdef __PORT_DBG__
	    /* se ho scorso tutto l'array senza trovare nulla */
	    if (letti > MAX_HASH_ENTRY) {
		errno = EPORT_NO_MORE_HASHENTRY;
		sem_post(&hash_mutex);
		return -1;
	    }
	#endif
    }
	
/*
   Se devo aspettare che venga aperta (create = TRUE) allora mi blocco in
   attesa sul semaforo della htable. Per non avere troppi semafori inutili
   in giro, me lo creo sul momento.
*/
    if (create == TRUE) {
	htable[i].valid = TRUE;
	htable[i].blocked = 1;
	strcpy(htable[i].name, name);
	sem_init(&htable[i].sem, 0, 0);
	sem_post(&hash_mutex);
	sem_xwait(&htable[i].sem,1,BLOCK);
    }
/*
   Se invece si e' gia' bloccata un'altra connect sopra, mi blocco anch'io.
   in ogni caso devo liberare la mutua esclusione sulla hash dato che ho
   gia' fatto tutto quello che dovevo fare con la hash.
*/
    else {
	if (htable[i].blocked > 0) {
	    htable[i].blocked++;
	    sem_post(&hash_mutex);
	    sem_xwait(&htable[i].sem,1,BLOCK);
	}
	else sem_post(&hash_mutex);
    }
/*
   Controlli di errore.
*/
    pd = htable[i].port_index;
    #ifdef __PORT_DBG__
	if (type != port_des[pd].type) {
	    errno = EPORT_UNSUPPORTED_ACC;
	    return -1;
	}
	if ((type == STICK) && (dim_mes != port_des[pd].dim_block)) {
	    errno = EPORT_WRONG_OP;
	    return -1;
	}
	if ((type != STICK) && (port_des[pd].dim_block % dim_mes) != 0) {
	    errno = EPORT_WRONG_OP;
	    return -1;
	}
    #endif

    sem_wait(&hash_mutex);
    pn = htable[i].port_int;
    if (pn != NIL) {
	#ifdef __PORT_DBG__
	    if (type == STREAM) {
		errno = EPORT_WRONG_TYPE;
		sem_post(&hash_mutex);
		return -1;
	    }
	    if (dim_mes != port_int[pn].dim_mes) {
		errno = EPORT_WRONG_OP;
		sem_post(&hash_mutex);
		return -1;
	    }
	#endif
	port_int[pi].next = htable[i].port_int;
	htable[i].port_int = pi;
    }
    else htable[i].port_int = pi;
    sem_post(&hash_mutex);
    port_int[pi].h_index = i;
    port_int[pi].port_index = pd;
    port_int[pi].valid = TRUE;
    return(pi);
}

/*----------------------------------------------------------------------*/
/* port_delete() : inversa di port_open, libera tutto		 	*/
/*----------------------------------------------------------------------*/
void port_delete(PORT pi)
{
    int i;
    struct port_ker *pd;
    struct port_com *pp;
    SYS_FLAGS f;

    pp = &port_int[pi];
    sem_wait(&hash_mutex);
    i = pp->h_index;
    pd = &port_des[htable[i].port_index];
    pd->valid = FALSE;
    sem_destroy(&pd->mutex);
    sem_destroy(&pd->pieno);
    sem_destroy(&pd->vuoto);
    
    f = kern_fsave();
    kern_free(pd->mem_block, pd->dim_block);
    kern_frestore(f);

    ungetdes(htable[i].port_index);
    pp->valid = FALSE;
    htable[i].port_int = pp->next;
    ungetint(pi);
    htable[i].valid = FALSE;
    sem_post(&hash_mutex);
}

/*----------------------------------------------------------------------*/
/* port_disconnect() : libera l'interfaccia di porta 		 	*/
/*----------------------------------------------------------------------*/
void port_disconnect(PORT pi)
{
    sem_wait(&hash_mutex);
    if (htable[port_int[pi].h_index].valid == TRUE)
        htable[port_int[pi].h_index].port_int = port_int[pi].next;
    port_int[pi].valid = FALSE;
    ungetint(pi);
    sem_post(&hash_mutex);
}

/*----------------------------------------------------------------------*/
/* port_send() : Invia un messaggio alla porta	 		 	*/
/*----------------------------------------------------------------------*/
WORD port_send(PORT pi, void *msg, BYTE wait)
{
    struct port_ker *pd;
    struct port_com *pp;

    pp = &(port_int[pi]);
    pd = &(port_des[pp->port_index]);
	
    #ifdef __PORT_DBG__
	if (pp->access == READ) {
          errno = EPORT_WRONG_OP;
          return -1;
        }
	if (!pd->valid) {
          errno = EPORT_INVALID_DESCR;
          return -1;
        }

    #endif

    if (pd->type == STICK) sem_wait(&pd->mutex);
    else if (pd->type == STREAM) {
            if (sem_xwait(&pd->pieno,pp->dim_mes,wait)) return(FALSE);
    }
    else {
	if (sem_xwait(&pd->pieno, pp->dim_mes,wait)) return(FALSE);
	sem_wait(&pd->mutex);
    }

    memcpy(pd->head, msg, pp->dim_mes);

    pd->head += pp->dim_mes;
    
    if (pd->head >= (pd->mem_block + pd->dim_block)) 
	pd->head -= pd->dim_block;

    if (pd->type == STICK) sem_post(&pd->mutex);
    else if (pd->type == STREAM) sem_xpost(&pd->vuoto, pp->dim_mes);
    else {
	sem_xpost(&pd->vuoto, pp->dim_mes);
	sem_post(&pd->mutex);
    }
    return(TRUE);
}

/*----------------------------------------------------------------------*/
/* port_receive() : Riceve un messaggio dalla porta 		 	*/
/*----------------------------------------------------------------------*/
WORD port_receive(PORT pi,void *msg,BYTE wait)
{
    struct port_ker *pd;
    struct port_com *pp;

    pp = &(port_int[pi]);
    pd = &(port_des[pp->port_index]);
	
    #ifdef __PORT_DBG__
	if (pp->access == WRITE) {
          errno = EPORT_WRONG_OP;
          return -1;
        }
	if (!pd->valid) {
          errno = EPORT_INVALID_DESCR;
          return -1;
        }
    #endif	

    if (pd->type == STICK) sem_wait(&pd->mutex);
    else if (pd->type == STREAM) {
    	 if (sem_xwait(&pd->vuoto,pp->dim_mes,wait)) return(FALSE);
    }
    else {
	if (sem_xwait(&pd->vuoto,pp->dim_mes,wait)) return(FALSE);
	sem_wait(&pd->mutex);
    }

    memcpy(msg, pd->tail, pp->dim_mes);

    pd->tail += pp->dim_mes;
    if (pd->tail >= (pd->mem_block + pd->dim_block)) 
	pd->tail -= pd->dim_block;

    if (pd->type == STICK) sem_post(&pd->mutex);
    else if (pd->type == STREAM) sem_xpost(&pd->pieno, pp->dim_mes);
    else {
	sem_xpost(&pd->pieno, pp->dim_mes);
	sem_post(&pd->mutex);
    }
    return(TRUE);
}

#ifdef __PORT_DBG__

void print_port(void)
{
    int i;
    struct port_ker *pd;
    struct port_com *pp;
	
/*
	kern_printf("Hash Table :\n");
  	for (i=0; i<MAX_HASH_ENTRY; i++)
  		kern_printf("%d\tvl: %d\tbl: %d\tpd: %d\t%s\n", i,
  			htable[i].valid, htable[i].blocked, htable[i].port_index,
  			htable[i].name);
*/
	kern_printf("Port des :\n");
	kern_printf("Free port des : %d\n", freeportdes);
	for (i=0; i<MAX_PORT_INT; i++)
	    if (port_int[i].valid) {
		pp = &port_int[i];
		pd = &port_des[pp->port_index];
    		kern_printf("%d %s  vt: %d      pn: %d\n",i,htable[pp->h_index].name,
        	pd->vuoto,pd->pieno);
	    }
/*
	kern_printf("Port int :\n");
  	kern_printf("Free port int : %d\n", freeportint);
  	for (i=0; i<MAX_PORT_INT; i++)
  		kern_printf("%d  vl : %d  dm : %d  port_index : %d  proc_id : %d\n", i, 
  			port_int[i].valid, port_int[i].dim_mes, 
  			port_int[i].port_index, port_int[i].proc_id);
*/
}

void port_write(PORT p)
{
    struct port_ker *pd;
    struct port_com *pp;
    char msg[80];

    pp = &port_int[p];
    pd = &port_des[pp->port_index];

    kern_printf(msg,"%d  pd: %d   vt: %d      pn: %d ",p,pp->port_index,
        pd->vuoto,pd->pieno);
}

#endif
