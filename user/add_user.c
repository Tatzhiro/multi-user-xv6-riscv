#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "user/set_password.h"

int main(void){
    
    int uid = 0;
    char buf[MAX_BUFFER_SIZE] = {};

    printf("Add New User\n");

    int fd = open("Passwords", O_RDWR);
    if(fd == -1) {
        write(1, "Cannot open Passwords\n", 23);
        exit(1);
    }

    while(read(fd, buf, sizeof(buf))){
        uid++;
    }; 
    
    setPassword(fd, uid);

    exit(0); 

}