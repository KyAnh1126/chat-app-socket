#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<vector>
#include<signal.h>
#include<pthread.h>
#include<string.h>
#include<semaphore.h>

#include<bits/stdc++.h>

using namespace std;

#define MAIN_NIC "127.0.0.1"
#define PORT 8080
#define MAX_CON 3
#define BUF_SZ 2048

int sockfd;
struct sockaddr_in server_address;
struct in_addr server_ip;
int server_address_len = sizeof(server_address);
int convert_ret, bind_ret, listen_ret, accept_ret, pthread_creat_ret;
vector<int> client_sockfds;
vector<int> client_queue;
int new_socket;
pthread_t threads[MAX_CON + 3];
sem_t semaphore;
pthread_mutex_t mutex_lock;

int optval = IP_PMTUDISC_WANT;

//this function handle singal when user type CTRL_Z or CTRL_C to close client's sockfd
void sig_handler_ctrlC_Z(int signo) {
    if(signo == SIGINT || signo == SIGTSTP) {
        close(sockfd);
        for(int client : client_sockfds) close(client);
        for(int client : client_queue) close(client);
        printf("closed server_fd\n");
        exit(EXIT_SUCCESS);
    }
}

//this function performs close a specific socket and erase socket from a vector client_sockfds
void close_erase(int socket) {
    close(socket);
    for(int i = 0; i < client_sockfds.size(); ++i) {
        if(client_sockfds[i] == socket) {
            client_sockfds.erase(client_sockfds.begin() + i);
            return;
        }
    }
    for(int i = 0; i < client_queue.size(); ++i) {
        if(client_queue[i] == socket) {
            client_queue.erase(client_queue.begin() + i);
            return;
        }
    }
}

//this function performs read a message from a client, and send it to all clients in client_sockfds (except the one send message), is registered by a thread element in threads array
void* solve_client(void* client_socket) {
    sem_wait(&semaphore); //client waits if full people in room
    long client_sockfd = (long)client_socket;
    printf("client_sockfd = %ld\n", client_sockfd);

    //add new socket to a vector
    client_sockfds.push_back(client_sockfd);

    //erase a socket from a client_queue (client_queue contains socket in waiting queue)
    for(int i = 0; i < client_queue.size(); ++i) {
        int client = client_queue[i];
        if(client == client_sockfd) client_queue.erase(client_queue.begin() + i);
    }

    //sending active message to allow client to chat with others
    char active_message[] = "1";

    int send_ret = send(client_sockfd, active_message, strlen(active_message), 0);
    if(send_ret < 0) {
        perror("send to client");
        close_erase(client_sockfd);
        sem_post(&semaphore);
        pthread_exit(NULL);        
    }
    //finish sending active message

    //sending ready-to-chat message
    char ready_to_chat_message[] = "Let's go! (type 'quit' to quit chat)";
    send_ret = send(client_sockfd, ready_to_chat_message, strlen(ready_to_chat_message), 0);

    if(send_ret < 0) {
        perror("send to client");
        close_erase(client_sockfd);
        sem_post(&semaphore);
        pthread_exit(NULL);
    }
    //finish sending ready-to-chat message

    while(1) {
        char buffer[BUF_SZ];

        int read_ret = read(client_sockfd, buffer, BUF_SZ);

        printf("\nread_ret = %d\n", read_ret);
        if(read_ret < 0) {
            perror("read from client");
            close_erase(client_sockfd);
            sem_post(&semaphore);
            pthread_exit(NULL);
        }

        //client close client's socket, send EOF message to server, and server perform close_erase() function
        if(read_ret == 0) {
            printf("client id = %ld quit chat\n", client_sockfd);
            close_erase(client_sockfd);
            sem_post(&semaphore);
            pthread_exit(NULL);
        }

        buffer[read_ret] = '\0';

        printf("read from client success\n");
        printf("message from client: %s\n", buffer);

        //client quit chat
        if(strcasecmp(buffer, "quit") == 0) {
            printf("close client_socket = %ld\n", client_sockfd);
            close_erase(client_sockfd);
            sem_post(&semaphore);
            pthread_exit(NULL);
        }

        //server receive message from client, and send it to all clients (except client_sockfd)
        for(int client : client_sockfds) {
            if(client == client_sockfd) continue;
            printf("client_sockfd to sent: %d\n", client);
            send_ret = send(client, buffer, read_ret, 0);
            printf("send_ret = %d\n", send_ret);
            if(send_ret < 0) {
                perror("send to client");
                close_erase(client_sockfd);
                sem_post(&semaphore);
                pthread_exit(NULL);
            }
            printf("send to client success\n");
        }
    }
    sem_post(&semaphore); //client thread finish, increment value of semaphore and other clients can chat
}

