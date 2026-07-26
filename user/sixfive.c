#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"


void check_mod(uint* i) {
    if(*i % 5 == 0 || *i % 6 == 0) fprintf(1, "%d\n", *i);
    *i = 0;
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        fprintf(2, "sixfive file1 file2 ... fileN \n");
        exit(1);
    }
    
    for (; argc > 1; argc--){
        int fd = open(argv[argc-1], O_RDONLY);
        if(fd == -1) {
            fprintf(2, "cannot open file %s\n", argv[argc-1]);
            exit(1);
        }
        char c[1];
        uint n;
        uint res = 0;
        while(1){ 
            n = read(fd, c, 1);
            if(n == 0) {
                check_mod(&res);
                break;
            }
            if('0' <= c[0] && c[0] <= '9'){
                res = res*10 + c[0] - '0';
                continue;
            }
            if(strchr(c, '-') || strchr(c, '\r') || strchr(c, '\t') || strchr(c, '\n') || strchr(c, '.') || strchr(c, ',') || strchr(c, '/')){
                check_mod(&res);
                continue;
            }
        }
        close(fd);
    }
}