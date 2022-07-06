#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/md5.h"
#include "user/set_password.h"

int main(void) {

    char read_buf[MAX_BUFFER_SIZE] = {};
    char newPassword[MAX_BUFFER_SIZE];
    char confirmedPassword[MAX_BUFFER_SIZE];
    char salt[SALT_LENGTH] = {};

    int row_count = login();
    printf("Change Password\n");
    int fd = open("Passwords", O_RDWR);

    // 既存のuser:passwordに新しいuser_passwordを上書きする。
    // userがあるrowまでfdを合わせる
    for (int i = 0; i < row_count; i++) {
        read(fd, read_buf, MAX_BUFFER_SIZE);
    }

    // 新しいパスワードの入力
    write(1, "Enter New Password : ", 22);
    gets(newPassword, MAX_BUFFER_SIZE);
    write(1, "Enter Again : ", 15);
    gets(confirmedPassword, MAX_BUFFER_SIZE);

    // check that the two passwords match
    if (strncmp(newPassword, confirmedPassword, HASH_LENGTH) == 0) { // passwords match, proceed
        write(1, "Your Password is Updated\n", 26);

        //新しいソルトを生成して、パスワードに付与する。
        genSalt(salt, SALT_LENGTH, strlen(confirmedPassword));
        addSalt(confirmedPassword, salt);

        char password_hash[HASH_LENGTH] = {};
        getmd5(confirmedPassword, strlen(confirmedPassword), password_hash);

        //ファイルにuser:passwordを格納(1行、max_buffer_size)
        int user_name_length = 0;
        while(1) {
            read(fd, read_buf, 1);
            if (read_buf[0] == ':') break;  // user_nameを飛び越える
            user_name_length++;
        }
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
    else {
        printf("Password Is Not Same\n");
        close(fd);
        exit(1);
    }
    printf("user is not found\n");
    close(fd);
    exit(1);
}