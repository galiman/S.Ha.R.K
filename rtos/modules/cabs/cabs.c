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
 CVS :        $Id: cabs.c,v 1.1 2005/02/25 10:53:41 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2005/02/25 10:53:41 $
 ------------

 Date:        2/7/96

 File:           Cabs.C
 Translated by : Giuseppe Lipari
 Revision:       1.1

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


#include <cabs/cabs/cabs.h>

#include <kernel/config.h>
#include <ll/ll.h>
#include <ll/string.h>
#include <kernel/model.h>
#include <kernel/const.h>
#include <sys/types.h>
#include <kernel/descr.h>
#include <errno.h>
#include <kernel/var.h>
#include <kernel/func.h>

/* "cab":       contiene dei buffer per memorizzare messaggi            */
/* "cab_data":  buffer contenente il messaggio e informazioni relative  */
/* "messaggio": si trova a partire da (*cab_data + 1)                   */

struct cab_data {		/* struttura del buffer di cab  */
    struct cab_data *next;	/* successivo buffer del cab    */
    unsigned short n_uso;	/* processi che usano il buffer */
};

struct cab_desc {			/* struttura descrittore di cab */
	char    name[MAX_CAB_NAME+1];	/* nome del CAB                 */
	CAB     next_cab_free;		/* indice successivo cab libero */
	BYTE    busy;			/* cab libero/occcupato         */
	char    *mem_cab;		/* memoria globale cab          */
	BYTE    n_buf;			/* numero dei buffer nel cab    */
	BYTE    nfree;			/* numero buffer liberi         */
	unsigned dim_mes;		/* dimensione del messaggio     */
	struct  cab_data *free;		/* puntatore primo buffer libero*/
	struct  cab_data *mrd;		/* puntatore must_recent_data   */
};

static struct   cab_desc  cabs[MAX_CAB]; /* vettore descrittori dei CAB  */
static CAB      free_cab;                /* indice del primo cab libero  */

static int checkcab(CAB id)
{
    if (id >= MAX_CAB) {
	errno = ECAB_INVALID_ID;
	return -1;
    }
    if (cabs[id].busy == TRUE) return TRUE;
    else errno = ECAB_CLOSED;
    return -1;
}

/*----------------------------------------------------------------------*/
/* cab_init -- inizializza le strutture dei cab                         */
/*----------------------------------------------------------------------*/
void CABS_register_module(void)
{
    int i;

    free_cab = 0;
    for (i=0; i < MAX_CAB - 1; i++) {
	cabs[i].next_cab_free = i+1;
	cabs[i].busy = FALSE;
    }
    cabs[MAX_CAB-1].next_cab_free = NIL;
    cabs[MAX_CAB-1].busy = FALSE;
//    for (i = CAB_INVALID_MSG_NUM; i <= CAB_CLOSED; i++)
//	exc_set(i,cab_exception);
}

/*----------------------------------------------------------------------*/
/* cab_create -- crea un cab, lo inizializza e restituisce l'indice     */
/*----------------------------------------------------------------------*/
CAB cab_create(char *name, int dim_mes, BYTE num_mes)
{
    CAB id;                     /* indice del cab da restituire         */
    struct cab_desc *pid;       /* puntatore al cab (velocizza accesso) */
    char *mem;                  /* puntatore di appoggio al buffer      */
    struct cab_data *tmp;       /* puntatore di scorrimento lista cab   */
    int i;                      /* variabile indice                     */
    SYS_FLAGS f;
    
    f = kern_fsave();

    /* Se non ci sono piu' cab liberi o il parametro num_mes < 1 */
    /* solleva l'eccezioni                                       */

    if (num_mes < 1) {
	errno = ECAB_INVALID_MSG_NUM;
	kern_frestore(f);
	return -1;
    }
    if ((id=free_cab) != MAX_CAB) {
	pid = &cabs[id];        /* prendo l'indirizzo del cab */
	free_cab = pid->next_cab_free;
    }
    else {
	errno = ECAB_NO_MORE_ENTRY;
	kern_frestore(f);
	return -1;
    }

    /* richiede un identificatore e la memoria */
    mem = kern_alloc((dim_mes + sizeof(struct cab_data)) * num_mes);

    kern_frestore(f);
    /* inizializzazione del descrittore del cab */

    strcpy(pid->name, name);
    pid->mem_cab = mem;
    pid->dim_mes = dim_mes;
    pid->n_buf = num_mes;

    /* inizializzazione primo messaggio e buffer liberi */

    pid->mrd = (struct cab_data *)mem;
    i = (int)num_mes;
	tmp = NULL;
    while (i--) {
	tmp = (struct cab_data *)mem;
	mem += sizeof(struct cab_data) + dim_mes;
	tmp->next = (struct cab_data *)mem;
	tmp->n_uso = 0;
    }

    tmp->next = NULL;
    pid->free = pid->mrd->next;

    mem = (char *)(pid->mrd + 1);
    for (i=0; i<dim_mes; i++) *(mem++) = 0;
    pid->nfree = num_mes - 1;

    f = kern_fsave();
    pid->busy = TRUE;
    kern_frestore(f);

    return(id);
}

