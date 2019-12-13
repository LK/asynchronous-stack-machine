#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "chp.h"
#include "list.h"
#include "hash.h"

int _print_expr (Expr *e);
int print_chp_stmt (chp_lang_t *c);

static int expr_count = 1;
static int stmt_count = 0;
static int chan_count = 0;

void print_vars (Chp *c)
{
  hash_bucket_t *b;
  int i;
  symbol *sym;

  if (!c || !c->sym) return;
  if (c->sym->n == 0) return;

  printf (" /* --- declaring all variables and channels --- */\n");

  for (i=0; i < c->sym->size; i++) {
    for (b = c->sym->head[i]; b; b = b->next) {
      sym = (symbol *) b->v;
      if (sym->ischan) {
        if (sym->bitwidth > 1) {
          printf (" aN1of2<%d> chan_%s;\n", sym->bitwidth, sym->name);
        }
        else if (sym->bitwidth == 1) {
          printf (" a1of2 chan_%s;\n", sym->name);
        }
        else {
          printf (" a1of1 chan_%s;\n", sym->name);
        }
      }
      else {
        if (sym->bitwidth > 1) {
          printf (" syn_var_init_false var_%s[%d];\n", sym->name,
          sym->bitwidth);
        }
        else if (sym->bitwidth == 1) {
          printf (" syn_var_init_false var_%s;\n", sym->name);
        }
      }
    }
  }
  printf (" /* --- end of declarations --- */\n\n");
}

void emit_const_1 (void)
{
  static int emitted = 0;
  if (!emitted) {
    printf (" syn_var_init_true const_1;\n");
  }
  emitted = 1;
}

void emit_const_0 (void)
{
  static int emitted = 0;
  if (!emitted) {
    printf (" syn_var_init_false const_0;\n");
  }
  emitted = 1;
}

static int base_var = -1;

int unop (char *s, Expr *e)
{
  int l, ret;

  l = _print_expr (e->u.e.l);

  printf (" %s e_%d(e_%d.out);\n", s,
  expr_count, l);
  return expr_count++;
}

int binop (char *s, Expr *e)
{
  int l, r, ret;

  l = _print_expr (e->u.e.l);
  r = _print_expr (e->u.e.r);

  printf (" %s e_%d(e_%d.out, e_%d.out);\n", s,
  expr_count, l, r);
  return expr_count++;
}

static Chp *__chp;

static int func_bitwidth;
static int func_found = 0;

