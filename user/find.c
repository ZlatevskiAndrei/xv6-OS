#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

void find(char *path, char *filename, int cmdargc, char *cmdargv[])
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
        fprintf(2, "find: path too long\n");
        close(fd);
        return;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0)
            continue;

        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;

        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;

        if (stat(buf, &st) < 0) {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }

        switch (st.type) {
        case T_DEVICE:
        case T_FILE:
            if (strcmp(de.name, filename) == 0) {
                if (cmdargv == 0) {
                    printf("%s\n", buf);
                } else {
                    char *argv[MAXARG];
                    int i;

                    for (i = 0; i < cmdargc; i++)
                        argv[i] = cmdargv[i];

                    argv[cmdargc] = buf;
                    argv[cmdargc + 1] = 0;

                    if (fork() == 0) {
                        exec(argv[0], argv);
                        fprintf(2, "find: exec %s failed\n", argv[0]);
                        exit(1);
                    }
                    wait(0);
                }
            }
            break;

        case T_DIR:
            find(buf, filename, cmdargc, cmdargv);
            break;
        }
    }

    close(fd);
}

int
main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(2, "usage: find path filename [-exec command ...]\n");
        exit(1);
    }

    if (argc == 3) {
        find(argv[1], argv[2], 0, 0);
    } else {
        if (strcmp(argv[3], "-exec") != 0) {
            fprintf(2, "usage: find path filename [-exec command ...]\n");
            exit(1);
        }

        find(argv[1], argv[2], argc - 4, &argv[4]);
    }

    exit(0);
}



