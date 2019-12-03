#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main ( int argc, char * argv[] )
{
    if (argc != 7 && argc != 9)
    {
      fprintf(stderr, "Error: %s file act_type out_size d_out in1_type d_in1 [in2_type] [d_in2]\n", argv[0]);
      return 0;
    }

    int i;
    FILE * f = fopen(argv[1], "w");
    if (f == NULL)
    {
      fprintf(stderr, "Error: could not open %s\n", argv[1]);
      return 0;
    }

    int in1_size = 0, in2_size = 0, out_size = 0;
    char in1_name[4], in2_name[4];
    sscanf(argv[3], "%d", &out_size);
    sscanf(argv[5], "%d,%s", &in1_size, in1_name);
    printf("in1: %s, %s, %d\n", argv[5], in1_name, in1_size);
    if (argc > 7)
    {
      sscanf(argv[7], "%d,%s", &in2_size, in2_name);
    }

    // watch vars
    fprintf(f, "watch Reset\nwatch _Reset\nwatch %s.cin\nwatch go_r\nwatch %s.d.out\n", argv[2], argv[2]);
    for(i = 0; i < in1_size; i++)
    {
      fprintf(f, "watch %s[%d].t\n", in1_name, i);
      fprintf(f, "watch %s[%d].f\n", in1_name, i);
    }

    if (argc > 7)
    {
      for(i = 0; i < in2_size; i++)
      {
        fprintf(f, "watch %s[%d].t\n", in2_name, i);
        fprintf(f, "watch %s[%d].f\n", in2_name, i);
      }
    }

    for(i = 0; i < out_size; i++)
    {
      fprintf(f, "watch out[%d].t\n", i);
      fprintf(f, "watch out[%d].f\n", i);
    }

    // set_principal
    fprintf(f, "set_principal Reset\nset_principal _Reset\nset_principal %s.cin\nset_principal go_r\n", argv[2]);
    for(i = 0; i < in1_size; i++)
    {
      fprintf(f, "set_principal %s[%d].t\n", in1_name, i);
      fprintf(f, "set_principal %s[%d].f\n", in1_name, i);
    }

    if (argc > 7)
    {
      for(i = 0; i < in2_size; i++)
      {
        fprintf(f, "set_principal %s[%d].t\n", in2_name, i);
        fprintf(f, "set_principal %s[%d].f\n", in2_name, i);
      }
    }

    for(i = 0; i < out_size; i++)
    {
      fprintf(f, "set_principal out[%d].t\n", i);
      fprintf(f, "set_principal out[%d].f\n", i);
    }

    // set init vals
    fprintf(f, "mode reset\nset Reset 1\nset _Reset 0\n");
    for(i = 0; i < in1_size; i++)
    {
      fprintf(f, "set %s[%d].t 0\n", in1_name, i);
      fprintf(f, "set %s[%d].f 1\n", in1_name, i);
    }

    if (argc > 7)
    {
      for(i = 0; i < in2_size; i++)
      {
        fprintf(f, "set %s[%d].t 0\n", in2_name, i);
        fprintf(f, "set %s[%d].f 1\n", in2_name, i);
      }
    }

    fprintf(f, "cycle\n");

    fprintf(f, "set Reset 0\nset _Reset 1\nset go_r 0\ncycle\nstatus X\n");

    // run
    fprintf(f, "mode run\nassert %s.cin 0\nset go_r 1\ncycle\n", argv[2]);

    // assert init vals
    for(i = 0; i < out_size; i++)
    {
      fprintf(f, "assert out[%d].t 0\n", i);
      fprintf(f, "assert out[%d].f 1\n", i);
    }

    // reset go_r
    fprintf(f, "set go_r 0\ncycle\n");

    // write x input value
    for(i = 0; i < in1_size; i++)
    {
      fprintf(f, "set %s[%d].t %d\n", in1_name, i, argv[6][i] - '0');
      fprintf(f, "set %s[%d].f %d\n", in1_name, i, 1-(argv[6][i] - '0'));
    }

    if (argc > 7)
    {
      // write y input value
      for(i = 0; i < in2_size; i++)
      {
        fprintf(f, "set %s[%d].t %d\n", in2_name, i, argv[8][i] - '0');
        fprintf(f, "set %s[%d].f %d\n", in2_name, i, 1-(argv[8][i] - '0'));
      }
    }


    fprintf(f, "cycle\nset go_r 1\ncycle\n");

    for (i=0; i < 8; i++)
    {
      fprintf(f, "assert out[%d].t %d\n", i, argv[4][i] - '0');
      fprintf(f, "assert out[%d].f %d\n", i, 1-(argv[4][i] - '0'));
    }
}