int _print_expr (Expr *e)
{
  int ret;
  switch (e->type) {
    case E_AND:
      ret = binop ("syn_expr_and", e);
    break;
    case E_OR:
      ret = binop ("syn_expr_or", e);
    break;
    case E_PLUS:
      ret = 0;
      //binop ("+", e);
      break;
    case E_MINUS:
      ret = 0;
    //binop ("-", e);
    break;
    case E_NOT:
    case E_COMPLEMENT:
      ret = unop ("syn_expr_not", e);
    break;
    case E_UMINUS:
      ret = 0;
    //printf ("~(");
    //print_expr (e->u.e.l);
    //printf (")");
    break;
    case E_PROBE:
      ret = 0;
    //printf ("#%s", (char *)e->u.e.l);
    break;
    case E_VAR:
    //printf ("%s", (char *)e->u.e.l);
    {
      symbol *v = find_symbol (__chp, (char *)e->u.e.l);
      if (!v) {
        fprintf (stderr, "Symbol `%s' not found\n", (char *)e->u.e.l);
        exit (1);
      }
      if (v->bitwidth == 1) {
        printf (" syn_expr_var e_%d(,var_%s.v);\n", expr_count, (char *)e->u.e.l);
      }
      else {
        printf (" syn_expr_vararray<%d> e_%d;\n", v->bitwidth, expr_count);
        printf (" (i:%d: e_%d.v[i] = var_%s[i].v;)\n", v->bitwidth,
        expr_count, v->name);
      }
      if (base_var == -1) {
        base_var = expr_count;
      }
      else {
        printf (" e_%d.go_r = e_%d.go_r;\n", expr_count, base_var);
      }
      if (func_bitwidth == -1) {
        func_bitwidth = v->bitwidth;
      }
      if (func_found == 1) {
        fprintf (stderr, "Function/variable combinations not supported\n");
        exit (1);
      }
      ret = expr_count++;
    }
    break;
    case E_INT:
      //printf ("%d", (int)e->u.v);
      if (e->u.v == 1) {
        emit_const_1 ();
        printf (" syn_expr_var e_%d(,const_1.v);\n", expr_count);
      }
      else {
        emit_const_0 ();
        printf (" syn_expr_var e_%d(,const_0.v);\n", expr_count);
      }
      if (base_var == -1) {
        base_var = expr_count;
      }
      else {
        printf (" e_%d.go_r = e_%d.go_r;\n", expr_count, base_var);
      }
      ret = expr_count++;
      break;
    case E_TRUE:
      emit_const_1 ();
      printf (" syn_expr_var e_%d(,const_1.v);\n", expr_count);
      if (base_var == -1) {
        base_var = expr_count;
      }
      else {
        printf (" e_%d.go_r = e_%d.go_r;\n", expr_count, base_var);
      }
      ret = expr_count++;
      break;
    case E_FALSE:
      emit_const_0 ();
      printf (" syn_expr_var e_%d(,const_0.v);\n", expr_count);
      if (base_var == -1) {
        base_var = expr_count;
      }
      else {
        printf (" e_%d.go_r = e_%d.go_r;\n", expr_count, base_var);
      }
      ret = expr_count++;
      break;
    case E_FUNCTION:
    /* functions are special cases */
      {
        Expr *tmp;
        int bits;
        char *t;

        func_found = 1;
        if (func_bitwidth != -1) {
          fprintf (stderr, "Only simple functions are supported!\n");
          exit (1);
        }

        t = e->u.fn.s + strlen (e->u.fn.s) - 1;
        while (t > e->u.fn.s) {
          /* has to be an integer */
          if (!isdigit (*t)) {
            if (*t != '_') {
              fprintf (stderr, "Functions must be of the form name_<bitwidth>\n");
              exit (1);
            }
            bits = atoi (t+1);
            break;
          }
          t--;
        }
        if (t == e->u.fn.s) {
          fprintf (stderr, "Functions must be of the form name_<bitwidth>\n");
          exit (1);
        }
        func_bitwidth = bits;
        tmp = e->u.fn.r;
        while (tmp) {
          if (tmp->u.e.l->type != E_VAR) {
            printf ("-- can't handle this case--\n");
            break;
          }
          tmp = tmp->u.e.r;
        }
        if (tmp) {
          return -1;
        }
        /* we have a bundled-data function */
        printf (" bundled_%s e_%d;\n", e->u.fn.s, expr_count);
        tmp = e->u.fn.r;
        int argcount = 0;
        while (tmp) {
          symbol *v = find_symbol (__chp, (char *)tmp->u.e.l->u.e.l);
          if (!v) {
            fprintf (stderr, "No such variable `%s'\n", (char *)tmp->u.e.l->u.e.l);
            exit (1);
          }
          if (v->bitwidth == 1) {
            printf (" e_%d.v%d = var_%s.v;\n", expr_count,
            argcount, v->name);
          }
          else {
            printf (" (i:%d: e_%d.v%d[i] = var_%s[i].v;)\n", v->bitwidth,
            expr_count, argcount, v->name);
          }
          tmp = tmp->u.e.r;
          argcount++;
        }
        if (base_var == -1) {
          base_var = expr_count;
        }
        else {
          printf(" e_%d.go_r = e_%d.go.r;\n", expr_count, base_var);
        }
        ret = expr_count++;
      }
      break;
    case E_EQ:
      ret = binop ("syn_expr_eq", e);
      printf("/* HOUSTON WE HAD AN E_EQ W/ RET %d */\n", ret);
      break;
    default:
      printf ("ERR, type=%d\n", e->type);
    break;
  }
  return ret;
}

/*
returns the expression # for e_<num>.out.{t,f}
or   e_<num>.out[i].{t,f} for multi-bit
output (func_bitwidth will be set to a value > 1)

base_var is a global (ick) for the expression # for e_<num>.go_r
*/

int print_expr (Expr *e)
{
  base_var = -1;
  func_bitwidth = -1;
  func_found = 0;
  return _print_expr (e);
}

