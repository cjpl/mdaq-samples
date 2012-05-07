#include <stdio.h>
#include <stdlib.h>

#include "evtgen.h"

int main(int argc, char** argv)
{
  WORD p[4];
  int i;

  printf("Init generator...\n");
  struct evtgen *evt = c_init_gen();
  printf("Get Random event:\n");
  for(i=0; i<900; i++) {
    c_gen_event(evt, p, 4, 1024);
    printf("[%03d]: %4d  %4d  %4d  %4d\n",i,p[0],p[1],p[2],p[3]);
  }

  return 0;
}
