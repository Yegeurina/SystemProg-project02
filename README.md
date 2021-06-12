# SystemProg-project02
- 소켓 네트워크 프로그램을 이용하여 클라이언트 - 서버 모델 기반의 채팅 프로그램을 제작
- 단체 채팅방과 같이 여러 클라이언트가 동시 접속을 통해 채팅을 하는 프로그램임



## server.c

[실행 방법]
```c
gcc -o server server.c -lpthread
./server 9999
```

-서버의 용도 : 전체 채팅을 관리해주는 역할

-서버에서 지원하는 명령어
> !menu
> > change name   
> > clear/update  
> > dutchpay  
> > filetransfer  
> > log file download  



## client.c

[실행방법]
```c
gcc -o client client.c -lpthread
./client 127.0.0.1 9999 bob
```   
-클라이언트 : 2개 이상 실행이 가능

