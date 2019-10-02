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

/* CVS :        $Id: fab_msg.h,v 1.2 2004/05/23 08:59:28 giacomo Exp $ */

/*
 * Copyright (C) 2000 Fabio Calabrese <fabiocalabrese77@yahoo.it>
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

/****************************************************
 *                                                  *
 *  file:        FAB_msg.h                          *
 *  libreria:    FAB_LIB                            *
 *  version:     1.0 25/08/2002                     *
 *  creato da:   Fabio CALABRESE                    *
 *                                                  *
 ****************************************************
 *                                                  *
 *  descrizione: libreria di funzioni per stampare  *
 *               messaggi circa il corretto esito   *
 *               dell'esecuzione delle istruzioni;  *
 *               stampa semplici stringhe colorate; *
 *               aiuta a fare il debbuging.         *
 *                                                  *
 ****************************************************/

#ifndef __FAB_LIB__FAB_msg_h__
#define __FAB_LIB__FAB_msg_h__
//-------------------BEGIN---------------------------

// *** Librerie di S.Ha.R.K. ***
#include <ll/i386/cons.h>
// *** Librerie Standard C ***
// *** Librerie FAB ***

// -----------
// Definizioni
//----------------------
#define FAB_EXIT_YES  1
#define FAB_EXIT_NO   0
#define FAB_EMPTY_MSG ""
//----------------------

// ---------------- 
// Funzioni & Macro 
//-------------------------------------------------
#define FAB_newline()       NL()
#define FAB_home()          HOME()
#define FAB_clr()           clear()

void    FAB_print(char * InitMsg, char * msg);
#define FAB_msg(TEST,OKMSG,ERRORMSG)         __FAB_go((TEST),(OKMSG),(ERRORMSG),FAB_EXIT_NO)
#define FAB_go(TEST,OKMSG,ERRORMSG,EXITFLAG) __FAB_go((TEST),(OKMSG),(ERRORMSG),(EXITFLAG))
//--------------------------------------------------


// Altro...

void __FAB_go(int test, char * OkMsg, char * ErrorMsg,int EXITflag);

//----------------------END--------------------------
#endif
