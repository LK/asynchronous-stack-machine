#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define BOOL 0
#define DUALRAIL 1

typedef struct _var {
  int type;         // 0=bool, 1=dualrail,
  int len_var_array;    // length of array, or 1 if no array
  char * varname;   // string of varname that prsim can watch
  int ** values;     // list of [set1, out1, set2, out2] ... where sets and outs are arrays of size len_var_array
  int len_values;   // length of list with values (must be even #)
  int num_cycles;
} var;

int read_file(FILE * f, var * vars, int * len_vars, int * max_len_vars)
{
  int i, j, k;
  char * line = NULL;
  size_t len = 0;
  int i_vars = 0;
  while (getline(&line, &len, f) != -1)
  {
    printf("%s", line);

    if (len < 10 || (line[0] != 'b' && line[0] != 'd')) // only handles bools & dualrails
    {
      fprintf(stderr, "ERROR: invalid line: %s\n", line);
      return -1;
    }

    // increase vars array if necessary
    if (i_vars == *max_len_vars)
    {
      vars = (var *) realloc(vars, sizeof(var) * (*max_len_vars) * 2);
      *max_len_vars *= 2;
    }
    if (vars == NULL)
    {
      fprintf(stderr, "ERROR: resizing vars array to %d\n", *max_len_vars);
      return -1;
    }

    // determine type
    vars[i_vars].type = (line[0] == 'd') ? DUALRAIL : BOOL;

    // parse line
    i = 0;
    while (i < len && line[i++] != ' ');
    // // printf("--> Found varname (type %c) w/ 1st lett: %c ... ", line[0], line[i]);

    // get variable name
    vars[i_vars].varname = (char *) malloc(sizeof(char) * (len-i) + 1);
    if (vars[i_vars].varname == NULL)
    {
      fprintf(stderr, "ERROR: could not allocate mem for varname\n");
    }
    j = 0;
    while (i < len && line[i] != ' ' && line[i] != '[')
    {
      vars[i_vars].varname[j] = line[i];
      // // printf("... i=%d, j=%d ", i, j);
      i++;
      j++;
    }
    vars[i_vars].varname[j] = '\0';
    // printf("--> Varname == %s\n", vars[i_vars].varname);

    while (line[i] == ' ') { i++; };
    // printf("--> lett is now @ %d = %c\n", i, line[i]);

    // get array length, if applicable
    if (line[i] == '[')
    {
      i++;
      j=0;
      vars[i_vars].len_var_array = 0;
      while (line[i] != ']')
      {
        vars[i_vars].len_var_array *= 10;
        vars[i_vars].len_var_array += line[i] - '0';
        // printf("===> size=%d\n", vars[i_vars].len_var_array);
        i++;
        j++;
        if (i >= len)
        {
          fprintf(stderr, "ERROR: incomplete line %s\n", line);
          return -1;
        }
      }

      i+=2; // skip ] & space
    }
    else
    {
      vars[i_vars].len_var_array = 1;
    }
    // printf("--> Size of var is: %d\n", vars[i_vars].len_var_array);

    // create list of values:
    // printf("--> Creating list values of size %ld\n", len-i);
    vars[i_vars].values = (int **) malloc(sizeof(int *) * (len-i));
    if (vars[i_vars].values == NULL)
    {
      fprintf(stderr, "ERROR: couldn't alloc mem for values array\n");
      return -1;
    }

    while (line[i] == ' ') { i++; }
    // printf("--> lett is now @ %d = %c\n", i, line[i]);

    vars[i_vars].len_values = 0;
    k=0;
    bool end_loop = false;
    while (i < len && line[i] != '\0' && end_loop == false)
    {
      // printf("-->loop, i=%d, k=%d,len=%ld, line[i]=%d=%c\n", i, k, len, line[i], line[i]);
      vars[i_vars].values[vars[i_vars].len_values] = (int *) malloc(sizeof(int) * vars[i_vars].len_var_array);

      if (vars[i_vars].values[vars[i_vars].len_values] == NULL)
      {
        fprintf(stderr, "ERROR: couldn't alloc mem for inner values array\n");
        return -1;
      }

      while (k < vars[i_vars].len_var_array)
      {
        // skip unusable characters
        while (i < len && line[i] != '0' && line[i] != '1' && line[i] != 'n' && line[i] != 'r' && line[i] != '\0') {
          // printf("--> -- skip %c\n", line[i]);
          i++;
        }

        // break if found end of line
        if (line[i] == '\0')
        {
          // printf("need to end loop\n");
          end_loop = true;
          break;
        }

        // otherwise add value to array
        if (line[i] == 'r')
        {
          vars[i_vars].values[vars[i_vars].len_values][k] = -2;
        }
        else if (line[i] == 'n')
        {
          vars[i_vars].values[vars[i_vars].len_values][k] = -1;
        }
        else
        {
          vars[i_vars].values[vars[i_vars].len_values][k] = line[i] - '0';
        }

        // printf("----> adding %d\n", vars[i_vars].values[vars[i_vars].len_values][k]);

        // detect
        if (vars[i_vars].values[vars[i_vars].len_values][k] < -2 || vars[i_vars].values[vars[i_vars].len_values][k] > 1)
        {
          fprintf(stderr, "ERROR: invalid variable value %d from %c\n", vars[i_vars].values[vars[i_vars].len_values][k], line[i]);
          return -1;
        }
        k++;
        i++;
        if (line[i] == '\0')
        {
          // printf("need to end loop2\n");
          end_loop = true;
        }
      }

      vars[i_vars].len_values++;
      // printf("----> len_values = %d\n", vars[i_vars].len_values);
      k=0;
    }

    vars[i_vars].len_values--; // because was incremented on last loop that found \0
    if (vars[i_vars].len_values > vars[vars->num_cycles].len_values)
    {
      vars->num_cycles = i_vars;
    }

    // move on to next variable
    i_vars++;

  }

  *len_vars = i_vars;
  printf("final len vars count = %d\n\n", *len_vars);
  return 1;
}

