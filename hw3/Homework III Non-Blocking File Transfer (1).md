
# Homework III Non-Blocking File Transfer
In this homework, you have to practice Nonblocking I/O in a network program.

## Introduction
You should write a server and a client. The server has to be a single-process, single-thread, and non-blocking server, and all connections are TCP in this homework.

One user can login the server on different hosts at the same time. Each client belongs to the same user should have the same view of files on the server.

The scenario of this homework is similar to Dropbox:
- A user can upload and save his files on the server.
- The clients of the user are running on different hosts.
- When any of client of the user uploads a file, the server has to transmit it to all other clients.
- When a new client connects to the server, the server should transmit all the files, which have been uploaded by the other clients, to the new client immediately.
- We will type `put <filename>` on different clients at the same time, and your programs have to deal with this case. 
- If one of the clients is sleeping, the server should send the file data to other clients in a non-blocking way.
- The uploading data only need to be sent to the clients that belong to the same user.
- If the file uploaded by the client already exits(A file with the same file name already exists on the server), it must be replaced.
- Your program must handle various file sizes and types 

:::info
**Note**: In your server program, you can include the following two lines to achieve the non-blocking function.
```cpp
int flag = fcntl(sock, F_GETFL, 0);
fcnctl(sock, F_SETTFL, flag|O_NONBLOCK);
```
:::

Please confirm that your server operates in the non-blocking mode. We will purposely let the socket send buffer full to test the correctness of your server.

## Inputs
   1. `./server <port>`
      Please make sure that you execute the server program in this format.
   2. `./client <ip> <port> <username>`
	  Please make sure that you execute the client program in this format.

   3. `put <filename>`
      This command, which is executed on the client side by the user, is to upload your files to the server side.
      Users can transmit any files they want. But these files, after being received by the server, need to be stored in the same directory created for the user on the server side.
      Each file should be sent to the other clients belonging to the same user.
      The file to be uploaded should reside in the same directory with the client program.

   4. `sleep <seconds>`
      This command is to let the client sleep for the specified period of time.

   5. `exit`
      This command is to disconnect with the server and terminate the program.


## Outputs
   1. Welcome message. (displayed at the client side)
```
Welcome to the dropbox-like server: <username>
```
   2. Uploading progess bar. (displayed at the sending client side)
```
[Upload] <filename> Start!
Progress : [######################]
[Upload] <filename> Finish!
```
   3. Downing progess bar. (displayed at the receiving client side)
```
[Download] <filename> Start!
Progress : [######################]
[Download] <filename> Finish!
```
   4. Sleeping count down. (displayed at the client side)
```
sleep 20
The client starts to sleep.
Sleep 1
.
.
Sleep 19
Sleep 20
Client wakes up.   
```

## Test Cases
### Steps
   1. First, make sure that two clients of userA have connected to the server. After one client uploads a test file to the server, the other should receive that test file from the server.
   2. When a new client of userA connects to the server (now there are totally three clients of userA on ther server), the server has to transmit the test file uploaded in step 1 to this new client.
   3. Two clients of userA upload a test file with different names respectively at the same time.
         At the end of file transmission, all clients of userA must have the same set of files. That is, the two sending clients should receive the file uploaded by each other, and the other client should receive both files.
   5. After a new client of userA connects to the server (now there are totally four clients of userA on ther server), we will execute `sleep 20` on this client to force it to sleep for 20 seconds. Within this 20 seconds, we will let another client transmit a file to the server. After receiving the file, the server will transmit it to all the other clients of the user, including the **"sleeping"** one. Your server should continually send the file data to clients even when one of the client is sleeping.
   5. When a new client of userB connects to the server, suppose a client of userA upload a test file, the server should not send the test file to the client of userB.
   6. Exit all clients. 

:::info
**Note**: 
We will use **"diff"** to compare the files sent and the file received to check whether they are the same to ensure that the file transfers are complete.
You **cannot** use **"fork"** system call or **"thread"** function call in this homework. Your server must be a single-process, single-thread, and non-blocking server.
:::

:::info
**Note**:  

