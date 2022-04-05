#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <map>
#include <vector>

using namespace std;

#define MAXLINE 1200

// this is server
int main(int argc, char** argv){ // receiver.. [(1)save filename] [(2)bind port]
	int sockfd;
	struct sockaddr_in servaddr,claddr;

	if(argc != 3){
		cout << "not enough args" << endl;
		return 1;
	}

	int nRecvBuf = 32*1024;

	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	//setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&nRecvBuf,sizeof(int));
	bzero(&servaddr,sizeof(servaddr));
	claddr.sin_family = AF_INET;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[2]));

	bind(sockfd,(sockaddr*)&servaddr,sizeof(servaddr));
	
	// dg_echo(int sockfd,sockaddr* pcliaddr,socklen_t clilen)
	//         sockfd    ,(SA*)&claddr      ,sizeof(claddr)
	/*
	int n;
	socklen_t len;
	char msg[1200];

	for(;;){
		len = sizeof(claddr);
		n = recvfrom(sockfd,msg,MAXLINE,0,(sockaddr*)&claddr,&len);
		sendto(sockfd,msg,n,0,(sockaddr*)&claddr,len);
	}
	*/
	//
	int n;
	socklen_t len = sizeof(claddr);
	char sdbuf[1024],rvbuf[1025];
	ofstream rfile(argv[1],ios::out | ios::binary);
	if(!rfile){
		cout << "cannot open" << endl;
		return 1;
	}
	memset(&rvbuf,0,sizeof(rvbuf));
	if((n = recvfrom(sockfd,rvbuf,1024,0,(sockaddr*)&claddr,&len)) < 0){ // receive file size packet
	//if((n = recvfrom(sockfd,rvbuf,1024,0,NULL,NULL)) < 0){ // receive file size packet
		cout << "receive error" << endl;
		return 1;
	}
	int fsize = atoi(rvbuf+4); // get file size
	cout << "get size " << fsize << endl;
	cout << "rvbuf: " << strlen(rvbuf) << endl;
	int sendret = sendto(sockfd,rvbuf,strlen(rvbuf),0,(sockaddr*)&claddr,len); // send ack
	cout << "sendto return value: " << sendret << endl;
	int numpackets = fsize / 1020 + (fsize % 1020 != 0);
	map<int,char*> tempbuf;
	vector<bool> isrecved(numpackets,false);
	int recvseq = -1;
	while(true){
		if((n = recvfrom(sockfd,rvbuf,1024,0,NULL,NULL)) < 0){ // receive data packet
			cout << "receive error" << endl;
			return 1;
		}
		string testsize(rvbuf,0,4);
		if(testsize == "size"){ // sender didn't receive size ack
			cout << "duplicate size packet" << endl;
			sendret = sendto(sockfd,rvbuf,strlen(rvbuf),0,(sockaddr*)&claddr,len); // send size ack
			//cout << "sendto return value: " << sendret << endl;
			//cout << "error: " << errno << endl;
			continue;
		}
		//cout << rvbuf << endl;
		int seqnum = *((int*)rvbuf);
		//cout << "get packet " << seqnum << endl;
		if(seqnum == recvseq + 1){
			if(seqnum == numpackets-1 && fsize % 1020 != 0)
				rfile.write(rvbuf+4,fsize%1020);
			else
				rfile.write(rvbuf+4,1020);
			memset(&sdbuf,0,sizeof(sdbuf));
			strcpy(sdbuf,to_string(seqnum).c_str());
			sendto(sockfd,sdbuf,strlen(sdbuf),0,(sockaddr*)&claddr,len);
			isrecved[seqnum] = true;
			recvseq++;
			for(int i = seqnum+1;i < numpackets;i++){
				if(!isrecved[i]) break;
				if(i == numpackets-1 && fsize % 1020 != 0)
					rfile.write(tempbuf[i]+4,fsize%1020);
				else
					rfile.write(tempbuf[i]+4,1020);
				recvseq++;
			}
		}
		else if(isrecved[seqnum]){
				memset(&sdbuf,0,sizeof(sdbuf));
				strcpy(sdbuf,to_string(seqnum).c_str());
				sendto(sockfd,sdbuf,strlen(sdbuf),0,(sockaddr*)&claddr,len);
		}
		else{
			//rfile.write(rvbuf+4,1020);
			tempbuf[seqnum] = new char[1024];
			memcpy(tempbuf[seqnum],rvbuf,1024);
			memset(&sdbuf,0,sizeof(sdbuf));
			strcpy(sdbuf,to_string(seqnum).c_str());
			sendto(sockfd,sdbuf,strlen(sdbuf),0,(sockaddr*)&claddr,len);
			isrecved[seqnum] = true;
			//recvseq = seqnum;
		}
		bool finished = true;
		for(int i = numpackets-1;i >= 0;i--)
			if(!isrecved[i]){
				finished = false;
				break;
			}
		if(finished) break;
		//cout << seqnum << " " << recvseq << endl;
	}
	for(auto &it : tempbuf)
		delete [] it.second;
	rfile.close();

	return 0;
}
