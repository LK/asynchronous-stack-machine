#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int read_varname(char * line, int i, char * varname)
{
  int v = 0;
  while (line[i] != ' ')
  {
    if (line[i] == '.')
    {
      varname[v] = '/';
    }
    else if (line[i] == '[' || line[i] == ']')
    {
      varname[v] = '\\';
      v++;
      varname[v] = line[i];
    }
    else
    {
      varname[v] = line[i];
    }
    i++; v++;
  }
  varname[v] = '\0';
  i++;
  printf("--- found varname %s, w/ value %c\n", varname, line[i]);
  return i;
}

int main (int argc, char * argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s  input_file  output_file\n", argv[0]);
    return 0;
  }

  /* *** OPEN FILES *** */
  FILE * in = fopen(argv[1], "r");
  FILE * out = fopen(argv[2], "w");
  if (in == NULL || out == NULL)
  {
    fprintf(stderr, "ERROR: cannot open %s\n", (in == NULL ? argv[1] : argv[2]));
    return 0;
  }

  // preamble
  fprintf(out, "ana go/r go/a Reset _Reset\n\n");
  fprintf(out, "l GND!\nh Vdd!\n");

  int i, v;
  char * line = NULL;
  size_t len = 0;
  while (getline(&line, &len, in) != -1)
  {
    char varname[(int)len];
    v = 0;
    // set
    if (line[0] == 's' && line[1] == 'e' && line[2] == 't' && line[3] == ' ')
    {
      i = read_varname(line, 4, varname);
      fprintf(out, "%c %s\n", (line[i] == '0') ? 'l' : 'h', varname);
    }
    // cycle
    else if (line[0] == 'c' && line[1] == 'y' && line[2] == 'c' && line[3] == 'l' && line[4] == 'e')
    {
      fprintf(out, "s\n");
    }
    // assert
    else if (line[0] == 'a' && line[1] == 's' && line[2] == 's' && line[3] == 'e' && line[4] == 'r' && line[5] == 't')
    {
      i = read_varname(line, 7, varname);
      fprintf(out, "assert %s %c\n", varname, line[i]);
    }

    // ignore all else
  }

  return 1;
}
