#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
void *menu_thread_handling(void *arg); //명령어 처리
void *handle_client(void *arg); //User Handler
void send_msg(char *msg, int len);

char *START_STRING = "Connected to chat_server \n"; // 클라이언트 환영 메시지

int client_cnt=0; // 참가자 수
int clinet_sock[MAX_CLIENT_NUM]; //참가자 소켓 번호 목록
char client_name_list[MAX_CLIENT_NUM][NAME_SIZE]; //참가자 닉네임

int chat_num=0; //대화 수
char Chatting[LOG_SIZE][BUF_SIZE]; //대화 로그 기록 버퍼

pthread_mutex_t mutx;

time_t ct;
struct tm tm;
int s_sock;

int main(int argc,char *argv[])
{
    int c_sock;
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

    pthread_mutex_init(&mutx, NULL);
    s_sock = socket(AF_INET,SOCK_STREAM,0); //TCP, IPv4
    
    if(s_sock==-1)  error_handling("Socket Fail");
    
    //s_adr 구조체의 내용 세팅
    memset(&s_adr, 0, sizeof(s_adr));
    s_adr.sin_family = AF_INET;
    s_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_adr.sin_port = htons(atoi(argv[1]));
    
    if(bind(s_sock,(struct sockaddr*)&s_adr, sizeof(s_adr))==-1)    error_handling("Bind Error");
    //클라이언트로 부터 연결 요청을 기다림
    if(listen(s_sock, 5)==-1) error_handling("Listen Error");

    pthread_create(&menu_handling,NULL,menu_thread_handling,(void *)NULL);

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

void *handle_client(void *arg)
{
    int c_sock = *((int *)arg);
    int len=0,i;
    int filesize=0;
    const char sig_file[BUF_SIZE] = {"File : cl -> sr"};
	const char Fmsg_end[BUF_SIZE] = {"FileEND : cl -> sr"};
	const char sig_file_all[BUF_SIZE] = {"File : cl -> sr all"};
	const char sig_whisper[BUF_SIZE] = {"Whisper : cl -> sr"};
	char msg[BUF_SIZE] = {NULL};
	char file_msg[BUF_SIZE] = {NULL};

    while((len = read(c_sock, msg, BUF_SIZE))!=0)
    {
        if(!strcmp(msg,sig_file))
        {
            int j;
            int noClient = 1;
            int fileGo = NULL;
            char tmpName[NAME_SIZE];

            read(c_sock, tmpName, NAME_SIZE);

            pthread_mutex_lock(&mutx);

            for(j=0;j<client_cnt;j++)
            {
                if(!strcmp(tmpName, client_name_list[j]))
                {
                    noClient=0;
                    fileGo=j;
                    break;
                }
            }

            if(noClient==1)
            {
                write(c_sock,"[NO Client. SORRY]",BUF_SIZE);
                pthread_mutex_unlock(&mutx);
                continue;
            }
            else if(noClient==0)
            {
                write(c_sock,"[Cotinue Ok NowGo]",BUF_SIZE);
            }

            write(clinet_sock[fileGo],"File : sr -> cl",BUF_SIZE);

            read(c_sock,&filesize,sizeof(int));
            printf("File size %d Bytes\n",filesize);
            write(clinet_sock[fileGo],&filesize,sizeof(int));

            while(1)
            {
                read(c_sock, file_msg,BUF_SIZE);
                if(!strcmp(file_msg,Fmsg_end))  break;
                write(clinet_sock[fileGo], file_msg , BUF_SIZE);
            }

            write(clinet_sock[fileGo],"FileEnd : sr -> cl",BUF_SIZE);

            pthread_mutex_unlock(&mutx);
            ct = time(NULL);
            tm = *localtime(&ct);
            printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
            printf("(!NOTICE)File Data transfered\n");


        }
        else if(!strcmp(msg, sig_file_all))
        {
            pthread_mutex_lock(&mutx);

            for(i=0;i<client_cnt;i++)
            {
                if(c_sock == clinet_sock[i])    continue;
                write(clinet_sock[i],"File : sr -> cl",BUF_SIZE);
            }

            read(c_sock,&filesize,sizeof(int));
            printf("File size %d Bytes\n",filesize);

            for(i=0;i<client_cnt;i++)
            {
                if(c_sock == clinet_sock[i])    continue;
                write(clinet_sock[i],&filesize,sizeof(int));
            }

            while(1)
            {
                read(c_sock,file_msg,BUF_SIZE);
                if(!strcmp(file_msg,Fmsg_end))  break;
                for(i=0;i<client_cnt;i++)
                {
                    if(c_sock == clinet_sock[i]) continue;
                    write(clinet_sock[i],file_msg,BUF_SIZE);
                }
            }

            for(i=0;i<client_cnt;i++)
            {
                if(c_sock == clinet_sock[i])    continue;
                wirte(clinet_sock[i],"FileEnd : sr -> cl",BUF_SIZE);
            }

            pthread_mutex_unlock(&mutx);
            ct = time(NULL);
            tm = *localtime(&ct);
            printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
            printf("(!NOTICE)File Data Transfered\n");
        }
        else if(!strcmp(msg, sig_whisper))
        {
            int j;
            int noClient = 1;
            int mGo=0;
            char tmpName[NAME_SIZE];
            char msg[BUF_SIZE];

            read(c_sock, tmpName, NAME_SIZE);

            pthread_mutex_lock(&mutx);
            for(j=0;j<client_cnt;j++)
            {
                if(!strcmp(tmpName,client_name_list[j]))
                {
                    noClient=0;
                    mGo=j;
                    break;
                }
            }
            pthread_mutex_unlock(&mutx);

            read(c_sock, msg, BUF_SIZE);

            if(noClient==1) wirte(c_sock,"Sorry, No Client like that",BUF_SIZE);
            else    write(clinet_sock[mGo],msg,BUF_SIZE);

        }
        else 
        {
               printf("%s",msg);
        }
    }

    pthread_mutex_lock(&mutx);
    for(i=0;i<client_cnt;i++) //remove disconnected client
    {
        if(c_sock == clinet_sock[i])
        {
            while(i++<client_cnt-1)
            {
                clinet_sock[i]=clinet_sock[i+1];
                strcpy(client_name_list[i],client_name_list[i+1]);
            }
            break;
        }
    }
    client_cnt--;
    pthread_mutex_unlock(&mutx);
    close(c_sock);
    return NULL;
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
            for (i = 0; i < client_cnt; i++) {
				if (FD_ISSET(clinet_sock[i], &read_fd)) {
					chat_num++;				//총 대화 수 증가
					// 모든 채팅 참가자에게 메시지 방송
					for (j = 0; j < client_cnt; j++)
						send(clinet_sock[j], bufmsg, strlen(bufmsg), 0);
					ct = time(NULL);
					tm = *localtime(&ct);
					printf("[%02d:%02d:%02d]", tm.tm_hour, tm.tm_min, tm.tm_sec);
					printf("%s", bufmsg);			//메시지 출력
				}
			}
        }
        else if(!strcmp(bufmsg,"/exit")) //exit
        {
            if(client_cnt!=0){
				puts("진행 중인 채팅이 있습니다.");
			}
			else
			{
				puts("Good bye!");
				close(s_sock);
				exit(0);
			}
        }
        else
            printf("No such command. See menu\n");
    }
}

void error_handling(char *msg) //error handling
{
    fputs(msg, stderr);
    fputs('\n',stderr);
    exit(1);
}
