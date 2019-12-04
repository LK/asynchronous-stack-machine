/*************************************************************************
 *
 *  Copyright (c) 2018 Rajit Manohar
 *  All Rights Reserved
 *
 **************************************************************************
 */
#ifndef __CHP_H__
#define __CHP_H__

#include "expr.h"
#include "list.h"
#include "hash.h"

/**
 * CHP
 */
enum chp_lang_type {
  CHP_COMMA, CHP_SEMI, CHP_SELECT, CHP_LOOP, CHP_SKIP,
  CHP_ASSIGN, CHP_SEND, CHP_RECV
};

struct chp_lang;

/* 
   chp_gc_t is a linked-list of guarded commands G -> S
   The "next" pointer points to the next guarded command.
   
   The CHP syntax includes "else" -> S. In this case,
   the "g" field will be NULL.

   For selections that are just a wait, the "s" field will be NULL
*/
typedef struct chp_gc {
  Expr *g;			/* guard */
  struct chp_lang *s;	/* statement */
  struct chp_gc *next;
} chp_gc_t ;


/*
  The type field is CHP_COMMA, CHP_SEMI, ..., CHP_RECV
  (see enumeration above)

  Based on the type, access the appropriate structure within the union
  "u".
  
  For example, x is a chp_lang_t *, and if x->type is CHP_ASSIGN, then 
  x->u.assign.id is the name of the variable, and x->u.assign.e is the expression

  The name can be looked up using "find_symbol" to determine if the
  variable is an integer or channel, and to determine its bit-width.

  list_t pointers are used to represent lists of items. If L is a
  list_t *, and it is a list of variables (i.e. char *'s), then you
  can traverse the list in the following manner:

  listitem_t *li;
  char *name;

  for (li = list_first (L); li; li = list_next (li)) {
      name = (char *) list_value (li);
  }

*/
typedef struct chp_lang {
  int type;
  union {
    struct {
      char *id;
      Expr *e;
    } assign;    /* for CHP_ASSIGN: "id := e" */
    
    struct {
      char *chan;		/* the channel name */
      list_t *rhs;		/* for CHP_SEND, this is a list 
				   of expressions (Expr *)
				   for CHP_RECV, this is a list
				   of char *'s (variable names) */
    } comm;
    
    struct {
      list_t *cmd;
    } semi_comma;		/* for CHP_SEMI (CHP_COMMA), this is
				 a list of commands (chp_lang_t *'s) 
				 that were separated by a semicolon
				 (comma) 
				*/
    
    chp_gc_t *gc;		/* for CHP_LOOP or CHP_SELECT */
  } u;
} chp_lang_t;


/* the top-level chp data structure */
typedef struct {
  chp_lang_t *c;
  struct Hashtable *sym;
} Chp;


/* 
   a variable can be either a channel or an integer of a specified
   bit-width
*/
typedef struct {
  const char *name;
  int ischan;			/* 0 if int, 1 if chan */
  int bitwidth;			/* bit width */
} symbol;

/* -- call this to read in the CHP program -- */
Chp *read_chp (const char *file);

/* -- call this to find a symbol from its name; returns NULL if the
   symbol is not found -- */
symbol *find_symbol (Chp *c, const char *name);


/* -- this is called by the parser to add a new symbol -- */
void add_symbol (Chp *c, const char *name, int ischan, int bitwidth);

/* -- prints out the symbols -- */
void print_symbols (Chp *);

#endif /* __CHP_H__ */
