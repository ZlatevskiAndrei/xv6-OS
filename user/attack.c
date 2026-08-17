#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define HGESIZE (126 * 1024 * 1024)

void
printataddr(void *addr)
{
    printf("%s\n", (char *)addr);
}

int
main(int argc, char *argv[])
{
    char *start = sbrk(HGESIZE);

    if (start == (char *)-1) {
        printf("sbrk failed\n");
        exit(1);
    }

    for (char *p = start; p < start + HGESIZE - 3; p++) {
        if (p[0] == 'h' &&
            p[1] == 'e' &&
            p[2] == 'h' &&
            p[3] == 'e') {
            printataddr(p);
        }
    }
    exit(0);
}
