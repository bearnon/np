#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
//#include <filesystem>
#include <sys/wait.h>
#include <sys/time.h>

using namespace std;

#define MAXLINE 1200

void sig_alrm(int signo){
	//cout << "sig_alrm: timeout!" << endl;
	return;
}

// this is client
int main(int argc,char **argv){ // sender.. [send filename] [target addr] [conn port]
	signal(SIGALRM,sig_alrm);
	siginterrupt(SIGALRM,1);
	int sockfd;
	struct sockaddr_in servaddr;
	char sdbuf[1024],rvbuf[1024];

	if(argc != 4){
		cout << "not enough args" << endl;
		return 1;
	}
	struct timeval start,end;
	gettimeofday(&start,0);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[3]));
	inet_pton(AF_INET,argv[2],&servaddr.sin_addr);
	sockfd = socket(AF_INET,SOCK_DGRAM,0);

	// dg_cli(FILE *fp,int sockfd,const sockaddr* pservaddr,socklen_t servlen)
	//        stdin   ,sockfd    ,(SA*)&servaddr           ,sizeof(servaddr)
	/*
	int n;
	char sendline[MAXLINE],recvline[MAXLINE+1];

	while(fgets(sendline,MAXLINE,stdin) != NULL){
		sendto(sockfd,sendline,strlen(sendline),0,(sockaddr*)&servaddr,sizeof(servaddr));

		n = recvfrom(sockfd,recvline,MAXLINE,0,NULL,NULL);
		
		recvline[n] = 0; // null terminate
		fputs(recvline,stdout);
	}
	*/
	//
	if(connect(sockfd,(sockaddr*)&servaddr,sizeof(servaddr)) < 0){
		cout << "connect error" << endl;
		return 1;
	}
	ifstream sfile(argv[1],ios::in | ios::binary);
	if(!sfile){
		cout << "cannot open" << endl;
		return 1;
	}
	//int fsize = std::filesystem::file_size(argv[1]);
	sfile.seekg(0,ios::end);
	int fsize = sfile.tellg();
	sfile.seekg(0,ios::beg);
	cout << "file size = " << fsize << endl;
	string fsize_str("size");
	fsize_str += to_string(fsize);
	memset(&sdbuf,0,sizeof(sdbuf));
	strcpy(sdbuf,fsize_str.c_str());
	int n;
	int timeoutcount = 0;
	while(true){ // send file size packet
		sendto(sockfd,sdbuf,sizeof(sdbuf),0,(sockaddr*)&servaddr,sizeof(servaddr));
		ualarm(100000,0);
		if((n = recvfrom(sockfd,rvbuf,1024,0,NULL,NULL)) < 0){ // wait for ack
			if(errno == EINTR){
				timeoutcount++;
			}
		}
		else{
			ualarm(0,0);
			rvbuf[n] = '\0';
			cout << "ack: " << rvbuf << endl;
			timeoutcount = 0;
			break;
		}
		if(timeoutcount == 30){
			cout << "timeout too much" << endl;
			return 1;
		}
	}
	int numpackets = fsize / 1020 + (fsize % 1020 != 0);
	
	for(int i = 0;i < numpackets;i++){ // send data
		if(i % 100 == 0)
			cout << "packet " << i << endl;
		memset(&sdbuf,0,sizeof(sdbuf));
		// packet seq number
		sdbuf[0] = i & 0xff;
		sdbuf[1] = (i>>8) & 0xff;
		sdbuf[2] = (i>>16) & 0xff;
		sdbuf[3] = (i>>24) & 0xff;
		//
		sfile.read(sdbuf+4,1020);
		//cout << "num in sdbuf: " << *((int*)sdbuf) << endl;
		//cout << sdbuf << endl;
		while(true){
			sendto(sockfd,sdbuf,sizeof(sdbuf),0,(sockaddr*)&servaddr,sizeof(servaddr));
			ualarm(90000,0);
			if((n = recvfrom(sockfd,rvbuf,1024,0,NULL,NULL)) < 0){ // wait for ack
				if(errno == EINTR){
					//cout << "timeout!" << endl;
					timeoutcount++;
				}
			}
			else{
				ualarm(0,0);
				rvbuf[n] = '\0';
				//cout << "ack: " << rvbuf << endl;
				//int acknum = atoi(rvbuf);
				timeoutcount = 0;
				if(atoi(rvbuf) != i)
					continue;
				break;
			}
			if(timeoutcount == 30){
				cout << "timeout too much" << endl;
				return 1;
			}
		}
		
	}
	gettimeofday(&end,0);
	int sec = end.tv_sec - start.tv_sec;
	double usec = end.tv_usec - start.tv_usec;
	cout << "time: " << sec + (usec/1000000.0) << " s" << endl;
	return 0;
}