/*----------------------------------------------------------------------*/
/* cab_reserve --  richiede un buffer in cui mettere i dati da inviare  */
/*              ritorna un puntatore al buffer                          */
/*----------------------------------------------------------------------*/
char *cab_reserve(CAB id)
{
    struct cab_desc *pid;
    char *buf;
    SYS_FLAGS f;

    /* controlla l'identificatore del CAB */
    if (checkcab(id) == -1) return(NULL);

    pid = &cabs[id];
    f = kern_fsave();
    /* Se il numero di elementi assegnati non e` = al massimo */
    if ((pid->nfree)--) {
	buf = (char *)(pid->free + 1);
	pid->free = pid->free->next;
	kern_frestore(f);
	return(buf);
    }
    else {
	errno = ECAB_TOO_MUCH_MSG;
	kern_frestore(f);
	return(NULL);
    }
}

/*----------------------------------------------------------------------*/
/*  cab_putmes -- immette un nuovo messaggio nel cab                    */
/*----------------------------------------------------------------------*/
int cab_putmes(CAB id, void *pbuf)
{
    struct cab_data *pold;
    struct cab_desc *pid;
    SYS_FLAGS f;

    if (checkcab(id) == -1) return -1;
    pid = &cabs[id];

    f = kern_fsave();
    pold = pid->mrd;
    if (pold->n_uso == 0) {
	pold->next = pid->free;
	pid->free = pold;
	(pid->nfree)++;
    }

    pid->mrd = ((struct cab_data *)pbuf) - 1;
    kern_frestore(f);
    return 1;
}

/*----------------------------------------------------------------------*/
/*  cab_getmes  --  richiede l'ultimo messaggio presente nel cab.       */
/*               Ritorna un puntatore al buffer piu' recente            */
/*----------------------------------------------------------------------*/
char *cab_getmes(CAB id)
{
    char *tmp;
    SYS_FLAGS f;

    if (checkcab(id) == -1) return(NULL);
    f = kern_fsave();

    /* cabs[id].mrd punta all'ultimo buffer inserito, incremento        */
    /* il puntatore di uno e ottengo l'area del messaggio, converto     */
    /* quindi il puntatore al tipo carattere. Segue l'incremento        */
    /* del campo contatore di uso buffer                                */

    (cabs[id].mrd->n_uso)++;
    tmp = (char *)(cabs[id].mrd + 1);

    kern_frestore(f);
    return(tmp);
}

/*----------------------------------------------------------------------*/
/* cab_unget -- segnala che il task non usa piu' il messaggio,          */
/*              se questo non e' piu' usato da nessuno viene rilasciato */
/*              ritorna un risultato                                    */
/*----------------------------------------------------------------------*/
int cab_unget(CAB id, void*pun_mes)
/* small id;                            indice del cab di lavoro        */
/* char *pun_mes;                       puntatore al messaggio          */
{
    struct cab_data *pbuf;
    struct cab_desc *pid;
    SYS_FLAGS f;

    if (checkcab(id) == -1) return -1;
    pid = &cabs[id];

    f = kern_fsave();
    pbuf = ((struct cab_data *)(pun_mes)) - 1;

    if ((--(pbuf->n_uso) == 0) && (pbuf != pid->mrd)) {
	pbuf->next = pid->free;
	pid->free = pbuf;
	(pid->nfree)++;
    }
    kern_frestore(f);
    return 1;
}

/*----------------------------------------------------------------------*/
/*      cab_delete -- libera la memoria e rilascia il cab               */
/*----------------------------------------------------------------------*/
void cab_delete(CAB id)
{
    struct cab_desc *pid;
    SYS_FLAGS f;

    pid = &cabs[id];
    f = kern_fsave();
    kern_free(pid->mem_cab,(pid->dim_mes + sizeof(struct cab_data))*pid->n_buf);

    pid->busy = FALSE;
    pid->next_cab_free = free_cab;
    free_cab = id;
    kern_frestore(f);
}