/*
go for expression is e_<ego>.go.r
out is e_<eout>.out.{t,f}
req is request for evaluation

returns new number for e_<num>.out.{t,f}
*/
int print_expr_tmpvar (char *req, int ego, int eout, int bits)
{
  int seq = stmt_count++;
  int evar = expr_count++;

  printf (" syn_fullseq s_%d;\n", seq);
  printf (" %s = s_%d.go.r;\n", req, seq);

  if (bits == 1) {
    printf (" syn_recv rtv_%d;\n", seq);
    printf (" syn_expr_var e_%d;\n", evar);
    printf (" syn_var_init_false tv_%d(rtv_%d.v);\n", seq, seq);
    printf (" e_%d.v = tv_%d.v;\n", evar, seq);
    printf (" s_%d.r.r = e_%d.go_r;\n", seq, ego);
    printf (" s_%d.r = rtv_%d.go;\n", seq, seq);
    printf (" e_%d.out.t = rtv_%d.in.t;\n", eout, seq);
    printf (" e_%d.out.f = rtv_%d.in.f;\n", eout, seq);
  }
  else {
    printf (" syn_recv rtv_%d[%d];\n", seq, bits);
    printf (" syn_expr_vararray<%d> e_%d;\n", bits, evar);
    printf (" syn_var_init_false tv_%d[%d];\n", seq, bits);
    printf (" (i:%d: e_%d.v[i] = tv_%d[i].v; e_%d.v[i]=rtv_%d[i].v;)\n",
    bits, evar, seq, evar, seq);
    printf (" s_%d.r.r = e_%d.go_r;\n", seq, ego);
    printf (" (i:%d: s_%d.r.r = rtv_%d[i].go.r;)\n", bits, seq, seq);
    printf (" syn_ctree<%d> ct_%d;\n", bits, seq);
    printf (" (i:%d: ct_%d.in[i]=rtv_%d[i].go.a;)\n", bits, seq, seq);
    printf (" s_%d.r.a = ct_%d.out;\n", seq, seq);
    printf (" (i:%d: e_%d.out[i].t = rtv_%d[i].in.t; e_%d.out[i].f = rtv_%d[i].in.f;)\n", bits, eout, seq, eout, seq);
  }
  printf (" s_%d.go.a = e_%d.go_r;\n", seq, evar);

  return evar;
}

static int gc_chan_count = 0;

/*
Returns integer that corresponds to r1of2 channel that is used to
evaluate the expression and return the Boolean result (t/f).
t = guard evaluated to true and the statement was executed
f = guard evaluated to false
*/
int print_one_gc (chp_gc_t *gc)
{
  int a, b;
  int ret = gc_chan_count++;
  if (!gc) return -1;

  printf (" r1of2 gc_%d;\n", ret);
  if (gc->g) {
    int seq;
    int go_r;
    char buf[1024];
    a = print_expr (gc->g);
    go_r = base_var;

    sprintf (buf, "gc_%d.r", ret);

    /* replace "a" with latched value */
    a = print_expr_tmpvar (buf, go_r, a, 1);

    b = print_chp_stmt (gc->s);
    int c = chan_count++;
    printf(" a1of1 c_%d;\n", c);
    printf(" syn_fullseq fs_%d(c_%d, c_%d);\n", stmt_count++, c, b);

    if (b == -1) {
      /* empty */
      printf (" gc_%d.t = e_%d.out.t;\n", ret, a);
    }
    else {
      printf (" e_%d.out.t = c_%d.r;\n", a, c);
      printf (" gc_%d.t = c_%d.a;\n", ret, c);
    }
    printf (" gc_%d.f = e_%d.out.f;\n", ret, a);
  }
  else {
    b = print_chp_stmt (gc->s);
    int c = chan_count++;
    printf(" a1of1 c_%d;\n", c);
    printf(" syn_fullseq fs_%d(c_%d, c_%d);\n", stmt_count++, c, b);
    printf (" gc_%d.r = c_%d.r;\n", ret, c);
    printf (" gc_%d.t = c_%d.a;\n", ret, c);
    //printf (" prs { Reset|~Reset -> gc_%d.f- }\n", ret);
    printf (" gc_%d.f = GND;\n", ret);
  }
  return ret;
}

