#include "common.h"

#define MAX_CLIENTS 4
#define MAX_USERNAME_LENGTH 32
#define MAX_MESSAGE_SIZE 64
#define MAX_EVENTS (MAX_CLIENTS + 1)



void usage(char* program_name)
{
    fprintf(stderr, "Usage: \n");

    fprintf(stderr, "\t%s", program_name);
    set_color(2, SOP_PINK);
    fprintf(stderr, " port\n");

    fprintf(stderr, "\t  port");
    reset_color(2);
    fprintf(stderr, " - the port on which the server will run\n");

    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) { 
    
    if (argc != 2){
        usage(argv[0]);
    }
    uint16_t port;
    port = atoi(argv[1]);

    if (port <= 1023 || port >= 65535){
        usage(argv[0]);
    }

    int server_socket = bind_tcp_socket(port, 16);

    int epoll_ds;

    if ((epoll_ds = epoll_create1(0)) < 0)
    {
        ERR("epoll_create:");
    }


    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = server_socket;
    if (epoll_ctl(epoll_ds, EPOLL_CTL_ADD, server_socket, &event) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    int ready_fds =  epoll_wait(epoll_ds, events, MAX_EVENTS, -1);

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGPIPE);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    
    for (int i = 0; i < ready_fds; i++){
        int client_socket = add_new_client(events[i].data.fd);
        int size;

        printf("Client socket: %d\n", client_socket);

        while(1){
            char buf;
            int read_chars;
            read_chars = read(client_socket, &buf, 1);
            if (read_chars == 0){
                break;
            }

            if (read_chars < 0){
                ERR("read");
            }
            printf("%c\n", buf);

            if (buf == '\n'){
                break;
            }

        }


        if (write(client_socket,"Hello world\n", 13) < 0){
            if (errno != EPIPE){
                ERR("write");
            }
        }
        
        if (TEMP_FAILURE_RETRY(close(client_socket)) < 0)
            ERR("close");
    }


    if (TEMP_FAILURE_RETRY(close(server_socket)) < 0)
            ERR("close");



    return EXIT_SUCCESS;

}
