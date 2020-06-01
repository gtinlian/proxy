#include <stdio.h>  //std library
#include <string.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include <netinet/in.h> /*struct sockaddr_in*/
#include <netinet/ip.h> 

#include <unistd.h>  /*read write*/

#include <arpa/inet.h>  /**/

#include <pthread.h> /**/


#define PROXY_DES_PORT 22
#define PROXY_DES_HOST "127.0.0.1"
#define LISENT_PORT 222
#define LISENT_HOST "0.0.0.0"


//多线程处理发送和接收
void readandwrite(void *fds){
	int *fd = (int*)fds;
	char buff[1024];
	int len=0;
	while(1){
		printf("%d -> %d\n",fd[0],fd[1]);
		len = read(fd[0],buff,1024);
		printf("len = %d",len);
		write(fd[1],buff,len);
	}
	return;
}

int main(int argc,char* argv[]){

	int fd_t1[2]; //socket fd cli and ser read list, call readandwrite function
	int fd_t2[2]; //socket fd cli and ser read list, call readandwrite function


	struct sockaddr_in cliaddr,seraddr,ser_cliaddr,cli_seraddr;   //socket struct save connect information, call memset set zero
	memset(&cliaddr,0,sizeof(cliaddr));
	memset(&seraddr,0,sizeof(seraddr));   
	memset(&ser_cliaddr,0,sizeof(ser_cliaddr));

	cliaddr.sin_addr.s_addr = inet_addr(PROXY_DES_HOST);
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(PROXY_DES_PORT);

	seraddr.sin_addr.s_addr = htonl(inet_addr(LISENT_HOST));
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(LISENT_PORT);

	pthread_t t1,t2;


	socklen_t ser_len = sizeof(ser_cliaddr);   //accpet 
	socklen_t cli_len = sizeof(cliaddr);   //connect


	int cli = socket(AF_INET,SOCK_STREAM,0);
	int ser = socket(AF_INET,SOCK_STREAM,0);
	if(!(cli && ser)){
		perror("create socket error");
		return -1;
	}

	if(bind(ser,(struct sockaddr*)&seraddr,sizeof(seraddr)))
	{perror("ser bind error"); return -1;}

	/*if(bind(cli,(struct sockaddr*)&cliaddr,sizeof(cliaddr)))
	{perror("cli bind error");return -1;}
	*/
	if(listen(ser,65536))
	{perror("lisent error"); return -1;}

	while(1){
		int ser_clifd = accept(ser,(struct sockaddr*)&ser_cliaddr,&ser_len);
		if(ser_clifd <= 0){
			perror("accept error");
			return -1;
		}
		else{
		fd_t1[0] = ser_clifd;
		fd_t2[1] = ser_clifd;

		int stat = connect(cli,(struct sockaddr*)&cliaddr,cli_len);
		if(stat == -1){
			perror("connect error");
			return 0;
		}else{
		fd_t1[1] = cli;
		fd_t2[0] = cli;
		
		pthread_create(&t1,0,readandwrite,(void*)fd_t1);
		pthread_create(&t2,0,readandwrite,(void*)fd_t2);
		
		
		pthread_join(&t1,NULL);
		pthread_join(&t2,NULL);
		}
		
		}
	}

}
