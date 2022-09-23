#  Homework2 UDP Reliable File Transfer
## Overview
In this homework, you need to implement a UDP reliable file transfer protocol on the application level using 3 different timeout mechanisms.

The UDP reliable file transfer protocol includes a sender and a receiver. The sender keeps sending the content of a file to the receiver by using UDP packets. The receiver keeps receiving the content from the sender and outputs the received file content to another file. **Important: the maximum size of every UDP datagram (packet) transfered is limited to 1024 Bytes.(Include your packet header)** (You can test this by using `tcpdump`.)

To cope with the packet loss in the network, the sender and receiver should **detect the packet loss events** (by using the timeout methods below) and deal with them (by re-transmitting lost packets). 


## Specification

You should implement senders and receivers using 3 different timeout methods, respectively:
* Timeout using SIGALRM
* Timeout using select
* Timeout using setsockopt

You can write as many source files as you need in this homework, and you can freely name your source files. However, after you compile your codes with Makefile, you have to make sure that **there will be 6 executables generated, and named as follows**:

* `sender_sigalrm`
* `receiver_sigalrm`
* `sender_select` 
* `receiver_select`
* `sender_sockopt`
* `receiver_sockopt`
>
> sender   - UDP client
>
> receiver - UDP server (you can reuse)
>
>
>Note: The receiver must NOT read a file locally to pretend that it has received the file from the sender.
>

You can output whatever information you need on the screen (`stdout`). We will only examine if the file sent and the file received are the same.

You can only use C or C++ in this homework.

### SIGALRM
`sender_sigalrm` will send a file to `receiver_sigalrm`, and `receiver_sigalrm` will save the file. Due to the unreliable underlying network conditions, your `sender_sigalrm` and `receiver_sigalrm` must use **SIGALRM** to implement your retransmission mechanism.

The command to start the receiver:
```shell
receiver_sigalrm [save filename] [bind port]
```

The command to start the sender:
```shell
sender_sigalrm [send filename] [target address] [connect port]
```

For example, if the sender is about to send a file named `a.txt` to the receiver, whose IP address and port are `127.0.0.1` and `5000`, and the receiver will save the file as `b.txt`:
* Start the receiver first:
```shell
receiver_sigalrm b.txt 5000
```
* Then start the sender:
```shell
sender_sigalrm a.txt 127.0.0.1 5000
```
* After finishing the file transfer, **both `sender_sigalrm` and `receiver_sigalrm` should terminate automatically**.
* We will test if `a.txt` is the same as `b.txt`.

> Note: For the **SIGALRM** timeout method, you need to use **siginterrupt** to allow the SIGALRM signal to interrupt a system call.
> 

### select
`sender_select` will send a file to `receiver_select`, and `receiver_select` will save the file. Due to the unreliable underlying network conditions, your `sender_select` and `receiver_select` must use **select** to implement your retransmission mechanism.

The command to start the receiver:
```shell
receiver_select [save filename] [bind port]
```

The command to start the sender:
```shell
sender_select [send filename] [target address] [connect port]
```

For example, if the sender is about to send a file named `a.txt` to the receiver, whose IP address and port are `127.0.0.1` and `5000`, and the receiver will save the file as `b.txt`:
* Start the receiver first:
```shell
receiver_select b.txt 5000
```
* Then start the sender:
```shell
sender_select a.txt 127.0.0.1 5000
```
* After finishing the transmission,  **both `sender_select` and `receiver_select` should terminate automatically**.
* We will test if `a.txt` is the same as `b.txt`.

### setsockopt
`sender_sockopt` will send a file to `receiver_sockopt`, and `receiver_sockopt` will save the file. Due to the unreliable underlying network conditions, your `sender_sockopt` and `receiver_sockopt` must use **setsockopt** to implement your retransmission mechanism.

The command to start the receiver:
```shell
receiver_sockopt [save filename] [bind port]
```

The command to start the sender:
```shell
sender_sockopt [send filename] [target address] [connect port]
```