void print_bool(FILE* f, char * prefix, char * varname, int arr_index, int val)
{
  if (val >=0 && arr_index >= 0)    // set array index value
  {
    fprintf(f, "%s %s[%d] %d\n", prefix, varname, arr_index, val);
  }
  else if (val >=0)               // set non-array variable value
  {
    fprintf(f, "%s %s %d\n", prefix, varname, val);
  }
  else if (arr_index >= 0)        // array, non-value
  {
    fprintf(f, "%s %s[%d]\n", prefix, varname, arr_index);
  }
  else                            // non-array, non-value
  {
    fprintf(f, "%s %s\n", prefix, varname);
  }
}

void print_dualrail(FILE* f, char * prefix, char * varname, int arr_index, int tval, int fval)
{
  if (tval >=0 && fval >=0 && arr_index >= 0)   // set array index value
  {
    fprintf(f, "%s %s[%d].t %d\n", prefix, varname, arr_index, tval);
    fprintf(f, "%s %s[%d].f %d\n", prefix, varname, arr_index, fval);
  }
  else if (tval >=0 && fval >=0)              // set non-array variable value
  {
    fprintf(f, "%s %s.t %d\n", prefix, varname, tval);
    fprintf(f, "%s %s.f %d\n", prefix, varname, fval);
  }
  else if (arr_index >=0)                     // array, non-value
  {
    fprintf(f, "%s %s[%d].t\n", prefix, varname, arr_index);
    fprintf(f, "%s %s[%d].f\n", prefix, varname, arr_index);
  }
  else                                        // non-array, non-value
  {
    fprintf(f, "%s %s.t\n", prefix, varname);
    fprintf(f, "%s %s.f\n", prefix, varname);
  }
}

