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

#define MAX_BUFFER_SIZE 100
#define HASH_LENGTH 32 

char *argv[] = { "sh", 0 };

void login() { //パスワードの入力と、照合

  int i, j, k;
  char input_user[MAX_BUFFER_SIZE]= {};
  char input_password[MAX_BUFFER_SIZE]={}; 
  char user_salt_pass_triplet[MAX_BUFFER_SIZE]= {};
  char salt[SALT_LENGTH] = {};

  

  // open passwords file
  int fd = open("Passwords", O_RDONLY);


  // prompt user for password
  write(1, "Enter Username : ", 18);
  gets(input_user, MAX_BUFFER_SIZE);


  input_user[strlen(input_user)-1] = 0; //input_userの語尾の改行を消す。

  while(read(fd, user_salt_pass_triplet, MAX_BUFFER_SIZE)){
    write(1, "read\n", 6);

    char saved_user[MAX_BUFFER_SIZE] = {};
    char saved_password[MAX_BUFFER_SIZE] ={};
    
      //Passwordsファイルからuserとsaltとpasswordを抽出。
    { 
      for(i = 0; user_salt_pass_triplet[i] != ':'; i++){
        saved_user[i] = user_salt_pass_triplet[i];
      }
      i++; // ':'を飛び越える

      for(j = 0; user_salt_pass_triplet[j+i] != ':'; j++){
        salt[j] = user_salt_pass_triplet[j+i];
      }
      j++; // ':'を飛び越える

      for(k = 0; user_salt_pass_triplet[k+j+i] != '\n'; k++){
        saved_password[k] = user_salt_pass_triplet[k+j+i];
      }
    }

    if(strncmp(input_user, saved_user, strlen(input_user)) == 0){
      while (1) {
        write(1, "Enter Password : ", 18);
        gets(input_password, MAX_BUFFER_SIZE);

        addSalt(input_password, salt);

        char password_hash[HASH_LENGTH] = {};
        getmd5(input_password, strlen(input_password), password_hash);
        if (strncmp(saved_password, password_hash, HASH_LENGTH) == 0) break;
        write(1, "Incorrect Password.\n", 21);
      }
      
      write(1, "Logging in...\n\n", 16); // correct password
      close(fd);
      return;
    }
  }
  printf("You Are Not Registered\n");
  login();
}



int
main(void)
{
  int pid, wpid;

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
    write(1, "No Password set.\nPlease set your Password\n", 43);
    fd = open("Passwords", O_CREATE|O_RDWR);
    setPassword(fd);
  }


  for(;;){
    login();
    printf("init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf("init: fork failed\n");
      exit(1);
    }
    if(pid == 0){
      exec("sh", argv);
      printf("init: exec sh failed\n");
      exit(1);
    }

    for(;;){
      // this call to wait() returns if the shell exits,
      // or if a parentless process exits.
      wpid = wait((int *) 0);
      if(wpid == pid){
        // the shell exited; restart it.
        break;
      } else if(wpid < 0){
        printf("init: wait returned an error\n");
        exit(1);
      } else {
        // it was a parentless process; do nothing.
      }
    }
  }

}