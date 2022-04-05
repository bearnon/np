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
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <map>
#include <vector>

#define MAXLINE 1025

using namespace std;

int main(int argc,char **argv){ // client
	if(argc != 4){
		cout << "argc must be 4" << endl;
		return 1;
	}
	
	struct sockaddr_in svaddr;
	int sockfd;
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	bzero(&svaddr,sizeof(svaddr));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET,argv[1],&svaddr.sin_addr);

	int conn = connect(sockfd,(sockaddr *)&svaddr,sizeof(svaddr));
	if(conn < 0){
		cout << "connect error" << endl;
		return 1;
	}
	char sdbuf[1024];
	memset(&sdbuf,0,sizeof(sdbuf));
	strcpy(sdbuf,argv[3]);

	cout << "Welcome to the dropbox-like server: " << argv[3] << endl;

	int maxfdp1,val,stdineof;
	ssize_t n,nwritten;
	fd_set rset,wset;
	char to[MAXLINE],fr[MAXLINE];
	char *toiptr,*tooptr,*friptr,*froptr;

	val = fcntl(sockfd,F_GETFL,0);
	fcntl(sockfd,F_SETFL,val|O_NONBLOCK);

	val = fcntl(STDIN_FILENO,F_GETFL,0);
	fcntl(STDIN_FILENO,F_SETFL,val|O_NONBLOCK);

	val = fcntl(STDOUT_FILENO,F_GETFL,0);
	fcntl(STDOUT_FILENO,F_SETFL,val|O_NONBLOCK);

	toiptr = tooptr = to; // initialize buffer pointers
	friptr = froptr = fr;
	stdineof = 0;
	char cmdbuf[1000];
	bool uploading = false;
	bool downloading = false;
	string dlfilename;
	string ulfilename;
	int dlfilesize;
	int ulfilesize;
	int bytesend = 0;
	int byterecv = 0;
	bool namesent = false;

	maxfdp1 = max(max(STDIN_FILENO,STDOUT_FILENO),sockfd) + 1;
	while(true){
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		//if(stdineof == 0 && toiptr < &to[MAXLINE])
			FD_SET(fileno(stdin),&rset); // read from stdin
		if(friptr < &fr[MAXLINE])
			FD_SET(sockfd,&rset); // read from socket
		if(tooptr != toiptr || !namesent)
			FD_SET(sockfd,&wset); // data to write to socket
		//if(froptr != friptr)
		//	FD_SET(STDOUT_FILENO,&wset); // data to write to stdout

		select(maxfdp1,&rset,&wset,NULL,NULL);

		if(FD_ISSET(fileno(stdin),&rset)){ // read from stdin
			memset(&cmdbuf,0,sizeof(cmdbuf));
			fgets(cmdbuf,1000,stdin);
			string command(cmdbuf);
			
			if(command.substr(0,5) == "/put "){
				toiptr[0] = '0';
				ulfilename = command.substr(5,command.length()-6);
				strcpy(toiptr+1,ulfilename.c_str());
				ifstream upfile(ulfilename,ios::binary);
				upfile.seekg(0,ios::end);
				ulfilesize = upfile.tellg();
				upfile.close();
				memcpy(toiptr+33,&ulfilesize,4);
				toiptr += 1024;
				bytesend = 0;
				uploading = 1;
				cout << "[Upload] " << ulfilename << " Start!" << endl;
				cout << "Progress : [                      ]";
			}
			else if(command.substr(0,7) == "/sleep "){
				cout << "The client starts to sleep." << endl;
				int sleepsec = atoi(command.substr(7).c_str());
				for(int i = 1;i <= sleepsec;i++){
					sleep(1);
					cout << "Sleep " << i << endl;
				}
				cout << "Client wakes up." << endl;
			}
			else if(command.substr(0,5) == "/exit" && command.length() == 6){
				close(sockfd);
				break;
			}
		}

		bool recved = false;
		if(FD_ISSET(sockfd,&rset)){ // read from socket
			int n = recv(sockfd,(char *)friptr,1024,0);
			if(n == 0){
				// close
			}
			else if(n > 0){
				recved = true;
				friptr += 1024;
			}
		}
		if(recved){
			if(froptr[0] == '0' && !downloading){ // download file name size
				char dlname[32];
				int dlsize;
				strcpy(dlname,froptr+1);
				dlfilename = string(dlname);
				memcpy(&dlsize,froptr+33,4);
				dlfilesize = dlsize;
				ofstream downfile(dlfilename,ios::binary);
				byterecv = 0;
				downfile.close();
				downloading = true;
				cout << "[Download] " << dlfilename << " Start!" << endl;
				cout << "Progress : [                      ]";
			}
			else if(froptr[0] == '1'){ // download file content
				ofstream downfile(dlfilename,ios::binary|ios::app);
				if(dlfilesize - byterecv < 1023)
					downfile.write(froptr+1,dlfilesize % 1023);
				else
					downfile.write(froptr+1,1023);
				downfile.close();
				byterecv += 1023;

				cout << "\33[2K\r";
				cout << "Progress : [";
				for(int i = 0;i < 22;i++)
					cout << ((byterecv >= dlfilesize*i/22) ? ('#') : (' '));
				cout << "]";
				if(byterecv >= dlfilesize){
					cout << "\n[Download] " << dlfilename << " Finish!" << endl;
					downloading = false;
				}
			}
			else if(froptr[0] == '2'){
				namesent = true;
			}
			froptr += 1024;
			if(froptr == friptr) froptr = friptr = fr;
		}

		if(FD_ISSET(sockfd,&wset) && ((n = toiptr - tooptr) > 0 || !namesent)){ // data to write to socket
			int nw;
			if(!namesent)
				send(sockfd,sdbuf,sizeof(sdbuf),0); // send the user name to server
			else
				nw = send(sockfd,(char *)tooptr,1024,0);
			if(nw > 0){
				tooptr += 1024;
				if(tooptr == toiptr) tooptr = toiptr = to;
			}
		}

		if(toiptr == tooptr && uploading){
			toiptr[0] = '1';
			ifstream upfile(ulfilename,ios::binary);
			upfile.seekg(bytesend,ios::beg);
			upfile.read(toiptr+1,1023);
			upfile.close();
			toiptr += 1024;
			bytesend += 1023;

			cout << "\33[2K\r";
			cout << "\rProgress : [";
			for(int i = 0;i < 22;i++)
				cout << ((bytesend >= ulfilesize*i/22) ? ('#') : (' '));
			cout << "]";
			if(bytesend >= ulfilesize){
				cout << "\n[Upload] " << ulfilename << " Finish!" << endl;
				uploading = false;
			}
		}
	}

	return 0;
}
