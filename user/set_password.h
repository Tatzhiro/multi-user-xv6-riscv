#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "user/md5.h"
#include "rand.h"

#define MAX_BUFFER_SIZE 100
#define HASH_LENGTH 32
#define SALT_LENGTH 18
#define MAX_UID_DIGIT 10

void addSalt(char buf[MAX_BUFFER_SIZE], const char salt[SALT_LENGTH]);
int login(int*, int*);
void setPassword(int fd, int uid);

// bufにソルトを追加
void addSalt(char buf[MAX_BUFFER_SIZE], const char salt[SALT_LENGTH]) {
  int buf_length = strlen(buf); //繰り返しの中でstrlenを使うとバグる
  for(int i = 0; i < SALT_LENGTH; i++){
    buf[buf_length - 1 + i] = salt[i];
  }
}

/**
 * @brief パスワードの入力と、照合
 * 
 * @return userの行番号: change_passwordで必要
 */
int login(int *myuid, int *row_count) {
  printf("login function\n");
  int h, i, j, k;
  char input_user[MAX_BUFFER_SIZE]= {}; 
  char uid_user_salt_pass_quadruple[MAX_BUFFER_SIZE]= {};
  int user_row = 0;

  // open passwords file
  int fd = open("Passwords", O_RDONLY);
  if(fd < 0) {
    printf("LOGIN ERROR: Passwords cannot be opened by non-root user\n");
    return 1;
  }

  // prompt user for password
  write(1, "\nPlease login\n", 15);
  write(1, "Enter Username : ", 18);
  gets(input_user, MAX_BUFFER_SIZE);


  input_user[strlen(input_user)-1] = 0; //input_userの語尾の改行を消す。

  while(read(fd, uid_user_salt_pass_quadruple, MAX_BUFFER_SIZE)){
    
    char saved_user[MAX_BUFFER_SIZE] = {};
    char salt[SALT_LENGTH] = {};
    char saved_password[MAX_BUFFER_SIZE] ={};

      //Passwordsファイルからuidとuserとsaltとpasswordを抽出。
    char str_uid[MAX_UID_DIGIT] = {};
    for(h = 0; uid_user_salt_pass_quadruple[h] != ':'; h++) {
      str_uid[h] = uid_user_salt_pass_quadruple[h];
    }
    int uid = atoi(str_uid);
    h++;
    for(i = 0; uid_user_salt_pass_quadruple[i+h] != ':'; i++){
      saved_user[i] = uid_user_salt_pass_quadruple[i+h];
    }
    i++; // ':'を飛び越える
    for(j = 0; uid_user_salt_pass_quadruple[j+i+h] != ':'; j++){
      salt[j] = uid_user_salt_pass_quadruple[j+i+h];
    }
    j++; // ':'を飛び越える
    
    for(k = 0; uid_user_salt_pass_quadruple[k+j+i+h] != '\n'; k++){
      saved_password[k] = uid_user_salt_pass_quadruple[k+j+i+h];
    }

    if(strlen(saved_user) == strlen(input_user) && strncmp(input_user, saved_user, strlen(saved_user)) == 0){
      while (1) {
        write(1, "Enter Password : ", 18);
        char input_password[MAX_BUFFER_SIZE]={};   //input_passwordは繰り返しのたびに初期化しないと、前のpasswordが残っている。
        gets(input_password, MAX_BUFFER_SIZE);

      
        addSalt(input_password, salt);
        char password_hash[HASH_LENGTH] = {};
        getmd5(input_password, strlen(input_password), password_hash);

        if (strncmp(saved_password, password_hash, HASH_LENGTH) == 0) { 
          *myuid = uid;
          break;
        }
        write(1, "Incorrect Password.\n", 21);

      }
      
      close(fd);
      *row_count = user_row;
      return 0;
    }
    user_row++;
  }
  printf("You Are Not Registered\n");
  close(fd);
  return login(myuid, row_count);
}

void setPassword(int fd, int uid) { //パスワードの設定

  char save_uid[MAX_UID_DIGIT] = {}; //char saltの隣に置くとバグる
  char newPassword[MAX_BUFFER_SIZE];
  char confirmedPassword[MAX_BUFFER_SIZE];
  char user_name[MAX_BUFFER_SIZE];
  int uid_length;
  int user_name_length;
  char salt[SALT_LENGTH] = {}; 

  // uidをstringとして抽出
  myitoa(uid, save_uid, 10);
  uid_length = strlen(save_uid);

  // user名を設定
  write(1, "Enter New Username : ", 22);
  gets(user_name, MAX_BUFFER_SIZE);
  user_name_length = strlen(user_name);
  user_name[user_name_length - 1] = 0; //改行を消す

  // prompt to enter password
  write(1, "Enter New Password : ", 22);
  gets(newPassword, MAX_BUFFER_SIZE);
  write(1, "Enter Again : ", 15);
  gets(confirmedPassword, MAX_BUFFER_SIZE);

  // check that the two passwords match
  if (strcmp(newPassword, confirmedPassword) == 0) { // passwords match, proceed
    write(1, "Password Is Set Successfully \n", 31);

    //genSaltの最後の引数にseedを加えることによって、userごとに違うソルトを持てるように変更を加えた。
    genSalt(salt, SALT_LENGTH, strlen(confirmedPassword));
    //文字列にソルトを追加
    addSalt(confirmedPassword, salt);

    char password_hash[HASH_LENGTH];
    getmd5(confirmedPassword, strlen(confirmedPassword), password_hash);

    //ファイルにuid:user:salt:passwordを格納
    write(fd, save_uid, uid_length);
    write(fd, ":", 1);
    write(fd, user_name , user_name_length);
    write(fd, ":", 1);
    write(fd, salt, SALT_LENGTH);
    write(fd, ":", 1);
    write(fd, password_hash, HASH_LENGTH);
    write(fd, "\n", 1);
    for (int i = 0; i < MAX_BUFFER_SIZE - SALT_LENGTH - HASH_LENGTH - uid_length - user_name_length - 4; i++) {
      write(fd, "", 1);
    }
    close(fd);
    
    return;
  }
  else { // passwords do not match
    printf("Passwords Do Not match. Try Again.\n");
    setPassword(fd, uid);
  }
}
