#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <string>
#include <map>
#include <vector>
#include <fstream>

#define MAXLINE 1025

using namespace std;

int main(int argc,char **argv){ // server
	if(argc != 2){
		cout << "argc must be 2" << endl;
		return 0;
	}
	struct sockaddr_in claddr,svaddr;
	int listenfd;
	int flag;
	map<int,string> cliname;

	listenfd = socket(AF_INET,SOCK_STREAM,0);

	bzero(&svaddr,sizeof(svaddr));
	svaddr.sin_family = AF_INET;
	svaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svaddr.sin_port = htons(atoi(argv[1]));

	bind(listenfd,(sockaddr *)&svaddr,sizeof(svaddr));
	listen(listenfd,10);

	flag = fcntl(listenfd,F_GETFL,0);
	fcntl(listenfd,F_SETFL,flag|O_NONBLOCK);

	int maxfd,connfd,sockfd;
	int client[FD_SETSIZE];
	fd_set rset,wset,allset;
	int nready,maxi;
	socklen_t cllen;
	char buf[1024];

	char to[FD_SETSIZE][MAXLINE],fr[FD_SETSIZE][MAXLINE];
	char *toiptr[FD_SETSIZE],*tooptr[FD_SETSIZE],*friptr[FD_SETSIZE],*froptr[FD_SETSIZE];
	map<int,string> fdfile;
	map<string,int> filesize;
	vector<int> byterecv(FD_SETSIZE,0);
	vector<int> bytesend(FD_SETSIZE,0);
	map<string,vector<string> > userfile;
	vector<int> fdnumfile(FD_SETSIZE,0);
	vector<int> downloading(FD_SETSIZE,-1);

	for(int i = 0;i < FD_SETSIZE;i++){
		toiptr[i] = tooptr[i] = to[i];
		friptr[i] = froptr[i] = fr[i];
	}
	
	maxfd = listenfd;
	maxi = -1;
	for(int i = 0;i < FD_SETSIZE;i++)
		client[i] = -1;

	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);
	while(true){
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(listenfd,&rset);
		for(int i = 0;i < FD_SETSIZE;i++){ // do fd set
			if(client[i] < 0) continue;
			if(toiptr[i] - tooptr[i] >= 10 || fdnumfile[i] < userfile[cliname[i]].size())
				FD_SET(client[i],&wset); // write to socket
			if(&fr[i][1024] - friptr[i] >= 1024)
				FD_SET(client[i],&rset); // read from socket
		}
		select(maxfd+1,&rset,&wset,NULL,NULL);
		if(FD_ISSET(listenfd,&rset)){
			cllen = sizeof(claddr);
			connfd = accept(listenfd,(sockaddr *)&claddr,&cllen);
			cout << connfd << " accepted" << endl;

			flag = fcntl(connfd,F_GETFL,0);
			fcntl(connfd,F_SETFL,flag|O_NONBLOCK);

			int i;
			for(i = 0;i < FD_SETSIZE;i++)
				if(client[i] < 0){
					client[i] = connfd;
					break;
				}
			FD_SET(connfd,&rset);
			if(connfd > maxfd) maxfd = connfd;
			if(i > maxi) maxi = i;
		}
		for(int i = 0;i <= maxi;i++){
			if(client[i] < 0) continue;

			if(FD_ISSET(client[i],&wset) && (toiptr[i] - tooptr[i] >= 10)){ // send
				int nw = send(client[i],(char *)tooptr[i],1024,0);
				if(nw > 0){
					tooptr[i] += 1024;
					if(tooptr[i] == toiptr[i]) {
						tooptr[i] = toiptr[i] = to[i];
					}
					FD_SET(client[i],&rset);
				}
			}

			bool recved = false;
			if(FD_ISSET(client[i],&rset) && (&fr[i][1024] - friptr[i] >= 1024)){ // recv
				int nr = recv(client[i],(char *)friptr[i],1024,0);
				if(nr == 0){ // client exit
					close(client[i]);
					cliname[i] = "";
					fdnumfile[i] = 0;
					toiptr[i] = tooptr[i] = to[i];
					friptr[i] = froptr[i] = fr[i];
					downloading[i] = -1;
					cout << client[i] << " exit" << endl;
					client[i] = -1;
					continue;
				}
				else if(nr > 0){
					friptr[i] += 1024;
					recved = true;
				}
			}

			if(fdnumfile[i] < userfile[cliname[i]].size() && toiptr[i] == tooptr[i]){ // not all file downloaded
				if(downloading[i] != -1){ // send file data
					toiptr[i][0] = '1';
					string dlfilename = userfile[cliname[i]][downloading[i]];
					ifstream downfile(dlfilename,ios::binary);
					downfile.seekg(bytesend[i],ios::beg);
					downfile.read(toiptr[i]+1,1023);
					downfile.close();
					toiptr[i] += 1024;
					bytesend[i] += 1023;
					if(bytesend[i] >= filesize[dlfilename]){ // send to eof
						cout << client[i] << " download complete " << dlfilename << endl;
						fdnumfile[i] += 1;
						downloading[i] = -1;
					}
				}
				else{ // send file name & size
					downloading[i] = fdnumfile[i];
					toiptr[i][0] = '0';
					string dlfilename = userfile[cliname[i]][fdnumfile[i]];
					strcpy(toiptr[i]+1,dlfilename.substr(cliname[i].length()+2).c_str()); // file name not including <username>__
					ifstream downfile(dlfilename,ios::binary);
					downfile.seekg(0,ios::end);
					int dlfilesize = downfile.tellg();
					downfile.close();
					memcpy(toiptr[i]+33,&dlfilesize,4);
					toiptr[i] += 1024;
					bytesend[i] = 0;
				}
			}

			if(recved){
				if(cliname.find(i) != cliname.end()){
				}
				if(cliname.find(i) == cliname.end() || cliname[i] == ""){ // still no name
					cliname[i] = string(froptr[i]);
					cout << "receive name " << froptr[i] << endl;
					toiptr[i][0] = '2';
					toiptr[i] += 1024;
				}
				else if(froptr[i][0] == '0'){ // command recving file name & size (/put)
					cout << "recv name size" << endl;
					char ulname[32];
					int ulsize;
					strcpy(ulname,froptr[i]+1);
					memcpy(&ulsize,froptr[i]+33,4);
					fdfile[i] = (cliname[i] + "__") + string(ulname); // file name become <user name>__<file name>
					filesize[fdfile[i]] = ulsize;
					ofstream upfile(fdfile[i],ios::binary);
					byterecv[i] = 0;
					upfile.close();
				}
				else if(froptr[i][0] == '1'){ // file content
					ofstream upfile(fdfile[i],ios::binary|ios::app);
					if(filesize[fdfile[i]] - byterecv[i] < 1023)
						upfile.write(froptr[i]+1,filesize[fdfile[i]] % 1023);
					else
						upfile.write(froptr[i]+1,1023);
					upfile.close();
					byterecv[i] += 1023;
					if(byterecv[i] >= filesize[fdfile[i]]){ // file upload complete
						cout << client[i] << " upload complete " << fdfile[i]  << byterecv[i] << endl;
						fdnumfile[i] += 1;
						userfile[cliname[i]].push_back(fdfile[i]);
					}
				}
				froptr[i] += 1024;
				if(froptr[i] == friptr[i]) froptr[i] = friptr[i] = fr[i];
			}
		}
	}
	return 0;
}
