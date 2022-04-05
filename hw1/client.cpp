#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

using namespace std;

int main(int argc, char **argv){
	
	if(argc != 3){
		cout << "argv[1]: ip address, argv[2]: port number" << endl;
		return 0;
	}

	struct sockaddr_in svaddr;
	int sockfd;
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	bzero(&svaddr,sizeof(svaddr));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET,argv[1],&svaddr.sin_addr); // convert ip addr

	int conn = connect(sockfd,(sockaddr *)&svaddr,sizeof(svaddr));
	if(conn < 0){
		cout << "connect error" << endl;
		return 1;
	}

	int maxfdp1;
	fd_set rset;
	char sdbuf[1000],rvbuf[1000];

	FD_ZERO(&rset);
	while(true){
		FD_SET(fileno(stdin),&rset);
		FD_SET(sockfd,&rset);
		maxfdp1 = max(fileno(stdin),sockfd) + 1;
		select(maxfdp1,&rset,NULL,NULL,NULL);
		if(FD_ISSET(sockfd,&rset)){ // read from server
			string line;
			memset(&rvbuf,0,sizeof(rvbuf));
			int temp = recv(sockfd,(char *)&rvbuf,sizeof(rvbuf),0);
			if(temp == 0) continue;
			cout << rvbuf;
			//fputs(rvbuf,stdout); // or use this
		}
		if(FD_ISSET(fileno(stdin),&rset)){ // write to server
			fgets(sdbuf,1000,stdin);
			string sd(sdbuf);
			if(sd.substr(0,4) == "exit" && sd.length() == 5) {
				close(sockfd);
				break;
			}
			send(sockfd,(char *)&sdbuf,strlen(sdbuf),0);
		}
	}

	return 0;
}
