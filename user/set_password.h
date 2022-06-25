#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "user/md5.h"
#include "rand.h"

#define MAX_BUFFER_SIZE 100
#define HASH_LENGTH 32
#define SALT_LENGTH 18

//文字列にソルトを追加
void addSalt(char buf[MAX_BUFFER_SIZE], char salt[SALT_LENGTH]) {
  for(int i = 0; i < SALT_LENGTH; i++){
    buf[strlen(buf)-1 + i] = salt[i];
  }
}

void setPassword(int fd) { //パスワードの設定

  char newPassword[MAX_BUFFER_SIZE];
  char confirmedPassword[MAX_BUFFER_SIZE];
  char user_name[MAX_BUFFER_SIZE];
  char salt[SALT_LENGTH] = {}; 

  // user名を設定
  write(1, "Enter New Username : ", 22);
  gets(user_name, MAX_BUFFER_SIZE);
  user_name[strlen(user_name)-1] = 0; //改行を消す

  // prompt to enter password
  write(1, "Enter New Password : ", 22);
  gets(newPassword, MAX_BUFFER_SIZE);
  write(1, "Enter Again : ", 15);
  gets(confirmedPassword, MAX_BUFFER_SIZE);

  // check that the two passwords match
  if (strcmp(newPassword, confirmedPassword) == 0) { // passwords match, proceed
    write(1, "Password Is Set Successfully \n", 31);

    
    genSalt(salt, SALT_LENGTH);
    //文字列にソルトを追加
    addSalt(confirmedPassword, salt);

    char password_hash[HASH_LENGTH];
    getmd5(confirmedPassword, strlen(confirmedPassword), password_hash);

    //ファイルにuser:salt:passwordを格納
    // TODO: 1行をMAX_BUFFER_SIZEと保証したい、またはreadで読むサイズ以下にしたい
    write(fd, user_name , strlen(user_name));
    write(fd, ":", 1);
    write(fd, salt, SALT_LENGTH);
    write(fd, ":", 1);
    write(fd, password_hash, HASH_LENGTH);
    write(fd, "\n", 1);
    for (int i = 0; i < MAX_BUFFER_SIZE - SALT_LENGTH - HASH_LENGTH - strlen(user_name) - 3; i++) {
      write(fd, "", 1);
    }
    close(fd);
    
    return;
  }

  else { // passwords do not match
    printf("Passwords Do Not match. Try Again.\n", 36);
    setPassword(fd);
  }

}
