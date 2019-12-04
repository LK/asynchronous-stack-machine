/*************************************************************************
 *
 *  Copyright (c) 2018 Rajit Manohar
 *  All Rights Reserved
 *
 **************************************************************************
 */
#ifndef __EXPR_H__
#define __EXPR_H__

#define  E_AND 	 0
#define  E_OR 	 1
#define  E_NOT 	 2
#define  E_PLUS  3
#define  E_MINUS 4
#define  E_MULT	 5
#define  E_DIV	 6
#define  E_MOD	 7
#define  E_LSL	 8
#define  E_LSR	 9
#define  E_ASR	 10
#define  E_UMINUS 11
#define  E_INT	 12  /* not a real token */
#define  E_VAR	 13  /* not a real token */
#define  E_QUERY 14
#define  E_LPAR	 15
#define  E_RPAR	 16
#define  E_XOR	 17
#define  E_LT	 18
#define  E_GT    19
#define  E_LE    20
#define  E_GE	 21
#define  E_EQ    22
#define  E_NE    23
#define  E_TRUE  24  /* not a real token */
#define  E_FALSE 25  /* not a real token */
#define  E_COLON 26
#define  E_PROBE 27
#define  E_COMMA 28
#define  E_CONCAT 29
#define  E_BITFIELD 30
#define  E_COMPLEMENT 31 /* bitwise complement */
#define  E_REAL 32  /* not a real token */
#define  E_END   33

#define E_NUMBER 34 /* # of E_xxx things */

#define E_FUNCTION 100

typedef struct expr {
  int type;
  union {
    struct {
      struct expr *l, *r;
    } e; /* for unary operators, only l is used; otherwise l, r are used */
    struct {
      char *s;
      struct expr *r;
    } fn;
    double f;
    unsigned int v;
  } u;
} Expr;


#endif /* __EXPR_H__ */
