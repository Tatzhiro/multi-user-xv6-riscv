// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "user/set_password.h"
#include "user/md5.h"
#include "rand.h" 

char *argv[] = { "sh", 0 };

void
makeShell(int uid) {
  int pid, wpid;
  pid = fork();
  if(pid < 0){
    printf("makeShell: fork failed\n");
    exit(1);
  }
  if(pid == 0){
    setuid(uid);
    exec("sh", argv);
    printf("makeShell: exec sh failed\n");
    exit(1);
  }
  for(;;){
      // this call to wait() returns if the shell exits,
      // or if a parentless process exits.
      wpid = wait((int *) 0);
      if(wpid == pid){
        // the shell exited; restart it.
        write(1, "user logged out\n\n", 18);
        break;
      } else if(wpid < 0){
        printf("init: wait returned an error\n");
        exit(1);
      } else {
        // it was a parentless process; do nothing.
      }
  }
}

int
main(void) 
{

  if(open("console", O_RDWR) < 0){ 
    mknod("console", CONSOLE, 0);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  // password
  int fd = open("Passwords", O_RDWR);
  if (fd < 0) { // no password
    close(fd);
    write(1, "No user\nPlease sign up as root\n", 32);
    fd = open("Passwords", O_CREATE|O_RDWR);

    // 最初のユーザーは全権を持つroot
    setPassword(fd, 0);
  }


  for(;;){
    int uid = 0;
    int null;
    login(&uid, &null);
    write(1, "Logging in...\n\n", 16); // correct password
    printf("init: starting sh\n");
    makeShell(uid);
  }
}