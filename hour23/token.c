/* token.c :
 * Sams Teach Yourself LINUX Programming in 24 Hours
 * ISBN 0-672-31582-3
 * Warren W. Gay VE3WWG
 *
 * This module provides the lexical function of breaking
 * the input into TQL tokens.
 */
static const char rcsid[] =
    "$Id: token.c,v 1.1 1999/02/20 21:32:09 wwg Exp $";

#include "tql.h"

/*
 * Lexical tables :
 */
static const char whitesp[] = " \t";   /* White space chars */
static const char quotes[] = "\"'`";    /* Quote characters */
static const char newlines[] = "\n\r"; /* Line terminations */
static const char identifier[] =             /* Identifiers */
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "_0123456789/";
static const char number[] = "1234567890";      /* Integers */
static const char opr[] =            /* Operator characters */
    "~!@#$%^&*()-+={}|[]\\:;<>?,./";

/*
 * Two character mapping table:
 */
static const struct {
    char    *opr;           /* Operator string */
    int     lex;            /* Lexical token value */
} pair[] = {
    { "<>", TKN_NE },       /* Not equal to */
    { "!=", TKN_NE },       /* Not equal to */
    { "<=", TKN_LE },       /* Less than or equal */
    { ">=", TKN_GE },       /* Greater than or equal */
    { NULL, 0 }             /* End of table */
};

/*
 * TQL SQL keywords Table:
 */
static const struct {
    char    *keyword;       /* Keyword string */
    int     lex;            /* Lexical token value */
} keyw[] = {
    { "select", TKN_SELECT },
    { "from",   TKN_FROM },
    { "where",  TKN_WHERE },
    { "and",    TKN_AND },
    { "or",     TKN_OR },
    { "order",  TKN_ORDER },
    { "by",     TKN_BY },
    { "count",  TKN_COUNT },
    { "shutdown", TKN_SHUTDN },
    { NULL,     0 }         /* End of table */
};

/*
 * Get keyword lexical token value for string:
 *
 * ARGS:
 *  str     input   String to be recognized
 *  lexDflt input   Default if not recognized
 *
 * RETURNS:
 *  TKN_*    Lexical token value of the keyword if recognized
 *  lexDflt  Argument lexDflt is returned if the string is
 *           not recognized as a keyword.
 */
static int
getKeyword(char *str,int lexDflt) {
    short x;

    for ( x=0; keyw[x].keyword != NULL; ++x )
        if ( !strcasecmp(keyw[x].keyword,str) )
            return keyw[x].lex; /* TKN_* value */

    return lexDflt;             /* Not keyword */
}

/*
 * Get input line, discard newline chars :
 *
 * ARGS:
 *  buf     output  Buffer for putting the line
 *  bufsiz  input   Max bytes for buf[]
 *
 * RETURNS:
 *  ptr     Input line loaded into buf[]
 *  NULL    Error or End-File
 */
static char *
getLine(char *buf,size_t bufsiz) {
    char *p;

    if ( ferror(stdin) || feof(stdin) )
        return NULL;        /* Error(s) or EOF */

    /*
     * Only prompt the user, if the input is
     * coming from standard input:
     */
    if ( isatty(fileno(stdin)) )
        fputs("> ",stdout);

    /*
     * Obtain input from stdin:
     */
    if ( !fgets(buf,bufsiz,stdin) )
            return NULL;    /* End File */

    /*
     * Look for LF or CR that we don't want. Note
     * CR only is a problem if someone pipes in a
     * DOS text file.
     */
    p = buf + strcspn(buf,newlines);
    if ( *p != NUL )        
        *p = NUL;           /* Stomp out newline */

    return buf;             /* return Input line */
}

/*
 * Allocate a token :
 *
 * This routine extracts the token starting at pointer p,
 * ending before pointer s, into an allocated string.
 *
 * ARGS:
 *  p       input   Start of new token.
 *  s       input   End + 1 of new token.
 *
 * RETURNS:
 *  ptr     Pointer to allocated string for the
 *          token. It must be freed by the caller
 *          when no longer needed.
 */
