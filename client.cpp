#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<vector>
#include<signal.h>
#include<string.h>
#include<pthread.h>
#include <termios.h>

#include<bits/stdc++.h>

using namespace std;

#define MAIN_NIC "127.0.0.1"
#define PORT 8080
#define BUF_SZ 2048
#define MSG_SZ 1024
#define CHAT_NAME_SZ 64

int sockfd;
struct sockaddr_in server_address;
struct in_addr server_ip;
int server_address_len = sizeof(server_address);
int convert_ret, connect_ret;
pthread_t send_thread, read_thread;

char chat_name[CHAT_NAME_SZ];

//this function remove newline ('\n') from a given string 
void replace_newline(char *str) {
    char *pos;
    while ((pos = strchr(str, '\n')) != NULL) {
        memmove(pos, pos + 1, strlen(pos));
    }
}

//this function get chat name which is typed from user
void get_chat_name() {
    printf("Enter your chat name: ");
    fgets(chat_name, CHAT_NAME_SZ, stdin);
    replace_newline(chat_name);
    strcat(chat_name, ": "); 
}

//this function handle singal when user type CTRL_Z or CTRL_C to close client's sockfd
void sig_handler_ctrlC_Z(int signo) {
    if(signo == SIGINT || signo == SIGTSTP) {
        close(sockfd);
        printf("closed client_fd\n");
        exit(EXIT_SUCCESS);
    }
}

//this function performs send message from client to server and registered by send_thread
void* send_message(void* tid) {
    while(1) {
        char message[MSG_SZ], full_message[MSG_SZ + 100];
        full_message[0] = '\0';
        tcflush(STDIN_FILENO, TCIFLUSH); //remove unread data (TCIFLUSH) from file descriptor - STDIN_FILENO (0), handle to remove data if user type message before server can read their message
        fgets(message, MSG_SZ, stdin);
        replace_newline(message);
        // printf("full_message = %s\n", full_message);
        // printf("message = %s\n", message);

        //handle close sockfd when user type 'quit'
        if(strcasecmp(message, "quit") == 0) {
            close(sockfd);
            printf("close sockfd & prepare for exit\n");
            exit(EXIT_SUCCESS);
        }
        
        strcat(full_message, chat_name);
        strcat(full_message, message);

        //send full_message to client by sockfd
        int send_ret = send(sockfd, full_message, strlen(full_message), 0);
        // printf("send_ret = %d\n", send_ret);
        if(send_ret < 0) {
            perror("send_ret");
        }

        // printf("sent to server success, full_message = %s\n", full_message);
    }
}

//this function performs read message from server and registered by read_thread
void* read_message(void* tid) {
    while(1) {
        char buffer[BUF_SZ];
        
        //read data from server by sockfd to buffer
        int read_ret = read(sockfd, buffer, BUF_SZ);
        //printf("read_ret = %d\n", read_ret);

        if(read_ret < 0) {
            perror("read_ret");
        }

        //indicate that server close server's socket and send EOF message
        if(read_ret == 0) {
            printf("server close socket.., quit chat now\n");
            close(sockfd);
            printf("closed sockfd\n");
            exit(EXIT_SUCCESS);
        }

        //else, print message from buffer
        buffer[read_ret] = '\0';
        printf("%s\n", buffer);
    }
}

int main() {

    // register signal to perform handle signal when user type CtrlZ or CtrlC
    signal(SIGINT, sig_handler_ctrlC_Z);
    signal(SIGTSTP, sig_handler_ctrlC_Z);
    get_chat_name();
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // create socket
    printf("sockfd = %d\n", sockfd);

    if(sockfd < 0) {
        perror("create socket");
        exit(EXIT_FAILURE);
    }

    printf("create socket success\n");

    server_address.sin_family = AF_INET;
    convert_ret = inet_pton(AF_INET, MAIN_NIC, &server_ip); //convert MAIN_NIC IP to binary IP Address

    if(convert_ret < 0) {
        perror("convert IPv4");
        exit(EXIT_FAILURE);
        close(sockfd);
    }

    printf("convert ipv4 success\n");

    server_address.sin_addr.s_addr = server_ip.s_addr;
    server_address.sin_port = htons(PORT);

    connect_ret = connect(sockfd, (struct sockaddr*)&server_address, server_address_len); //send a request to server to connect, initialize TCP three-way handshake
    printf("connect_ret = %d\n", connect_ret);

    if(connect_ret < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("connect success\n");

    char buffer[BUF_SZ];
    int read_ret = read(sockfd, buffer, BUF_SZ);
    // printf("read_ret = %d\n", read_ret);

    if(read_ret < 0) {
        perror("read_ret");
    }

    if(read_ret == 0) {
        printf("server close socket.., quit chat now\n");
        close(sockfd);
        printf("closed sockfd\n");
        exit(EXIT_SUCCESS);
    }

    buffer[read_ret] = '\0';
    printf("%s\n", buffer);

    //this active variable checks if server allows client to chat (or client have to wait in ready queue)
    bool active = 0;
    char active_verify[2];
    read_ret = read(sockfd, active_verify, 1);
    printf("read_ret (active) = %d\n", read_ret);

    if(read_ret < 0) {
        perror("read_ret");
    }

    if(read_ret == 0) {
        printf("server close socket.., quit chat now\n");
        close(sockfd);
        printf("closed sockfd\n");
        exit(EXIT_SUCCESS);
    }

    active_verify[read_ret] = '\0';
    printf("active_verify = %s\n", active_verify);

    // if active is 1, indicates server allows client to chat
    if(strcasecmp(active_verify, "1") == 0) {
        active = 1;
        printf("preparing for creating threads..\n");
    }

    //create threads for reading message and sending message
    if(active) {
        if (pthread_create(&read_thread, NULL, read_message, (void*)1) != 0) {
            perror("pthread_create read_thread");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&send_thread, NULL, send_message, (void*)2) != 0) {
            perror("pthread_create send_thread");
            exit(EXIT_FAILURE);
        }

        pthread_join(read_thread, NULL);
        pthread_join(send_thread, NULL);
    }
    close(sockfd);
    return 0;
}