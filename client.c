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

#define BUF_SIZE 500 //최대 입력가능한 문자열 길이
#define NAME_SIZE 20 //최대 닉네임 길이
#define NOTSET 0
#define EXIST 1
#define NOTEXIST 2

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);

char name[NAME_SIZE];
char msg[BUF_SIZE];
int client_exist = NOTSET;
int setFName = 0;
int wOk = 1;

time_t ct;
struct tm tm;

int main(int argc, int *argv[])
{
    int sock;
    struct sockaddr_in s_adr;
    pthread_t send_trhread, receive_thread;
    void * thread_return;

    if(argc != 4)
    {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
    }

    sprintf(name, "%s", argv[3]);
	sock=socket(AF_INET, SOCK_STREAM, 0);
	
    if(sock == -1)  error_handling("Socket Fail");

	memset(&s_adr, 0, sizeof(s_adr));
	s_addr.sin_family=AF_INET;
	s_addr.sin_addr.s_addr=inet_addr(argv[1]);
	s_addr.sin_port=htons(atoi(argv[2]));

    if(connect(sock,(struct sockaddr*)&s_adr,sizeof(s_adr))==-1)    error_handling("Connect Error");

    printf("CONNECTING..... \n [TIP] If you want \"MENU\" -> /menu \n\n");

    pthread_create(&send_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&receive_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(send_thread, &thread_return);
	pthread_join(receive_thread, &thread_return);

	close(sock);  
	return 0;

}

void *send_msg(void *arg)
{
    int sock = *((int *)arg);
    int Flength = 0;
    int i=0;
    int filesize =0;
    int fEnd = 0;
    char name_msg[NAME_SIZE+BUF_SIZE];
    char t_msg[BUF_SIZE];
    char last_msg[BUF_SIZE];
    char t_name_msg[BUF_SIZE];
    char noUse[BUF_SIZE];
	const char enter[BUF_SIZE] = {"\n"};
	const char whisper[BUF_SIZE] = {"/whisper\n"};

    while(1)
    {
        if(wOk == 0)    sleep(1);

        fgets(msg, BUF_SIZE , stdin);

        if(!strcmp(msg,"/exit\n"))
        {
            puts("Good bye.");
			close(sock);
			exit(0);
        }
        else if(!(strcmp(msg,"/sendfile\n")))
        {
            char location[BUF_SIZE];
            char who[NAME_SIZE];
            FILE *fp;
            FILE *size;

            printf("(!RECORD)File location : ");
            scanf("%s",location);

            size = fopen(location,"rb");
            if(size == NULL)
            {
                printf("(!NOTICE)NO file like that\n");
                continue;
            }

            printf("(!RECORD)To Who(ID)? : ");
            scanf("%s",who);

            write(sock, "File : cl -> sr",BUF_SIZE);
            write(sock,who, NAME_SIZE);
            
            while(client_exist == NOTSET)   sleep(1);
            if(client_exist==NOTEXIST)
            {
                printf("(!NOTICE)NO User like that");
                client_exist=NOTSET;
                continue;
            }

            while(1)
            {
                fEnd=fread(noUse,1,BUF_SIZE,size);
                filesize+=fEnd;
                if(fEnd!=BUF_SIZE)  break;
            }
            fclose(size);

            printf("(!NOTICE)File Transfer start\n(File size : %d Bytes)\n",filesize);
            wirte(sock, &filesize,sizeof(int));
            filesize=0;

            fp=fopen(location,"rb");

            while(1)
            {
                Flength = fread(t_msg,1,BUF_SIZE,fp);
                if(Flength != BUF_SIZE){
                    for(i=0;i<Flength;i++)  last_msg[i]=t_msg[i];
                    writte(sock, last_msg, BUF_SIZE);
                    wite(sock,"FileEND : cl -> sr",BUF_SIZE);
                    break;
                }

                 write(sock, t_msg, BUF_SIZE);
                
            }
            fclose(fp);
            printf("(!NOTICE)File transfer finish\n");
            client_exist=NOTSET;

        }
        else if(!strcmp(msg, "/sendfile all\n"))
        {
            char location[BUF_SIZE];
            FILE *fp;
            FILE *size;
        
            printf("(!RECORD)File location : ");
            scanf("%s",location);

            size = fopen(location,"rb");
            if(size == NULL)
            {
                printf("(!NOTICE) NO File like that\n");
                continue;
            }

            write(sock, "File : cl -> sr all",BUF_SIZE);

            while(1)
            {
                fEnd = fread(noUse,1,BUF_SIZE,size);
                filesize+=fEnd;
                if(fEnd != BUF_SIZE)    break;
            }
            fclose(size);

            printf("(!NOTICE)File Transfer start\n(File size : %d Bytes)\n",filesize);
            write(sock, &filesize,sizeof(int));
            filesize=0;

            fp = fopen(location,"rb");

            while(1)
            {
                Flength = fread(t_msg,1,BUF_SIZE,fp);
                if(Flength != BUF_SIZE)
                {
                    for(i=0;i<Flength;i++)  last_msg[i]=t_msg[i];
                    write(sock,lasgt_msg,BUF_SIZE);
                    write(sock,"FileEND : cl -> sr",BUF_SIZE);
                    break;
                }
                write(sock,t_msg,BUF_SIZE);
            }

            fclose(fp);
            printf("(!NOTICE)File transfer finish\n");
        }
        else if(!strcmp(msg,"/menu\n"))
        {
            printf("\n");
            printf("[MENU]\n\n");
            printf("1. /menu -> some orders \n");
            printf("2. /whisper -> whispering to someone\n");
            printf("3. /sendfile -> 1:1 file transfer \n");
            printf("4. /sendfile all -> 1:N file transfer \n");
            printf("5. /exit -> chatting program exit \n");
            printf("\n[END MENU] \n\n");
        }
        else if(setFName ==1)
        {
            if(strcmp(msg,enter))   setFName=0;
        }
        else if(!strcmp(msg,whisper))
        {
            char who[NAME_SIZE];
            char wmsg[BUF_SIZE];

            strcpy(t_msg,NULL);

            printf("(!RECORD) who(ID) Message : ");
            scanf("%s %[^\n]",who,wmsg);

            write(sock, "Whisper : cl -> sr",BUF_SIZE);

            write(sock,who,NAME_SIZE);

            strcpy(t_msg,"\n");
            sprintf(t_name_msg,"[(Whispering)%s] %s",name,t_msg);
            sprintf(name_msg, "[(Whispering)%s] %s",name,wmsg);

            name_msg[strlen(name_msg)]="\n";

            if(strcmp(name_msg,t_name_msg)!=0)  write(sock,name_msg,BUF_SIZE);

        }
        else{
            strcpy(t_msg,"\n");
            sprintf(t_name_msg,"[%s] %s",name,t_msg);
            sprintf(name_msg,"[%s] %s",name,msg);
            if(strcmp(name_msg,t_name_msg)!=0)  write(sock,name_msg,BUF_SIZE);
        } 
    }
    
    return NULL
    
}

void error_handling(char *msg) //error handling
{
    fputs(msg, stderr);
    fputs('\n',stderr);
    exit(1);
}