static char *
mkToken(char *p,char *s) {
    size_t n = s - p;                    /* Token length */
    char *token = (char *) malloc(n+1); /* Space for NUL */

    strncpy(token,p,n)[n] = NUL;        /* Copy in token */
    return token;              /* Return token to caller */
}

/*
 * Parse a token from the input string :
 *
 * ARGS:
 *  str     I/O     Input parse pointer, and
 *                  later a returned parse ptr.
 *  tclass  output  Ptr to char that identifies
 *                  the token classification.
 *
 * RETURNS:
 *  ptr     An allocated string ptr holding the
 *          token is returned. The caller should
 *          free the string when it is no longer
 *          required.
 */
static char *
getToken(char **str,char *tclass) {
    short x;            /* Work index */
    char *p, *s;        /* Lexical parsing pointers */
    char q;             /* Quote character */

    /*
     * Top of the parse loop: Make sure we have characters
     * to parse.
     */
loop:
    if ( !*str || !**str )
        return NULL;                     /* No more data */

    p = *str + strspn(*str,whitesp);  /* Skip whitespace */
    if ( !*p )
        return *str = NULL;             /* End of string */

    if ( strchr(quotes,*p) != NULL ) {         /* Quote? */
        /*
         * Quoted String :
         */
        *tclass = TKN_STRING;           /* Quoted string */
        q = *p++;                         /* Start quote */
        s = strchr(p,q);               /* Find end quote */
        if ( !s ) {             /* No terminating quote? */
            *str = NULL;       /* No more str to process */
            return strdup(p);           /* Return string */
        }
        *str = s + 1;                  /* Skip end quote */
        return mkToken(p,s);            /* Return string */

    } else {                           /* else not quote */

        if ( isdigit(*p) ) {              /* Is a digit? */
            /*
             * Numeric value :
             */
            *tclass = TKN_NUMBER;    /* Numeric constant */
            s = p + strspn(p,number);        /* Find end */

        } else {
            /*
             * Identifier, keyword or operator :
             */
            s = p + strspn(p,identifier); /* Identifier? */
            if ( s > p )                 /* Is s past p? */
                *tclass = TKN_IDENT; /* identifier token */
            else {
                /*
                 * Assume an operator :
                 */
                *tclass = TKN_OPR;  /* Oper. ch or delim */
                s = p + strspn(p,opr);       /* Find end */
                if ( s <= p ) {         /* Was s past p? */
                    *str = p + 1;   /* No, Skip bad char */
                    goto loop;   /* Restart from the top */
        	}
                /*
                 * Operator confirmed:
                 */
                if ( s - p >= 2 ) {/* Check for dual chr */
                    /*
                     * Test for two char operators
                     * like >= and <= etc.
                     */
                    s = p + 1;     /* Assume 1 char oper */

                    for ( x=0; pair[x].opr != NULL; ++x )
                        if ( !strncmp(p,pair[x].opr,2) ) {
                            *tclass = pair[x].lex;  /*yes*/
                            s = p + 2;   /* 2 char oper. */
                            break;
                        }
                                 /* else 1 char operator */
                } else {
                    /*
                     * All other operators are 1 char :
                     */
                    s = p + 1;    /* make s 1 chr past p */
                    if ( *p == ';' )  /* semi is special */
                        *tclass = TKN_END;  /* End query */
                }
            }
        }
    }

    /*
     * Here we update the caller on where the parsing
     * pointer is now. We make a token from what we
     * found. If the token is TKN_IDENT, we check to
     * see if it is a recognized keyword, and change
     * its classification accordingly.
     */
    *str = s;                         /* Next parse here */
    p = mkToken(p,s);                  /* Allocate token */
    if ( *tclass == TKN_IDENT )      /* Test for keyword */
        *tclass = getKeyword(p,*tclass);/* xlate to kywd */
    return p;                            /* Return token */
}

/*
 * Input a token from standard input.
 *
 * ARGS:
 *  tclass  output  Returned token classification of the
 *                  token that is returned, if any.
 * RETURNS:
 *  ptr     Ptr to allocated token is returned, with the
 *          corresponding classification indicated via
 *          the tclass argument.
 *  NULL    End-file or error on stdin.
 */
