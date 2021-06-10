#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define LOG_SIZE 10000 //최대 기록 가능 log
#define BUF_SIZE 500 //최대 입력가능한 문자열 길이
#define NAME_SIZE 20 //최대 닉네임 길이
#define MAX_CLIENT_NUM 10 //최대 참가자 수

void error_handling(char *msg); //경고 메시지 발송 및 종료
void *menu_thread__handling(void *arg) //명령어 처리
void *handle_client(void *arg); //User Handler
void send_msg(char *msg, int len);

char *START_STRING = "Connected to chat_server \n"; // 클라이언트 환영 메시지

int client_cnt=0; // 참가자 수
int clinet_sock[MAX_CLIENT_NUM]; //참가자 소켓 번호 목록
char client_ip_list[MAX_CLIENT_NUM][NAME_SIZE]; //접속 IP 목록
char client_name_list[MAX_CLIENT_NUM][NAME_SIZE]; //참가자 닉네임

int chat_num=0; //대화 수
char Chatting[LOG_SIZE][BUF_SIZE]; //대화 로그 기록 버퍼

pthrad_mutex_t mutx;

time_t ct;
struct tm tm;

int main(int argc,char *argv[])
{
    int s_sock, c_sock;
    int i;
    struct sockaddr_in s_adr, c_adr;
    int c_adr_size;
    char client_name[NAME_SIZE];

    fd_set read_fd;

    pthread_t menu_handling, client_handling;

    if(argc !=2 )
    {
        printf("Usage : %s <port>\n", argv[0]);
		exit(1);
    }

    pthread_mutex_init(&mutex, NULL);
    s_sock = socket(AF_INET,SOCK_STREAM,0); //TCP, IPv4
    
    if(s_sock==-1)  error_handling("Socket Fail");
    
    //s_adr 구조체의 내용 세팅
    memset(&s_adr, 0, sizeof(s_adr));
    s_adr.sin_family = AF_INET;
    s_adr.sin_addr.s_sddr = htonl(INADDR_ANY);
    s_adr.sin_port = htons(atoi(argv[1]));
    
    if(bind(s_sock,(struct sockaddr*)&s_adr, sizeof(S_adr))==-1)    error_handling("Bind Error");
    //클라이언트로 부터 연결 요청을 기다림
    if(listen(s_sock, 5)==-1) error_handling("Listen Error");

    pthread_create(&menu_handling,NULL,menu_thread__handling,(void *)NULL);

    while(1)
    {
        FD_ZERO(&read_fd);
        FD_SET(s_sock,&read_fd);

        for(i=0;i<client_cnt;i++)
            FD_SET(clinet_sock[i], &read_fd);
        
        if(FD_ISSET(s_sock,&read_fd)){
            
            c_adr_size = sizeof(c_adr);
            c_sock = accept(s_sock, (struct sockaddr*)&c_adr,&c_adr_size);
            
            if(c_sock ==-1) error_handling("Accept Fail");

            if(client_cnt>=MAX_CLIENT_NUM)
            {
                printf("CONNECT FAIL : %d\n",c_sock);
                write(c_sock,"Too Many Users. SORRY",BUF_SIZE);
                continue;
            }

            pthread_mutex_lock(&mutx);

            clinet_sock[client_cnt]=c_sock;
            read(c_sock,client_name,NAME_SIZE);
            strcpy(client_name_list[client_cnt++],client_name);

            pthread_mutex_unlock(&mutx);

            pthread_create(&client_handling, NULL, handle_client, (void *)c_sock);
            pthread_detach(menu_handling);

            send(c_sock,START_STRING,strlen(START_STRING),0);
            ct = time(NULL);
            tm = *localtime(&ct);
            printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
            printf("Connected client IP : %s\n",inet_ntoa(c_adr.sin_addr));

            printf("SERVER > ");

        }

    }

    close(s_sock);
    return 0;

    
}

void *menue_thread_handling(void *arg) // 명령어 처리
{
    int i,j;
    fd_set read_fd; //읽기 감지
    
    printf("\n");
	printf("[MENU]\n\n");
	printf("1. /menu -> some orders \n");
    printf("2. /log -> save chatting log file \n");
    printf("3. /num_user -> Number of users in the current chat\n");
    printf("4. /num_chat -> Number of chat in the current chat\n");
    printf("5. /notice -> Full notice from server\n");
	printf("6. /exit -> chatting program exit \n");
    
    while(1)
    {
        char bufmsg[BUF_SIZE];
        printf("SERVER > ");
        fgets(bufmsg,BUF_SIZE,stdin); //명령어 입력
        if(!strcmp(bufmsg,'\n')) continue;
        else if(!strcmp(bufmsg,"/menu"))  //menu
        {
            printf("[MENU]\n\n");
            printf("1. /menu -> some orders \n");
            printf("2. /log -> save chatting log file \n");
            printf("3. /num_user -> Number of users in the current chat\n");
            printf("4. /num_chat -> Number of chat in the current chat\n");
            printf("5. /notice -> Full notice from server\n");
	        printf("6. /exit -> chatting program exit \n");
        }
        else if(!strcmp(bufmsg,"/log")) // log
        {

        }
        else if(!strcmp(bufmsg,"/num_user")) //num_user
        {
            printf("Number of user in current chat : %d",client_cnt);
        }
        else if(!strcmp(bufmsg,"/num_chat")) //num_chat
        {
            printf("Number of chat in current chat : %d",chat_num);
        }
        else if(!strcmp(bufmsg,"/notice")) //notice
        {

        }
        else if(!strcmp(bufmsg,"/exit")) //exit
        {

        }
    }
}

void error_handling(char *msg) //error handling
{
    fputs(msg, stderr);
    fputs('\n',stderr);
    exit(1);
}