void print_prsim_test(FILE * f, var * vars, int len_vars, bool random, bool break_on_warn, bool reset, bool _reset)
{
  int v, a, l;
  // print header lines
  fprintf(f, "initialize\n");
  if (random)
  {
    fprintf(f, "random\n");
  }
  if (break_on_warn)
  {
    fprintf(f, "break-on-warn\n");
  }
  if (reset)
  {
    fprintf(f, "watch Reset\n");
  }
  if (_reset)
  {
    fprintf(f, "watch _Reset\n");
  }

  // Watch Variables
  for (v=0; v<len_vars; v++)                // each var
  {
    for (a=0; a<vars[v].len_var_array; a++) // each index of var array
    {
      int arr_index = (vars[v].len_var_array == 1) ? -1 : a;
      switch (vars[v].type) {
        case BOOL:
          print_bool(f, "watch", vars[v].varname, arr_index, -1);
          break;
        case DUALRAIL:
          print_dualrail(f, "watch", vars[v].varname, arr_index, -1, -1);
          break;
        default:
          printf("Encountered unknown type\n");
          // do nothing
      }
    }
  }
  fprintf(f, "\n");

  // Reset
  fprintf(f, "mode reset\n");
  if (reset)
  {
    fprintf(f, "set Reset 1\n");
  }
  if (_reset)
  {
    fprintf(f, "set _Reset 0\n");
  }

  for (v=0; v<len_vars; v++)                              // each var
  {
    if (vars[v].len_values > 0 && vars[v].values[0][0] >= 0)    // only reset vars that are set to a val
    {

      for (a=0; a<vars[v].len_var_array; a++)               // each index of var array
      {
        int arr_index = (vars[v].len_var_array == 1) ? -1 : a;

        switch (vars[v].type) {
          case BOOL:
            print_bool(f, "set", vars[v].varname, arr_index, 0);
            break;
          case DUALRAIL:
            print_dualrail(f, "set", vars[v].varname, arr_index, 0, 0);
            break;
          default:
            // do nothing
            printf("Encountered unknown type\n");
        }
      }
    }
  }
  fprintf(f, "cycle\n");

  // End Reset
  for (v=0; v<len_vars; v++)                              // each var
  {
    for (a=0; a<vars[v].len_var_array; a++)               // each index of var array
    {
      int arr_index = (vars[v].len_var_array == 1) ? -1 : a;

      switch (vars[v].type) {
        case BOOL:
          print_bool(f, "assert", vars[v].varname, arr_index, 0);
          break;
        case DUALRAIL:
          print_dualrail(f, "assert", vars[v].varname, arr_index, 0, 0);
          break;
        default:
          // do nothing
          printf("Encountered unknown type\n");
      }
    }
  }

  if (reset)
  {
    fprintf(f, "set Reset 0\n");
  }
  if (_reset)
  {
    fprintf(f, "set _Reset 1\n");
  }
  fprintf(f, "cycle\n");
  fprintf(f, "mode run\n");


  // Set Values, Cycle, & Assert
  for (l=0; l<vars[vars->num_cycles].len_values; l+=2)   // highest num of set/out value
  {
    for (v=0; v<len_vars; v++)                              // each var
    {
      // printf("v=%s l=%d max=%d\n", vars[v].varname, l, vars[v].len_values);

      if (l < vars[v].len_values)
      {
        // printf("---> Setting %s\n", vars[v].varname);

        for (a=0; a<vars[v].len_var_array; a++)               // each index of var array
        {
          int arr_index = (vars[v].len_var_array == 1) ? -1 : a;
          switch (vars[v].type) {
            case BOOL:
              if (vars[v].values[l][a] >= 0)                    // skip "n", don't assert values
              {
                print_bool(f, "set", vars[v].varname, arr_index, vars[v].values[l][a]);
              }
              break;
            case DUALRAIL:
              if (vars[v].values[l][a] >= 0)                    // skip "n", don't assert values
              {
                print_dualrail(f, "set", vars[v].varname, arr_index, vars[v].values[l][a], 1-vars[v].values[l][a]);
              }
              else if (vars[v].values[l][a] == -2)            // 'r' means reset dualrail
              {
                print_dualrail(f, "set", vars[v].varname, arr_index, 0, 0);
              }
              break;
            default:
              // do nothing
              printf("Encountered unknown type\n");
          }
        }
      }
    }
    fprintf(f, "cycle\n\n");
    // Assert Set Value is Outputted
    for (v=0; v<len_vars; v++)                              // each var
    {
      if (l+1 < vars[v].len_values)
      {
        for (a=0; a<vars[v].len_var_array; a++)               // each index of var array
        {
          int arr_index = (vars[v].len_var_array == 1) ? -1 : a;
          switch (vars[v].type) {
            case BOOL:
              if (vars[v].values[l+1][a] >= 0)                    // skip "n", don't assert values
              {
                print_bool(f, "assert", vars[v].varname, arr_index, vars[v].values[l+1][a]);
              }
              break;
            case DUALRAIL:
              if (vars[v].values[l+1][a] >= 0)                    // skip "n", don't assert values
              {
                print_dualrail(f, "assert", vars[v].varname, arr_index, vars[v].values[l+1][a], 1-vars[v].values[l+1][a]);
              }
              else if (vars[v].values[l+1][a] == -2)            // 'r' means reset dualrail
              {
                print_dualrail(f, "assert", vars[v].varname, arr_index, 0, 0);
              }
              break;
            default:
              // do nothing
              printf("Encountered unknown type\n");
          }
        }
      }
    }
  }
}

