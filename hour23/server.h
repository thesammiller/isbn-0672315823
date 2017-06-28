/* server.h :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 * $Id: server.h,v 1.2 1999/02/20 21:34:38 wwg Exp $
 *
 * This is the main server include file.
 */
#ifndef _server_h_
#define _server_h_ "$Revision: 1.2 $"

#include "tql.h"

/*
 * Server external values:
 */
extern struct S_TQLD {
    char    *skey;          /* IPC Key */
    key_t   key;            /* Binary key value */
    int     msqid;          /* Message queue IPC ID */
    int     signo;          /* Termination signal */
} tqld;

/*
 * Server Table Object:
 */
typedef struct {
    char    *name;          /* Table Name */
    FILE    *file;          /* Open FILE */
} Table;

/*
 * Server Row Object:
 */
typedef struct {
    struct passwd pw;       /* Row contents */
} Row;

/*
 * Column indexes:
 */
#define COLX_COUNT          0   /* COUNT(*) */
#define COLX_PW_NAME        1   /* pw_name */
#define COLX_PW_PASSWORD    2   /* pw_passwd */
#define COLX_PW_UID         3   /* pw_uid */
#define COLX_PW_GID         4   /* pw_gid */
#define COLX_PW_GECOS       5   /* pw_gecos */
#define COLX_PW_DIR         6   /* pw_dir */
#define COLX_PW_SHELL       7   /* pw_shell */

/*
 * SELECT Column:
 */
typedef struct {
    char    tclass;         /* Token class */
    char    *ident;         /* Token ptr */
    int     colx;           /* Column index */
} SelList;

/*
 * FROM Table Reference:
 */ 
typedef struct {
    char    tclass;         /* Token class */
    char    *tabnam;        /* Table name */
    Table   *table;         /* Open Table */
    Row     *row;           /* Current row */
} FromList;

/*
 * WHERE Clause Info:
 */
typedef struct {
    char    tcl_op1;        /* Token class */
    char    *op1;           /* Operand 1 */
    char    tcl_opr;        /* Operator tclass */
    char    tcl_op2;        /* Operand tclass */
    char    *op2;           /* Operand 2 */
    char    tcl_conj;       /* Conjunction if any */
    Bool    b;              /* Result of this clause */
} WhereClause;

/*
 * ORDER BY Clause Info:
 */
typedef struct {
    char    tclass;         /* Token class */
    char    *ref;           /* Column reference */
    int     fldno;          /* Sort field no. */
    Bool    num;            /* Numeric sort */
} OrderBy;

/*
 * Prepared SELECT Statement Info:
 */
typedef struct {
    int         n_slist;                /* SELECT */
    SelList     slist[MAX_SELLIST];     /* column refs */
    int         n_flist;                /* FROM */
    FromList    flist[MAX_FROMLIST];    /* table refs */
    int         n_wlist;                /* WHERE */
    WhereClause wlist[MAX_WHRECLSE];    /* clauses */
    int         n_olist;                /* ORDER BY */
    OrderBy     olist[MAX_ORDERBY];     /* clauses */
} Select;

/*
 * Server External Functions :
 */
extern void server(void);
extern int prepare(Msg *msg,Select *sel);

extern char *colName(int colx);
extern int colIndex(const char *col);
extern Bool isNumeric(int colx);
extern Bool value(const char *str,long *longp);
extern long RowCount;

extern Row *newRow(void);
extern Row *initRow(Row *row);
extern Row *passwdRow(Row *dest,struct passwd *src);
extern Row *cloneRow(Row *dest,Row *src);
extern Row *disposeRow(Row *row);
extern void deleteRow(Row *row);
extern char *gcolRow(Row *row,const char *colname);
extern char *longDup(long lv);

extern Table *tqlOpenTable(const char *table_name);
extern int tqlRewindTable(Table *table);
extern int tqlReadTable(Table *table,Row *row);
extern int tqlWriteTable(Table *table,Row *row);
extern void tqlCloseTable(Table *table);
extern long tqlCountTable(Table *table);

extern Select *initSelect(Select *_this);
extern Select *disposeSelect(Select *_this);

extern int xeqSelect(Select *sel,pid_t client_pid);

extern char *valueOf(char tclass,char *sym,Row *row);
extern Bool numCmp(WhereClause *wc,Row *row);
extern Bool strCmp(WhereClause *wc,Row *row);
extern Bool colCmp(WhereClause *wc,Row *row);
extern Bool compare(WhereClause *wc,int n,Row *row);

extern Bool initSort(void);
extern Bool doOrderBy(Table *tbl,OrderBy *olist,int n);

#endif /* _server_h_ */

/* end sever.h */
