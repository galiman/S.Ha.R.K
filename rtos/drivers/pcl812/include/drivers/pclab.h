/*--------------------------------------------------------------*/
/*	File header da includere nel main quando si usano	*/
/*	le funzioni di aquisizione contenute in PCLAB.C		*/
/*--------------------------------------------------------------*/

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

float	ad_conv(int );			/* conversione A/D	*/
int	da_conv(float, int );	       	/* conversione D/A	*/
int	par_in(void);			/* ingresso digitale	*/
void	par_out(int n);			/* uscita  digitale	*/

__END_DECLS

/*--------------------------------------------------------------*/
