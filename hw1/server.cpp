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
#include <cctype>
#include <vector>
#include <utility>
#include <map>
#include <sstream>

using namespace std;

int main(int argc, char **argv){

	if(argc != 2){
		cout << "argv[1]: port number" << endl;
		return 0;
	}

	struct sockaddr_in claddr,svaddr;
	int listenfd;

	listenfd = socket(AF_INET,SOCK_STREAM,0);

	bzero(&svaddr,sizeof(svaddr));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htonl(INADDR_ANY); // listen to any addr
	svaddr.sin_addr.s_addr = htonl(INADDR_ANY); // listen to any addr
	svaddr.sin_port = htons(atoi(argv[1]));

	bind(listenfd,(sockaddr *)&svaddr,sizeof(svaddr));
	listen(listenfd,10);

	int maxfd,connfd,sockfd;
	int client[FD_SETSIZE];
	fd_set rset,allset;
	int nready,maxi;
	socklen_t cllen;
	char buf[1024];
	vector<pair<string,string> > ipport_name;
	ipport_name.resize(FD_SETSIZE);

	maxfd = listenfd;
	maxi = -1;
	for(int i = 0;i < FD_SETSIZE;i++)
		client[i] = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);
	while(true){
		rset = allset;
		nready = select(maxfd+1,&rset,NULL,NULL,NULL);
		if(FD_ISSET(listenfd,&rset)){ // accept new client connection
			cllen = sizeof(claddr);
			connfd = accept(listenfd,(sockaddr *)&claddr,&cllen);
			char newaddr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET,&claddr.sin_addr,newaddr,sizeof(newaddr));
			printf("new client: %s, port %d\n",newaddr,ntohs(claddr.sin_port));
			string addrstr(newaddr);
			addrstr += ":";
			addrstr += to_string(ntohs(claddr.sin_port));
			ipport_name[connfd] = make_pair("anonymous",addrstr);
			cout << connfd << ": " << ipport_name[connfd].first << " " << ipport_name[connfd].second << endl;

			// send hello message to client
			string hellomsg("[Server] Hello, anonymous! From: ");
			hellomsg += addrstr + '\n';
			memset(&buf,0,sizeof(buf));
			strcpy(buf,hellomsg.c_str());
			send(connfd,(char *)&buf,strlen(buf),0);

			// send someone's coming to others
			memset(&buf,0,sizeof(buf));
			string coming("[Server] Someone is coming!\n");
			strcpy(buf,coming.c_str());
			int sendfd;
			for(int i = 0;i <= maxi;i++){
				if((sendfd = client[i]) < 0) continue;
				if(FD_ISSET(sendfd,&allset)){
					send(sendfd,(char *)&buf,strlen(buf),0);
				}
			}

			int i;
			for(i = 0;i < FD_SETSIZE;i++)
				if(client[i] < 0){
					client[i] = connfd;
					break;
				}
			FD_SET(connfd,&allset);
			if(connfd > maxfd) maxfd = connfd;
			if(i > maxi) maxi = i;
			if(--nready <= 0) continue;
		}
		for(int i = 0;i <= maxi;i++){
			if((sockfd = client[i]) < 0) continue;
			if(FD_ISSET(sockfd,&rset)){ // read from an online client
				memset(&buf,0,sizeof(buf));
				int temp = recv(sockfd,(char *)&buf,sizeof(buf),0); // receive
				if(temp == 0) {	// client exit
					close(sockfd);
					FD_CLR(sockfd,&allset);
					client[i] = -1;
					cout << sockfd << " exit" << endl;

					int tempfd;
					memset(&buf,0,sizeof(buf));
					string outmsg("[Server] ");
					outmsg += ipport_name[sockfd].first + " is offline.\n";
					strcpy(buf,outmsg.c_str());
					for(int j = 0;j <= maxi;j++){
						if((tempfd = client[j]) < 0) continue;
						if(FD_ISSET(tempfd,&allset)){
							send(tempfd,(char *)&buf,strlen(buf),0);
						}
					}
				}
				
				string line(buf);
				stringstream ss;
				ss << line;
				string command;
				ss >> command;
				if(command == "who"){ // list all users online
					int tempfd;
					for(int j = 0;j <= maxi;j++){
						if((tempfd = client[j]) < 0) continue;
						if(FD_ISSET(tempfd,&allset)){
							memset(&buf,0,sizeof(buf));
							string whomsg("[Server] ");
							whomsg += ipport_name[tempfd].first + " " + ipport_name[tempfd].second;
							if(i == j) whomsg += " ->me\n";
							else whomsg += "\n";
							strcpy(buf,whomsg.c_str());
							send(sockfd,(char *)&buf,strlen(buf),0);
						}
					}
				}
				else if(command == "name"){ // change name
					string newname;
					ss >> newname;
					bool canrename = true;
					int tempfd;
					bool containnoteng = false;
					for(int i = 0;i < newname.length();i++)
						if(!isalpha(newname[i])){
							containnoteng = true;
							break;
						}
					if(newname == "anonymous"){
						memset(&buf,0,sizeof(buf));
						strcpy(buf,"[Server] ERROR: Username cannot be anonymous.\n");
						send(sockfd,(char *)&buf,strlen(buf),0);
						canrename = false;
					}
					else if(newname.length() < 2 || newname.length() > 12 || containnoteng){
						memset(&buf,0,sizeof(buf));
						strcpy(buf,"[Server] ERROR: Username can only consists of 2~12 English letters.\n");
						send(sockfd,(char *)&buf,strlen(buf),0);
						canrename = false;
					}
					for(int j = 0;j <= maxi;j++){ // search duplicate name
						if((tempfd = client[j]) < 0) continue;
						if(FD_ISSET(tempfd,&allset) && i != j){
							if(ipport_name[tempfd].first == "anonymous") continue;
							if(newname == ipport_name[tempfd].first){
								string nameused("[Server] ERROR: ");
								nameused += newname + " has been used by others.\n";
								memset(&buf,0,sizeof(buf));
								strcpy(buf,nameused.c_str());
								send(sockfd,(char *)&buf,strlen(buf),0);
								canrename = false;
								break;
							}
						}
					}

					if(canrename){
						string oldname = ipport_name[sockfd].first;
						ipport_name[sockfd].first = newname;
						memset(&buf,0,sizeof(buf));
						string namemsg("[Server] You're now known as ");
						namemsg += newname + ".\n";
						strcpy(buf,namemsg.c_str());
						send(sockfd,(char *)&buf,strlen(buf),0);
						
						memset(&buf,0,sizeof(buf));
						string namebcast("[Server] ");
						namebcast += oldname + " is now known as ";
						namebcast += newname + ".\n";
						strcpy(buf,namebcast.c_str());
						for(int j = 0;j <= maxi;j++){
							if((tempfd = client[j]) < 0) continue;
							if(FD_ISSET(tempfd,&allset) && i != j){
								send(tempfd,(char *)&buf,strlen(buf),0);
							}
						}
					}
				}
				else if(command == "yell"){ // broadcast message
					int tempfd;
					string yellmsg("[Server] ");
					yellmsg += ipport_name[sockfd].first + " ";
					yellmsg += ss.str();
					memset(&buf,0,sizeof(buf));
					strcpy(buf,yellmsg.c_str());
					for(int j = 0;j <= maxi;j++){
						if((tempfd = client[j]) < 0) continue;
						if(FD_ISSET(tempfd,&allset)){
							send(tempfd,(char *)&buf,strlen(buf),0);
						}
					}
				}
				else if(command == "tell"){ // private message
					bool cantell = true;
					string myname = ipport_name[sockfd].first;
					if(myname == "anonymous"){
						cantell = false;
						memset(&buf,0,sizeof(buf));
						strcpy(buf,"[Server] ERROR: You are anonymous.\n");
						send(sockfd,(char *)&buf,strlen(buf),0);
					}

					int tellfd = -1,tempfd;
					string telluser;
					ss >> telluser;
					if(telluser == "anonymous"){
						memset(&buf,0,sizeof(buf));
						strcpy(buf,"[Server] ERROR: The client to which you sent is anonymous.\n");
						send(sockfd,(char *)&buf,strlen(buf),0);
					}
					else{
						for(int j = 0;j <= maxi;j++){
							if((tempfd = client[j]) < 0) continue;
							if(ipport_name[tempfd].first == telluser){
								tellfd = tempfd;
								break;
							}
						}
						if(tellfd == -1){
							cantell = false;
							memset(&buf,0,sizeof(buf));
							strcpy(buf,"[Server] ERROR: The receiver doesn't exist.\n");
							send(sockfd,(char *)&buf,strlen(buf),0);
						}
						if(cantell){
							memset(&buf,0,sizeof(buf));
							strcpy(buf,"[Server] SUCCESS: Your message has been sent.\n");
							send(sockfd,(char *)&buf,strlen(buf),0);
							string tellmsg("[Server] ");
							tellmsg += ipport_name[sockfd].first + " tell you";
							int startpos = line.find(' ',5);
							tellmsg += line.substr(startpos);
							memset(&buf,0,sizeof(buf));
							strcpy(buf,tellmsg.c_str());
							send(tellfd,(char *)&buf,strlen(buf),0);
						}
					}
				}
				else{ // error command
					memset(&buf,0,sizeof(buf));
					strcpy(buf,"[Server] ERROR: Error command.\n");
					send(sockfd,(char *)&buf,strlen(buf),0);
				}
			}
		}

	}

	return 0;
}