When a client receives a file from the server, it has to store the file in the current directory of the client. 
```
current directory of client
├── client
├── testfile1
├── testfile2
└── ……
```
When a user first uploads a file to the server, the server will create a directory for the user which is named after the user's name in the current directory of the server.

Then, your server should store the files which belong to the user in his directory.
```
current directory of server
├── server
└── tom
     ├── testfile1
     └── ……
```
:::   
### Output
The following cases will be executed one by one.

#### **[Execute Server]**
```shell
[user@inp1 ~/hw3 ] ./server 8888
```
#### **[Case 1: Upload a file]**
```shell
[user@inp1 ~/hw3 ] cp testfile 0/testfile 

# In Terminal 0
[user@inp1 ~/hw3/0 ] ./client 127.0.0.1 8888 tom
Welcome to the dropbox-like server: tom

# In Terminal 1
[user@inp1 ~/hw3/1 ] ./client 127.0.0.1 8888 tom
Welcome to the dropbox-like server: tom

# In Terminal 0
[user@inp1 ~/hw3/0 ] 
put testfile
[Upload] testfile Start!
Progress : [######################]
[Upload] testfile Finish!

# In Terminal 1
[user@inp1 ~/hw3/1 ] 
[Download] testfile Start!
Progress : [######################]
[Download] testfile Finish!

[user@inp1 ~/hw3 ] diff 0/testfile 1/testfile
```
#### **[Case 2: A new client of the same user logs in]**
```shell
# Terminal 2
[user@inp1 ~/hw3/2 ] ./client 127.0.0.1 8888 tom
Welcome to the dropbox-like server: tom
[Download] testfile Start!
Progress : [######################]
[Download] testfile Finish!

[user@inp1 ~/hw3 ] diff 0/testfile 2/testfile
```  

#### **[Case 3: Upload files from different clients at the same time]**
```shell
[user@inp1 ~/hw3 ] cp testfile2 0/testfile2
[user@inp1 ~/hw3 ] cp testfile3 1/testfile3

# Terminal 0
[user@inp1 ~/hw3/0 ] 
put testfile2
[Upload] testfile2 Start!
Progress : [######################]
[Upload] testfile2 Finish!

# Terminal 1
[user@inp1 ~/hw3/1 ] 
put testfile3
[Upload] testfile3 Start!
Progress : [######################]
[Upload] testfile3 Finish!

# Terminal 2
[user@inp1 ~/hw3/2 ] 
[Download] testfile2 Start!
Progress : [######################]
[Download] testfile2 Finish!
[Download] testfile3 Start!
Progress : [######################]
[Download] testfile3 Finish!

# Terminal 0
[user@inp1 ~/hw3/0 ] 
[Download] testfile3 Start!
Progress : [######################]
[Download] testfile3 Finish!

# Terminal 1
[user@inp1 ~/hw3/1 ] 
[Download] testfile2 Start!
Progress : [######################]
[Download] testfile2 Finish!

[user@inp1 ~/hw3 ] diff 0/testfile2 1/testfile2
[user@inp1 ~/hw3 ] diff 0/testfile2 2/testfile2
[user@inp1 ~/hw3 ] diff 1/testfile3 2/testfile3
[user@inp1 ~/hw3 ] diff 1/testfile3 0/testfile3
```  
#### **[Case 4: Put a client to sleep]**
```shell
[user@inp1 ~/hw3 ] cp testfile4 0/testfile4

# Terminal 3
[user@inp1 ~/hw3/3 ] ./client 127.0.0.1 8888 tom
Welcome to the dropbox-like server: tom
sleep 20
The client starts to sleep.
Sleep 1
.
.
Sleep 19
Sleep 20
Client wakes up. 

# Terminal 0
[user@inp1 ~/hw3/0 ] 
put testfile4
[Upload] testfile4 Start!
Progress : [######################]
[Upload] testfile4 Finish!

# Terminal 1
[user@inp1 ~/hw3/1 ] 
[Download] testfile4 Start!
Progress : [######################]
[Download] testfile4 Finish!

# Terminal 2
[user@inp1 ~/hw3/2 ] 
[Download] testfile4 Start!
Progress : [######################]
[Download] testfile4 Finish!

# Terminal 3
[user@inp1 ~/hw3/3 ] 
[Download] testfile4 Start!
Progress : [######################]
[Download] testfile4 Finish!

[user@inp1 ~/hw3 ] diff 0/testfile4 1/testfile4
[user@inp1 ~/hw3 ] diff 0/testfile4 2/testfile4
[user@inp1 ~/hw3 ] diff 0/testfile4 3/testfile4
```