/*
prints loop/selection staetment, returns channel # for the "go" command.
*/
int print_gc (int loop, chp_gc_t *gc)
{
  int start_gc_chan = gc_chan_count;
  int end_gc_chan;
  int ret, i;
  int na;
  static int gc_num = 0;
  int mygc;

  mygc = gc_num++;
  printf ("\n /*--- emit individual gc (#%d) [%s] ---*/\n", mygc, loop ? "loop" : "selection");
  start_gc_chan = print_one_gc (gc);
  end_gc_chan = start_gc_chan;
  gc = gc->next;
  while (gc) {
    end_gc_chan = print_one_gc (gc);
    gc = gc->next;
  }

  ret = chan_count++;
  printf (" a1of1 c_%d;\n", ret);

  printf (" /* gc cascade, start=%d, end=%d */\n", start_gc_chan,
  end_gc_chan);

  for (i=start_gc_chan; i < end_gc_chan; i++) {
    /* gc cascade */
    printf (" gc_%d.f = gc_%d.r;\n", i, i+1);
  }
  if (!loop) {
    printf (" gc_%d.r = c_%d.r;\n", start_gc_chan, ret);
  }
  else {
    na = stmt_count++;
    printf (" syn_notand na_%d;\n", na);
    printf (" na_%d.x = c_%d.r;\n", na, ret);
    printf (" na_%d.out = gc_%d.r;\n", na, start_gc_chan);
  }

  if (start_gc_chan == end_gc_chan) {
    /* only one guard */
    if (!loop) {
      printf (" gc_%d.t = c_%d.a;\n", start_gc_chan, ret);
    }
    else {
      printf (" gc_%d.t = na_%d.y;\n", start_gc_chan, na);
      printf (" gc_%d.f = c_%d.a;\n", start_gc_chan, ret);
    }
  }
  else {
    int a, b;
    /* multi-stage or gate */
    a = stmt_count++;
    printf (" syn_or2 or_%d(gc_%d.t,gc_%d.t);\n", a,
    start_gc_chan, start_gc_chan+1);
    for (i=start_gc_chan+2; i < end_gc_chan; i++) {
      b = stmt_count++;
      printf (" syn_or2 or_%d(or_%d.out,gc_%d.t);\n", b, a, i);
      a = b;
    }
    if (!loop) {
      printf (" or_%d.out = c_%d.a;\n", a, ret);
    }
    else {
      printf (" or_%d.out = na_%d.y;\n", a, na);
      printf (" gc_%d.f = c_%d.a;\n", end_gc_chan, ret);
    }
  }
  printf (" /* ----- end of gc (#%d) ----- */\n\n", mygc);
  return ret;
}

