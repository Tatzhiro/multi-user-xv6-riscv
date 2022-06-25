#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/md5.h"
#include "user/set_password.h"

#define MAX_BUFFER_SIZE 100
#define HASH_LENGTH 32
#define SALT_LENGTH 18

int main(void){

  int i, j, k;
  char input_user[MAX_BUFFER_SIZE]= {};
  char input_password[MAX_BUFFER_SIZE]={};
  char user_pass_salt_triplet[MAX_BUFFER_SIZE]= {};
  char read_buf[MAX_BUFFER_SIZE] = {};
  int loop_count = 0;
  char newPassword[MAX_BUFFER_SIZE];
  char confirmedPassword[MAX_BUFFER_SIZE];
  int user_name_length;
  char salt[SALT_LENGTH] = {};


  // open passwords file
  int fd = open("Passwords", O_RDONLY);


  // prompt user for password
  write(1, "Enter Username : ", 18);
  gets(input_user, MAX_BUFFER_SIZE);

  user_name_length = strlen(input_user);

  input_user[user_name_length - 1] = 0; //input_userの語尾の改行を消す。


  while(read(fd, user_pass_salt_triplet, MAX_BUFFER_SIZE)){


    char saved_user[MAX_BUFFER_SIZE] = {};
    char saved_password[MAX_BUFFER_SIZE] ={};

    //Passwordsファイルからuserとsaltとpasswordを抽出。
    {
      for(i = 0; user_pass_salt_triplet[i] != ':'; i++){
        saved_user[i] = user_pass_salt_triplet[i];
      }
      i++; // ':'を飛び越える

      for(j = 0; user_pass_salt_triplet[j+i] != ':'; j++){
        salt[j] = user_pass_salt_triplet[j+i];
      }
      j++; // ':'を飛び越える

      for(k = 0; user_pass_salt_triplet[k+j+i] != '\n'; k++){
        saved_password[k] = user_pass_salt_triplet[k+j+i];
      }
    }

    if(strncmp(input_user, saved_user, user_name_length) == 0){ //ユーザ名と保存された名前が等しい場合

      write(1, "Enter Password : ", 18);
      gets(input_password, MAX_BUFFER_SIZE);

      //入力したパスワードに、ソルトを追加
      addSalt(input_password, salt);

      char password_hash[HASH_LENGTH] = {};
      getmd5(input_password, strlen(input_password), password_hash);


      while(strncmp(saved_password, password_hash, HASH_LENGTH) != 0) { // パスワードが違う場合
        write(1, "Incorrect Password.\nPlease enter correct Password : ", 53);
        gets(input_password, MAX_BUFFER_SIZE);

        //ソルトを追加
        addSalt(input_password, salt);
        getmd5(input_password, strlen(input_password), password_hash);
      }

      if (strncmp(saved_password, password_hash, HASH_LENGTH) == 0) { // パスワードが正しい場合
        printf("Change Password\n");
        int fd = open("Passwords", O_RDWR);

        //既存のuser:passwordに新しいuser_passwordを上書きする。
        for(i = 0; i<loop_count; i++){
          read(fd, read_buf, MAX_BUFFER_SIZE);
        }

        // 新しいパスワードの入力
        write(1, "Enter New Password : ", 22);
        gets(newPassword, MAX_BUFFER_SIZE);
        write(1, "Enter Again : ", 15);
        gets(confirmedPassword, MAX_BUFFER_SIZE);

        // check that the two passwords match
        if (strncmp(newPassword, confirmedPassword, HASH_LENGTH) == 0) { // passwords match, proceed
          write(1, "You Can Change Password\n", 31);

          //新しいソルトを生成して、パスワードに付与する。
          genSalt(salt, SALT_LENGTH, strlen(confirmedPassword));
          addSalt(confirmedPassword, salt);

          char password_hash[HASH_LENGTH] ={};
          getmd5(confirmedPassword, strlen(confirmedPassword), password_hash);

          //ファイルにuser:passwordを格納(1行、max_buffer_size)
          write(fd, saved_user, user_name_length);
          write(fd, ":", 1);
          write(fd, salt, SALT_LENGTH);
          write(fd, ":", 1);
          write(fd, password_hash, HASH_LENGTH);
          write(fd, "\n", 1);
          for (int i = 0; i < MAX_BUFFER_SIZE - SALT_LENGTH - HASH_LENGTH - user_name_length - 3; i++) {
            write(fd, "", 1);
          }
          close(fd);

          exit(0);

        }
        else{
          printf("Password Is Not Same\n");
          exit(1);
        }
      }
    }
    loop_count++;
  }
  printf("user is not found\n");
  exit(1);
}