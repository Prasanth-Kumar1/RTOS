#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

struct JoinRequest {
    char groupName[20];
    char name[20];
};
struct JoinResponse {
    int id;
    int groupId;
};
struct Message {
    int id;
    int groupId;
    char name[20];
    char message[200];
};

struct sockaddr_in serv;  
int fd;                   
int conn;                 
struct Message message;
void *send_func(void *n) {
    int sd = *(int *)n;
    while (1) {
        printf("Enter message: ");
        fgets(message.message, sizeof(message.message), stdin);
        write(sd, &message, sizeof(message));
    }

    return 0;
}

void *recv_func(void *n) {
    int rsd = *(int *)n;
    struct Message message;

    while (read(rsd, &message, sizeof(message))) {
      //  printf("%s: %s \n", message.name, message.message);
    }

    return 0;
}
void signal_handler(int n) {
    char ans;
    exit(0);

    if (n == SIGINT) {
        printf("exit Y/N\n");
        scanf("%c", &ans);
        if (ans == 'y' || ans == 'Y') {
            printf("client exited\n");

            exit(0);
        }
    }
}

int main(int argc, char const *argv[]) {
    signal(SIGINT, signal_handler);  
    pthread_t thread;
    const char *clientname = argv[4];  
    printf("%s\n", clientname);
    const char *clientgroup =
        argv[5]; 
    printf("%s\n", clientgroup);
    struct JoinRequest request;
    strcpy(request.name, clientname);
    strcpy(request.groupName, clientgroup);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    printf("socket created\n");
    serv.sin_family = AF_INET;
    int port_no = atoi(argv[3]);
    printf("%d\n", port_no);
    serv.sin_port = htons(port_no);
    printf("%s\n", argv[2]);
    if (inet_pton(AF_INET, argv[2], &serv.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    write(fd, &request, sizeof(request));
    struct JoinResponse response;
    read(fd, &response, sizeof(response));

    int connectionId = response.id;
    int group_Id = response.groupId;
    printf("%d %d\n", connectionId, group_Id);
    if (pthread_create(&thread, NULL, recv_func, (void *)&fd) < 0) {
        perror("pthread_create()");
        exit(EXIT_FAILURE);
    }
    message.id = connectionId;
    message.groupId = group_Id;
    strcpy(message.name, clientname);

    pthread_create(&thread, NULL, send_func, (void *)&fd);
    pthread_join(thread, NULL);

    return 0;
}
