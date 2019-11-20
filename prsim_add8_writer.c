#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main ( int argc, char * argv[] )
{
    if (argc < 5 || strlen(argv[2]) < 8 || strlen(argv[3]) < 8 || strlen(argv[4]) < 8)
    {
      fprintf(stderr, "Error: invalid test value\n");
      return 0;
    }

    int i;
    FILE * f = fopen(argv[1], "w");
    if (f == NULL)
    {
      fprintf(stderr, "Error: could not open %s\n", argv[1]);
      return 0;
    }

    // watch vars
    fprintf(f, "watch Reset\nwatch ba.cin\nwatch go_r\nwatch ba.d.out\n");
    for(i = 0; i < 8; i++)
    {
      fprintf(f, "watch x[%d].t\n", i);
      fprintf(f, "watch x[%d].f\n", i);
      fprintf(f, "watch y[%d].t\n", i);
      fprintf(f, "watch y[%d].f\n", i);
      fprintf(f, "watch out[%d].t\n", i);
      fprintf(f, "watch out[%d].f\n", i);
    }

    // set_principal
    fprintf(f, "set_principal Reset\nset_principal ba.cin\nset_principal go_r\n");
    for(i = 0; i < 8; i++)
    {
      fprintf(f, "set_principal x[%d].t\n", i);
      fprintf(f, "set_principal x[%d].f\n", i);
      fprintf(f, "set_principal y[%d].t\n", i);
      fprintf(f, "set_principal y[%d].f\n", i);
      fprintf(f, "set_principal out[%d].t\n", i);
      fprintf(f, "set_principal out[%d].f\n", i);
    }

    // set init vals
    fprintf(f, "mode reset\nset Reset 1\n");
    for(i = 0; i < 8; i++)
    {
      fprintf(f, "set x[%d].t 0\n", i);
      fprintf(f, "set x[%d].f 1\n", i);
      fprintf(f, "set y[%d].t 0\n", i);
      fprintf(f, "set y[%d].f 1\n", i);
    }
    fprintf(f, "cycle\n");

    fprintf(f, "set Reset 0\nset go_r 0\ncycle\nstatus X\n");

    // run
    fprintf(f, "mode run\nassert ba.cin 0\nset go_r 1\ncycle\n");

    // assert init vals
    for(i = 0; i < 8; i++)
    {
      fprintf(f, "assert out[%d].t 0\n", i);
      fprintf(f, "assert out[%d].f 1\n", i);
    }

    // reset go_r
    fprintf(f, "set go_r 0\ncycle\n");

    // write x input value
    for(i = 0; i < 8; i++)
    {
      fprintf(f, "set x[%d].t %d\n", i, argv[2][i] - '0');
      fprintf(f, "set x[%d].f %d\n", i, 1-(argv[2][i] - '0'));
    }

    // write y input value
    for(i = 0; i < 8; i++)
    {
      fprintf(f, "set y[%d].t %d\n", i, argv[3][i] - '0');
      fprintf(f, "set y[%d].f %d\n", i, 1-(argv[3][i] - '0'));
    }

    fprintf(f, "cycle\nset go_r 1\ncycle\n");

    for (i=0; i < 8; i++)
    {
      fprintf(f, "assert out[%d].t %d\n", i, argv[4][i] - '0');
      fprintf(f, "assert out[%d].f %d\n", i, 1-(argv[4][i] - '0'));
    }
}