#### **[Case 5: Separation of different users]**
```shell
[user@inp1 ~/hw3 ] cp testfile5 0/testfile5
# Terminal 4
[user@inp1 ~/hw3/4 ] ./client 127.0.0.1 8888 frank
Welcome to the dropbox-like server: frank

# Terminal 0
[user@inp1 ~/hw3/0 ] 
put testfile5
[Upload] testfile5 Start!
Progress : [######################]
[Upload] testfile5 Finish!
# Terminal 1
[user@inp1 ~/hw3/1 ] 
[Download] testfile5 Start!
Progress : [######################]
[Download] testfile5 Finish!

# Terminal 2
[user@inp1 ~/hw3/2 ] 
[Download] testfile5 Start!
Progress : [######################]
[Download] testfile5 Finish!

# Terminal 3
[user@inp1 ~/hw3/3 ] 
[Download] testfile5 Start!
Progress : [######################]
[Download] testfile5 Finish!

[user@inp1 ~/hw3 ] diff 0/testfile5 1/testfile5
[user@inp1 ~/hw3 ] diff 0/testfile5 2/testfile5
[user@inp1 ~/hw3 ] diff 0/testfile5 3/testfile5
[user@inp1 ~/hw3 ] [ -e 4/testfile5 ]
```
#### **[Case 6: Exit]** 
```shell
# Terminal 0
[user@inp1 ~/hw3/0 ] exit
# Terminal 1
[user@inp1 ~/hw3/1 ] exit
# Terminal 2
[user@inp1 ~/hw3/2 ] exit
# Terminal 3
[user@inp1 ~/hw3/3 ] exit
# Terminal 4
[user@inp1 ~/hw3/4 ] exit
```

## Homework Submission Requirements
* The due date of this homework is **2022/1/3 MON 23:55**
* Submit your code to e3 and put all code under same folder named <Student ID> (DO NOT ZIP FILES). 
* It's not necessary to upload your binaries as well since we will recompile your code from the sources.
* The `Makefile` must be in your submission. Feel free to add your own header files or set the file name yourselves. Remember to modify the `Makefile` to fit it into your program structure.
* Sample directory structure before `make`. 
```
/
├── client.c
├── server.c
└── Makefile      # your makefile, please refer to the sample provided by TA
```
- Sample directory structure after `make`
```
/
├── client.c
├── server.c
├── client        # client binary, must be generated by 'make' command
├── server        # server binary, must be generated by 'make' command
└── Makefile      # your makefile, please refer to the sample provided by TA
```
    
## Grading Policy
We will score your code by six test cases using script.
```
[user@inp1 ~ ] ./test <path to the program's directory> <port>  <output mode>
```
- Output modes
    - 2 to print server/client messages
    - 1 to print client messages
    - 0 print nothing
- If your program passes these six test cases, you will get 100 points.
- If your program did not output countdown and wake up message on the terminal of the client that executed the sleep command, you will not get the score of case 4.
- Furthermore, if the sleeping client get the test file uploaded by other client before he wakes up, you will not get the score of case 4.
- We will not test sending empty files or non-existent files, but your program must handle files of any size.
- You can only use C or C++ in this homework.

### Penalty
* If program implement by "fork" system call or "thread" function call in this homework you will get 0.
* Each problem that leads to fail to run (compile) your code will discount your score by 0.85.
* Late submission: final score = original score $\times$ $(\frac{3}{4})^{n}$, where ${n}$ is the number of late days.
    