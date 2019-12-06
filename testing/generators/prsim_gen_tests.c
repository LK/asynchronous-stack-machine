#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main ( int argc, char * argv[] )
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s  input_file  output_file\n");
    return 0;
  }

  FILE * in = fopen(argv[1], "r");
  FILE * out = fopen(argv[2], "w");
  if (in == NULL || out == NULL)
  {
    fprintf(stderr, "Error: cannot open %s", (in == NULL ? argv[1] : argv[2]));
    return 0;
  }

  char * line = NULL;
  size_t len = 0;
  while(getline(&line, &len, in) != -1)
  {
    fprintf(out, "%s", line);
    char word[len];
    
  }

  return 1;
}

/*
    bool/dualrail varname[size of array], setup value, expected_output
*/
