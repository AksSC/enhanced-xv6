#include "kernel/types.h"
#include "kernel/syscall.h"
#include "kernel/stat.h"
#include "user/user.h"

// does this also count successfully for child processes spawned by args?

char* syscalls[] = {"fork", "exit", "wait", "pipe", "read", "kill", "exec", "fstat", "chdir", "dup", "getpid", "sbrk", "sleep", "uptime", "open", "write", "mknod", "unlink", "link", "mkdir", "close", "waitx", "getSysCount", "sigalarm", "sigreturn", "settickets"};

int main(int argc, char *argv[]){
    if(argc < 3){
        fprintf(2, "Usage: syscount <mask> command [args]\n");
        exit(1);
    }

    int mask = atoi(argv[1]);

    if(mask & (mask - 1)){
        fprintf(2, "Mask must be a power of 2\n");
        exit(1);
    }

    int pid = fork();
    if(pid < 0){
        fprintf(2, "Fork failed\n");
        exit(1);
    }
    if(pid == 0){
        exec(argv[2], &argv[2]); // execute given cmd
        fprintf(2, "Exec failed\n");
        exit(1);
    }else{
        int idk = wait(0);
        if(idk < 0){
            fprintf(2, "Wait failed\n");
            exit(1);
        }

        int sysc = 0;
        for(int i = 0; i < 32; i++)
        {
            if(mask & (1 << i)){
                sysc = i; break;
            }
        }

        int count = getSysCount(sysc);
        if(count < 0){
            fprintf(2, "Invalid syscall count\n");
            exit(1);
        }

        printf("PID %d called %s %d times.\n", pid, syscalls[sysc-1], count);
    }

    exit(0);
}
