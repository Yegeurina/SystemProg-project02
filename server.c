#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/types.h> 
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 500 //최대 입력가능한 문자열 길이
#define NAME_SIZE 20 //최대 닉네임 길이
#define MAX_CLIENT_NUM 10 //최대 참가자 수

void error_handling(char *msg); //경고 메시지 발송 및 종료

int main()
{

}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputs('\n',stderr);
    exit(1);
}