For example, if the sender is about to send a file named `a.txt` to the receiver, whose IP address and port are `127.0.0.1` and `5000`, and the receiver will save the file as `b.txt`:
* Start the receiver first:
```shell
receiver_sockopt b.txt 5000
```
* Then start the sender:
```shell
sender_sockopt a.txt 127.0.0.1 5000
```
* After finishing the transmission, you have to **both `sender_sockopt` and `receiver_sockopt` should terminate automatically**.
* We will test if `a.txt` is the same as `b.txt`.

## Performance (Bonus +20)
**You are free to decide whether or not to enroll in this competition to earn at most 20 bonus.**

If you decide to enroll in the competition, you have to sign up the registration form, and you can only choose **one  retransmission mechanism that you implement (SIGALRM, select, or setsockopt)** to participate in the competition.

There are two performance metrics to measure your program's performance under unreliable underlying network conditions (packet loss, limited link bandwidth, non-zero RTT, and variable queuing delay):
1. File Transfer Time (sec) (${r_1}$) - The shorter, the better.
2. Total Amount of Transfered Data (Bytes) (${r_2}$) - The smaller, the better.

Suppose there are ${M}$ people to participate in this competition, and you are ranked ${r_1}$ and ${r_2}$ in the two performance metrics, respectively. Therefore, among total ${M}$ students, your final ranking would be ${(r_1+r_2)}$. After that, we will sort all students with their final ranking. If you are ranked ${f}$, you will receive ${20 \times (1 - \frac{f}{M})}$ bonus.

For example, suppose there are ${30}$ students enrolling in the competition. Further suppose that in R1 you are ranked ${15}$ and in R2 you are ranked ${3}$. Therefore, your total ranking would be ${18}$. Suppose that the final ranking sequence is ${2, 3, 3, 7, 10, 13, 14, 17, 18, 18, ..., N}$. In the sequence, ${18}$ is the nineth number, so you will get ${20 \times (1 - \frac{9}{30}) = 14}$ bonus.

**Note that if the program you choose cannot be compiled or transfer the file correctly, you will be eliminated from the competition and get 0 bonus even though you have registered for the competition.** (However, you will not get any penalty if that happens.)


## Network Environment Settings
We will test if your program can work correctly under specific **bi-directional packet loss rates**. We will randomly drop your packets and check whether the results are still correct.

To test your programs, you need to simulate the network scenarios (packet loss rate), and check if the file received is no different from the input.

Use the tool `tc` to simulate packet losses. `tc` is a linux built-in command line program to control the network traffic in the kernel. With `tc`, you can set network state parameters on a network interface card.

> You should **NOT** use `tc` on the workstation (i.e.  inplinux, bsd, and linux). **Instead, you should do that on your own PC.**
> 
> Because it requires *administrator permission* (i.e., `sudo`) to use `tc`, you can only use `tc` **on your local pc** to test if your senders and receivers work correctly when packet losses and re-ordering occur.
> 
> You can still test your programs on the workstations to check if they can be built successfully with your Makefile, and check if they can work correctly in the ideal case (no packet loss, no link bandwidth limitation, and RTT is 0).
> 

