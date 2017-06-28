/* tqlc.h :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 * $Id: tqlc.h,v 1.1 1999/02/20 15:21:12 wwg Exp $
 *
 * This is the Client program include file.
 */
#ifndef _tqlc_h_
#define _tqlc_h_ "$Revision: 1.1 $"

#include "tql.h"

/*
 * Client program external functions:
 */
extern size_t lexSQL(char *buf,size_t bufsiz);
extern int doColHdr(Resp1 *r1);

/*
 * External Variables:
 */
extern char *skey;
extern key_t key;
extern int msqid;

#endif /* _tqlc_h_ */

/* end tqlc.h */
