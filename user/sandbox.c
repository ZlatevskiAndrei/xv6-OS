#include <stddef.h>
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int mask;
  char* path;

  if(argc < 4){
    printf("usage: sandbox mask command [args...]\n");
    exit(0);
  }

  mask = atoi(argv[1]);
  path = argv[2];

  if(fork() == 0){
    interpose(mask, path);

    exec(argv[3], &argv[3]);

    printf("sandbox: exec %s failed\n", argv[3]);
    exit(0);
  }

  wait(NULL);
  exit(0);
}