/*
Prints chp statement, returns channel # for "go" command
*/
int print_chp_stmt (chp_lang_t *c)
{
  int ret;
  int a, b, go_r;
  symbol *v, *u;
  char buf[100];
  if (!c) return -1;
  switch (c->type) {
    case CHP_SKIP:
    printf ("/* skip */  ");
    printf (" a1of1 c_%d;\n", chan_count);
    printf (" syn_skip s_%d(c_%d);\n", stmt_count, chan_count);
    stmt_count++;
    ret = chan_count++;
    break;
    case CHP_ASSIGN:
    printf ("/* assign */\n");
    a = print_expr (c->u.assign.e);
    go_r = base_var;
    ret = chan_count++;
    b = stmt_count++;
    printf (" a1of1 c_%d;\n", ret);
    sprintf (buf, "c_%d.r", ret);
    //printf (" e_%d.go_r = c_%d.r;\n", go_r, ret);
    if (func_bitwidth == -1) {
      func_bitwidth = 1;
    }
    a = print_expr_tmpvar (buf, go_r, a, func_bitwidth);
    v = find_symbol (__chp, c->u.assign.id);
    if (!v) {
      fprintf (stderr, "Variable %s not found\n", c->u.assign.id);
      exit (1);
    }
    if (func_bitwidth != v->bitwidth) {
      fprintf (stderr, "Function bitwidth (%d) doesn't match variable bitwidth (%d)\n", func_bitwidth, v->bitwidth);
      exit (1);
    }
    if (v->bitwidth == 1) {
      printf (" syn_recv s_%d(c_%d);\n", b, ret);
      printf (" s_%d.in.t = e_%d.out.t;\n", b, a);
      printf (" s_%d.in.f = e_%d.out.f;\n", b, a);
      printf (" s_%d.v = var_%s.v;\n", b, c->u.assign.id);
    }
    else {
      printf (" syn_recv s_%d[%d];\n", b, v->bitwidth);
      printf (" (i:%d: s_%d[i].go.r = c_%d.r;)\n", v->bitwidth, b, ret);
      printf (" syn_ctree<%d> ct_%d;\n", v->bitwidth, b);
      printf (" (i:%d: ct_%d.in[i]=s_%d[i].go.a;)\n", v->bitwidth,
      b, b);
      printf (" ct_%d.out=c_%d.a;\n", b, ret);
      printf (" (i:%d: s_%d[i].in.t = e_%d.out[i].t;\n          s_%d[i].in.f = e_%d.out[i].f;\n          s_%d[i].v = var_%s[i].v; )\n", v->bitwidth, b, a, b, a, b, c->u.assign.id);
    }
    printf ("\n");
    break;
    case CHP_SEND:
    printf ("/* send */\n");
    if (list_length (c->u.comm.rhs) == 1) {
      a = print_expr ((Expr *)list_value (list_first (c->u.comm.rhs)));
      go_r = base_var;
      ret = chan_count++;
      b = stmt_count++;
      if (func_bitwidth == -1) {
        func_bitwidth = 1;
      }
      printf (" a1of1 c_%d;\n", ret);
      printf (" a1of1 recv_r_%d;\n", b);
      printf (" syn_fullseq fs_%d(c_%d, recv_r_%d);\n", b, ret, b);

      sprintf (buf, "recv_r_%d.r", b);
      a = print_expr_tmpvar (buf, go_r, a, func_bitwidth);

      //printf (" c_%d.r = e_%d.go_r;\n", ret, base_var);
      printf (" recv_r_%d.a = chan_%s.a;\n", b, c->u.comm.chan);
      v = find_symbol (__chp, c->u.comm.chan);
      if (!v) {
        fprintf (stderr, "Channel `%s' not found\n", c->u.comm.chan);
        exit (1);
      }
      if (v->bitwidth != func_bitwidth) {
        fprintf (stderr, "Channel `%s' bitwidth (%d) doesn't match expression (%d)\n", c->u.comm.chan, v->bitwidth, func_bitwidth);
        exit (1);
      }
      if (func_bitwidth == 1) {
        printf (" chan_%s.t = e_%d.out.t;\n", c->u.comm.chan, a);
        printf (" chan_%s.f = e_%d.out.f;\n", c->u.comm.chan, a);
      }
      else {
        printf (" (i:%d: chan_%s.d[i] = e_%d.out[i];)\n", func_bitwidth,
        v->name, a);
      }
    }
    else if (list_length (c->u.comm.rhs) == 0) {
      ret = chan_count++;
      v = find_symbol (__chp, c->u.comm.chan);
      if (!v) {
        fprintf (stderr, "Channel `%s' not found\n", c->u.comm.chan);
        exit (1);
      }
      if (v->bitwidth != 0) {
        fprintf (stderr, "Channel `%s' needs to have 0 bitwidth\n", c->u.comm.chan);
        exit (1);
      }
      printf (" a1of1 c_%d;\n", ret);
      printf (" syn_sendraw s_%d (c_%d, chan_%s);\n", stmt_count++, ret,
      c->u.comm.chan);
    }
    printf ("\n");
    break;
    case CHP_RECV:
    printf ("/* recv */\n");
    if (list_length (c->u.comm.rhs) == 1) {
      ret = chan_count++;
      a = stmt_count++;
      printf (" a1of1 c_%d;\n", ret);
      printf (" syn_fullseq fs_%d(c_%d,);\n", a, ret);
      v = find_symbol (__chp, c->u.comm.chan);
      if (!v) {
        fprintf (stderr, "Channel `%s' not found\n", c->u.comm.chan);
        exit (1);
      }
      u = find_symbol (__chp, (char *)list_value (list_first (c->u.comm.rhs)));
      if (!u) {
        fprintf (stderr, "Variable %s not found\n", (char *)list_value (list_first (c->u.comm.rhs)));
        exit (1);
      }
      if (v->bitwidth != u->bitwidth) {
        fprintf (stderr, "Channel `%s' bitwidth (%d) doesn't match expression (%d)\n", c->u.comm.chan, v->bitwidth, u->bitwidth);
        exit (1);
      }
      if (v->bitwidth == 1) {
        printf (" syn_recv s_%d(fs_%d.r);\n", a, a);
        printf (" s_%d.in = chan_%s;\n", a, c->u.comm.chan);
        printf (" s_%d.v = var_%s.v;\n", a, u->name);
      }
      else {
        printf (" syn_recv s_%d[%d];\n", a, v->bitwidth);
        printf (" (i:%d: s_%d[i].go.r = fs_%d.r.r;)\n", v->bitwidth, a, a);
        printf (" syn_ctree<%d> ct_%d;\n", v->bitwidth, a);
        printf (" (i:%d: ct_%d.in[i]=s_%d[i].go.a;)\n", v->bitwidth,
        a, a);
        printf (" ct_%d.out=fs_%d.r.a; fs_%d.r.a=chan_%s.a;\n", a, a, a, v->name);
        printf (" (i:%d: s_%d[i].in.t = chan_%s.d[i].t;\n          s_%d[i].in.f = chan_%s.d[i].f;\n          s_%d[i].v = var_%s[i].v; )\n", v->bitwidth, a, v->name, a, v->name, a, u->name);
      }
    }
    else if (list_length (c->u.comm.rhs) == 0) {
      ret = chan_count++;
      v = find_symbol (__chp, c->u.comm.chan);
      if (!v) {
        fprintf (stderr, "Channel `%s' not found\n", c->u.comm.chan);
        exit (1);
      }
      if (v->bitwidth != 0) {
        fprintf (stderr, "Channel `%s' needs to have 0 bitwidth\n", c->u.comm.chan);
        exit (1);
      }
      printf (" a1of1 c_%d;\n", ret);
      printf (" syn_recvraw s_%d (c_%d, chan_%s);\n", stmt_count++, ret,
      c->u.comm.chan);
    }
    printf ("\n");
    break;
    case CHP_COMMA:
    case CHP_SEMI:
    {
      listitem_t *li;
      if (list_length (c->u.semi_comma.cmd) == 1) {
        return print_chp_stmt ((chp_lang_t *)list_value (list_first (c->u.semi_comma.cmd)));
      }

      printf ("/* %s */\n", c->type == CHP_COMMA ? "comma" : "semicolon");
      a = chan_count++;
      ret = a;
      printf (" a1of1 c_%d;\n", ret);
      for (li = list_first (c->u.semi_comma.cmd); list_next (li); li = list_next (li)) {
        int s;
        b = print_chp_stmt ((chp_lang_t *)list_value (li));
        s = stmt_count++;
        if (c->type == CHP_SEMI) {
          printf (" syn_seq s_%d(c_%d);\n", s, a);
        }
        else {
          printf (" syn_par s_%d(c_%d);\n", s, a);
        }
        printf (" s_%d.s1 = c_%d;\n", s, b);
        if (!list_next (list_next (li))) {
          /* if this is the last loop iteration */
          b = print_chp_stmt ((chp_lang_t *)list_value (list_next (li)));
          printf (" s_%d.s2 = c_%d;\n", s, b);
        }
        else {
          printf (" a1of1 c_%d;\n", chan_count);
          printf (" s_%d.s2 = c_%d;\n", s, chan_count);
          a = chan_count++;
        }
      }
      printf ("\n");
    }
    break;

    case CHP_LOOP:
    case CHP_SELECT:
    ret = print_gc ((c->type == CHP_LOOP) ? 1 : 0, c->u.gc);
    break;
    default:
    fprintf (stderr, "ERR\n");
    exit (1);
    break;
  }
  return ret;
}

void print_chp_structure (Chp *c)
{
  int i;
  printf ("import \"lab_syn.act\";\n");
  printf ("import \"opcode_conditions.act\";\n");
  printf ("import \"functions.act\";\n\n");
  printf ("import \"combined_functions.act\";\n\n");
  printf ("defproc toplevel (a1of1 go)\n{\n");
  /* --- your code starts here --- */

  /* symbol table printed for you */
  print_vars (c);

  i = print_chp_stmt (c->c);
  printf (" go = c_%d;\n", i);
  printf ("}\n");
}

void check_types (Chp *c)
{
  /* -- your code here -- */

}

int main (int argc, char **argv)
{
  Chp *c;

  if (argc != 2) {
    fprintf (stderr, "Usage: %s <chp>\n", argv[0]);
    return 1;
  }

  c = read_chp (argv[1]);

  /* check that variables are used properly */
  check_types (c);

  /* print the CHP program please */
  __chp = c;
  print_chp_structure (c);

  return 0;
}
