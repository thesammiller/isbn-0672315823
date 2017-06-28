/* $Header: /home/student1/dos_cvrt/RCS/dos_cvrt.h,v 1.2 1998/12/06 21:30:53 student1 Exp $
 * Basil Fawlty      $Date: 1998/12/06 21:30:53 $
 *
 * Header File for the dos_cvrt utility program:
 *
 * $Log: dos_cvrt.h,v $
 * Revision 1.2  1998/12/06 21:30:53  student1
 * Added put_ch() and defined RC_* macros
 *
 * Revision 1.1  1998/11/27 02:28:57  student1
 * Initial revision
 */
#ifndef _dos_cvrt_h_
#define _dos_cvrt_h_ "@(#)dos_cvrt.h $Revision: 1.2 $"

/*
 * Command exit codes :
 */
#define RC_CMDOPTS	1	/* Options usage error */
#define RC_OPENERR	2	/* File open failure */
#define RC_READERR	3	/* File read error */
#define RC_WRITERR	4	/* File write error */

extern int unix2dos(const char *pathname);
extern int dos2unix(const char *pathname);
extern int put_ch(int ch);

#endif /* _dos_cvrt_h_ */

/* $Source: /home/student1/dos_cvrt/RCS/dos_cvrt.h,v $ */