//get client socket info (include client IP and port)
void get_client_info(int client_sockfd) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    char client_ip[INET_ADDRSTRLEN];

    int get_peer_name_ret = getpeername(client_sockfd, (struct sockaddr*)&client_addr, &addr_len);
    if(get_peer_name_ret < 0) {
        perror("get peer name");
        return;
    }

    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("Client Address (sockfd = %d): IP: %s, Port: %d\n", client_sockfd, client_ip, ntohs(client_addr.sin_port));
}

int main() {

    signal(SIGINT, sig_handler_ctrlC_Z);
    signal(SIGTSTP, sig_handler_ctrlC_Z);
    pthread_mutex_init(&mutex_lock, NULL); //init mutex
    sem_init(&semaphore, 0, MAX_CON); //init semaphore, init value of semaphore is MAX_CON (only 3 people can chat simultaneously in this room)
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket
    printf("sockfd = %d\n", sockfd);

    if(sockfd < 0) {
        perror("create socket");
        exit(EXIT_FAILURE);
    }

    printf("create socket success\n");

    server_address.sin_family = AF_INET;
    convert_ret = inet_pton(AF_INET, MAIN_NIC, &server_ip);

    if(convert_ret < 0) {
        perror("convert IPv4");
        exit(EXIT_FAILURE);
        close(sockfd);
    }

    printf("convert ipv4 success\n");

    server_address.sin_addr.s_addr = server_ip.s_addr;
    server_address.sin_port = htons(PORT);

    bind_ret = bind(sockfd, (struct sockaddr*)&server_address, server_address_len); //bind server address to sockfd

    if(bind_ret < 0) {
        perror("bind sockfd");
        exit(EXIT_FAILURE);
        close(sockfd);
    }

    printf("bind success\n");

    listen_ret = listen(sockfd, MAX_CON); //allow server to listen client's requests

    if(listen_ret < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
        close(sockfd);        
    }

    printf("listen success\n");
    printf("server is listening...\n");

    while(1) {
        pthread_mutex_lock(&mutex_lock); //mutex lock, synchronize threads
        new_socket = accept(sockfd, (struct sockaddr*)&server_address, (socklen_t*)&server_address_len); //accept client's request
        printf("\nnew_socket = %d\n", new_socket);
        if(new_socket < 0) {
            perror("accept client");
            continue;
        }

        client_queue.push_back(new_socket); //add new_socket to client_queue. until this moment, client can not chat to others
        printf("accept success\n");
        get_client_info(new_socket);

        //send waiting_message to client
        char waiting_message[] = "waiting for server..";
        int send_ret = send(new_socket, waiting_message, strlen(waiting_message), 0);

        printf("send_ret (waiting message) = %s\n", waiting_message);
        if(send_ret < 0) {
            perror("send_ret (waiting message)");
            close_erase(new_socket);
            continue;
        }

        //create a thread for solve_client() function
        pthread_creat_ret = pthread_create(&threads[new_socket], NULL, solve_client, (void*)new_socket);
        printf("pthread_creat_ret = %d\n", pthread_creat_ret);

        if(pthread_creat_ret < 0) {
            perror("create thread");
            close_erase(new_socket);
            continue;
        }
        pthread_mutex_unlock(&mutex_lock); // mutex unlock
    }
    for(int client : client_sockfds) {
        pthread_join(threads[client], NULL);
    }
    pthread_mutex_destroy(&mutex_lock);
    sem_destroy(&semaphore);
    close(sockfd);
    return 0;
}