static char *
inpToken(char *tclass) {
    char *token;                      /* Allocated token */
    static char ibuf[256] = "";          /* Input buffer */
    static char *bufp = ibuf;    /* Input buffer pointer */

    /*
     * Loop until we get a token, or until error/End-File
     * occurs:
     */
    do  {
        /*
         * Only call getLine() when our static input
         * buffer ibuf[] has been exhausted of data:
         */
        if ( !bufp || !*bufp ) {
            if ( feof(stdin)
              || !getLine(ibuf,sizeof ibuf) ) {
                *tclass = TKN_EOF;
                return NULL;
            }
            bufp = ibuf;      /* Restart parse in ibuf[] */
        }
        token = getToken(&bufp,tclass); /* Extract token */
    } while ( !token );     /* Loop until we get a token */

    return token;                        /* Return token */
}

/*
 * The putToken() function puts a token into the buffer
 * in compact TQL format.
 *
 * ARGS:
 *  buf[]   output  The buffer where the token is copied
 *                  into.
 *  bufsiz  input   The maximum size of buf[] array.
 *  tclass  input   The class of token being "put".
 *  token   input   The token string being "put".
 *
 * RETURNS:
 *  ptr     Pointer to where next byte is to be placed
 *          into the buffer.
 */
static char *
putToken(char *buf,size_t bufsiz,char tclass,char *token) {
    size_t n;                   /* Byts to put */

    /*
     * Determine the number of bytes we must "put". For
     * tokens that include strings, we need:
     *  1 byte                  for tclass
     *  strlen(token) bytes     for the token itself.
     *  1 byte                  for the NUL byte.
     *
     * Else for simple tokens, we only need 1 byte.
     */
    if ( tclass == TKN_STRING
    ||   tclass == TKN_IDENT
    ||   tclass == TKN_NUMBER )
        n = 1 + strlen(token) + 1;    /* Bytes needed */
    else
        n = 1;           /* Simple token needs 1 byte */

    /*
     * Check if we have enough to fit this token.
     * If not, return NULL to indicate overflow.
     */
    if ( n > bufsiz )
        return NULL;               /* Buffer overflow */

    /*
     * Put the first byte. For simple tokens, just
     * put the single token character. For complex
     * tokens, put the token class:
     */
    switch ( tclass ) {
    case TKN_OPR :
        *buf = *token;        /* Single operator char */
        break;
    default :
        *buf = tclass;        /* Token classification */
    }

    /*
     * Only for TKN_IDENT, TKN_STRING and TKN_NUMBER
     * do we need to include the string token:
     */
    if ( n > 1 )
        strcpy(buf+1,token);            /* Copy token */

    return buf + n;                 /* Return new ptr */
}

/*
 * Input one SQL statement, parse it, and place it into
 * the supplied buffer to be sent to the TQL server:
 *
 * ARGS:
 *  buf[]       output  The buffer that receives the
 *                      compact TQL format query
 *                      statement.
 *  bufsiz      input   The maximum size of the buf[]
 *                      array.
 *
 * RETURNS:
 *  n       The number of compacted TQL bytes that
 *          have been placed into the buffer buf[].
 *  0       End-file or Error on stdin.
 */
size_t
lexSQL(char *buf,size_t bufsiz) {
    char *bufp = buf;              /* Current buf ptr */
    char *ebuf = buf + bufsiz;      /* End buffer ptr */
    char *token;           /* Current allocated token */
    char tclass;              /* Token classification */

    /*
     * Read TQL statements from stdin, until EOF, or
     * we see the statement terminator ';'
     */
    while ( (token = inpToken(&tclass)) != NULL ) {
        bufp = putToken(bufp,ebuf-bufp,tclass,token);
        free(token);

        if ( !bufp ) {
            logf("TQL input buffer overflow.\n\n");
            bufp = buf;                 /* Start over */
            continue;
        }

        /*
         * If we got the terminating ';', then return
         * what we have:
         */
        if ( tclass == TKN_END )
            return bufp - buf;
    }

    /*
     * Indicate EOF on stdin, or Error.
     */
    return 0;
}

/* End token.c */