void print_vars(var * vars, int len_vars)
{
  for (int i=0; i<len_vars; i++)
  {
    printf("%s = type %d, array size %d, num vals %d:\n= { ", vars[i].varname, vars[i].type, vars[i].len_var_array, vars[i].len_values);
    for (int l=0; l<vars[i].len_values; l++)
    {
      printf("[");
      for(int m=0; m<vars[i].len_var_array; m++)
      {
        printf("%d", vars[i].values[l][m]);
      }
      printf("] ");
    }
    printf("}\n");
  }

}

int main (int argc, char * argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s  input_file  output_file [--random] [--break-on-warn] [--reset] [--_reset]\n", argv[0]);
    return 0;
  }

  /* *** OPEN FILES *** */
  FILE * in = fopen(argv[1], "r");
  FILE * out = fopen(argv[2], "w");
  if (in == NULL || out == NULL)
  {
    fprintf(stderr, "ERROR: cannot open %s", (in == NULL ? argv[1] : argv[2]));
    return 0;
  }

  /* *** GET ARGS *** */
  bool random = false;
  bool bow = false;
  bool reset = false;
  bool _reset = false;

  for (int i=3; i<argc; i++)
  {
    if (strcmp(argv[i], "--random") == 0)
    {
      random = true;
    }
    else if (strcmp(argv[i], "--break-on-warn") == 0)
    {
      bow = true;
    }
    else if (strcmp(argv[i], "--reset") == 0)
    {
      reset = true;
    }
    else if (strcmp(argv[i], "--_reset") == 0)
    {
      _reset = true;
    }
  }

  int len_vars = 0;
  int max_len_vars = 10;
  var * vars = (var *) malloc(sizeof(var) * max_len_vars);
  if (vars == NULL)
  {
    fprintf(stderr, "ERROR: could not malloc vars array\n");
    return -1;
  }
  vars->num_cycles = 0;

  /* *** READ FILE *** */
  int read_err = read_file(in, vars, &len_vars, &max_len_vars);

  if (read_err < 0)
  {
    return -1;
  }

  // see variables array
  print_vars(vars, len_vars);

  // print out prsim file
  print_prsim_test(out, vars, len_vars, random, bow, reset, _reset);

  return 1;
}

/*
    bool/dualrail varname[size of array], setup value, expected_output
    dualrail d[12] 0000000000000001, 000000000000000
*/
