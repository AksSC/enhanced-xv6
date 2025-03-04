// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"
// #include "kernel/fcntl.h"

// #define NFORK 5
// #define IO 2

// int main()
// {
//   int n, pid;
//   int wtime, rtime;
//   int twtime = 0, trtime = 0;
//   for (n = 0; n < NFORK; n++)
//   {
//     pid = fork();
//     if (pid < 0)
//       break;
//     if (pid == 0)
//     {
//       if (n < IO)
//       {
//         sleep(200); // IO bound processes
//       }
//       else
//       {
//         for (volatile int i = 0; i < 1000000000; i++)
//         {
//         } // CPU bound process
//       }
//       // printf("Process %d finished\n", getpid());
//       exit(0);
//     }
//   }
//   for (; n > 0; n--)
//   {
//     if (waitx(0, &wtime, &rtime) >= 0)
//     {
//       trtime += rtime;
//       twtime += wtime;
//     }
//   }
//   printf("Average rtime %d,  wtime %d\n", trtime / NFORK, twtime / NFORK);
//   exit(0);
// }

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define NFORK 5
#define IO 2

int main() {
  int n, pid;
  int wtime, rtime;
  int twtime = 0, trtime = 0;
  for(n = 0; n < NFORK; n++) {
    pid = fork();
    if (pid < 0)
      break;
    if (pid == 0) {
      if (n < IO) {
        for (volatile int i = 0; i < 100; i++) {
          sleep(1);
          for(volatile int j = 0; j < 10000000; j++) {}
        }
      } else {
        for (volatile int i = 0; i < 100; i++) {
          for (volatile int j = 0; j < 10000000; j++) {}
        }
      }
      printf("Process %d finished\n", getpid());
      exit(0);
    }
  }
  for(; n > 0; n--) {
    if(waitx(0, &wtime, &rtime) >= 0) {
      trtime += rtime;
      twtime += wtime;
    } 
  }
  printf("Average rtime %d,  wtime %d\n", trtime / NFORK, twtime / NFORK);
  exit(0);
}