Refer to [man tc](http://man7.org/linux/man-pages/man8/tc.8.html) for more information.


### Add a rule to set the packet loss rate, queuing delay, link bandwidth, and re-ordering
In your linux shell, use the following command to set the **packet loss rate**, **queuing delay**, **link bandwidth**, and **re-ordering**:
```shell
sudo tc qdisc add dev <Device> root netem loss random <Packet Loss Rate> delay <Delay Time> <Variation> distribution <Distribution> rate <Link Bandwidth> reorder <Percent>
```
For example, you can add a rule on egress to set the packet loss rate to **5%**, set the queuing delay to between **7 ms~13 ms** (i.e., 10ms +- 3ms) with normal distribution, set the link bandwidth to **200 kbps**, and set packet re-ordering probability to **25%** on device "lo".
```shell
sudo tc qdisc add dev lo root netem loss 5% delay 10ms 3ms distribution normal rate 200kbit reorder 25%
```

### Delete all rules on a device
In your linux shell, use the following command:
```shell
sudo tc qdisc del dev <Device> root
```
For example, you can delete all rule settings on egress on the device "lo".
```shell
sudo tc qdisc del dev lo root
```
> Note: If there is an error message like `RTNETLINK answers: File exists` when you add a rule, try to delete all rules first.
> 

### Display all rules on a device
In your linux shell, use the following command:
```shell
sudo tc qdisc show dev <Device>
```

For example, you can display all rule settings on the device “lo”.
```shell
sudo tc qdisc show dev lo
```

## Generate Test Files Randomly
You can use `dd` to generate a test file of a specific file size with random contents. `dd` is a linux built-in command line program used to convert and copy a file.
> Refer to [man dd](http://man7.org/linux/man-pages/man1/dd.1.html) for further information.

Type the following command in your linux shell to generate one file named `<Ouput File Name>` with the file size `<File Size>`:
```shell
dd if=/dev/urandom of=<Output File Name> bs=<File Size> count=1
```
For example, the following command generates a file named `a_big_file` with file size = `1MB`:
```shell
dd if=/dev/urandom of=a_big_file bs=1MB count=1
```

## Check Correctness
Use the tool `diff` to check if the file transferd is correct. `diff` is also a built-in linux command line tool.

In your linux shell, use the following command:
```shell
diff <File 1> <File 2>
```
For example, you can check if the file `a_file` is equal to `b_file`.
```shell
diff a_file b_file
```

If there is no message output on the screen, it means that `a_file` is equal to `b_file`, otherwise it will show the differences between `a_file` and `b_file`.

## Homework Submission Requirements
* The due date of this homework is **2021/12/13 MON 23:55**
* Submit your code to e3 and put all code under same folder named <Student ID> **(DO NOT ZIP FILES)**. 
* It's not necessary to upload your binaries as well since we will recompile your code from the sources.
* The `Makefile` must be in your submission. Feel free to add your own header files or set the file name yourselves. Remember to modify the `Makefile` to fit it into your program structure.
* Sample directory structure before `make`. 

The directory structure in your file should be as follows (for instance, if you have only a `Makefile` and three couples of sender/receiver .c files):
```
301234567
├── Makefile
├── sender_sigalrm.c/cpp
├── receiver_sigalrm.c/cpp
└── ……
```

## Grading Policy
1. Timeout using SIGALRM ***(30%)***
    * There are 3 test cases (**10% for each**) with different network parameter values and file sizes.
    * We will test your programs with packet loss rate less than 10% and the transferred file size will be less than 1MB.
    * Your programs should finish file transferring in 1 minute.
    * Both `sender_sigalrm` and `receiver_sigalrm` should work correctly.
    * The file sent and the file received should be the same.
    * If your programs get blocked without making any progress, you will get 0 points.
    
2. Timeout using select       ***(30%)***
    * There are 3 test cases (**10% for each**) with different network parameter values and file sizes.
    * We will test your programs with packet loss rate less than 10% and the transferred file size will be less than 1MB.
    * Your programs should finish file transferring in 1 minute.
    * Both `sender_select` and `receiver_select` should work correctly.
    * The file sent and the file received should be the same.
    * If your programs get blocked without making any progress, you will get 0 points.
4. Timeout using setsockopt ***(30%)***
    * There are 3 test cases (**10% for each**) with different network parameter values and file sizes.
    * We will test your programs with packet loss rate less than 10% and the transferred file size will be less than 1MB.
    * Your programs should finish file transferring in 1 minute.
    * Both `sender_sockopt` and `receiver_sockopt` should work correctly.
    * The file sent and the file received should be the same.
    * If your programs get blocked without making any progress, you will get 0 points.
6. Makefile        ***(10%)***
    * Generate the six executables named above.
7. Performance                 ***(+20%)***
    * We will test your program with different packet loss, limited link bandwidth, non-zero RTT, and variable queuing delay.



### Penalty
* Submit your files with a zip file: -15%
* Incorrect upload format (e.g., wrong directory structure, wrong file names): -15%
* The maximum size of the transfered UDP datagram (packet) is greater than 1024 Bytes: -30%
* Fail to compile all your codes with the `make` command: -10%
* Late submission: final score = original score $\times$ $(\frac{3}{4})^{n}$, where ${n}$ is the number of late days
