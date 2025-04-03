#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define PERMS 0644
#define MSG_KEY1 0x123  // Queue cho tiến trình A gửi -> B nhận
#define MSG_KEY2 0x456  // Queue cho tiến trình B gửi -> A nhận

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int send_queue, recv_queue;
char role;

void *send_message(void *arg) {
    struct my_msgbuf buf;
    buf.mtype = 1;
    
    printf("[%c] Ready to send messages. Type 'exit' to quit.\n", role);
    
    while (1) {
        fgets(buf.mtext, sizeof(buf.mtext), stdin);
        buf.mtext[strcspn(buf.mtext, "\n")] = '\0';  // Xóa ký tự xuống dòng

        if (msgsnd(send_queue, &buf, strlen(buf.mtext) + 1, 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        
        if (strcmp(buf.mtext, "exit") == 0) {
            break;
        }
    }
    
    pthread_exit(NULL);
}

void *receive_message(void *arg) {
    struct my_msgbuf buf;
    
    printf("[%c] Ready to receive messages.\n", role);
    
    while (1) {
        if (msgrcv(recv_queue, &buf, sizeof(buf.mtext), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        printf("[%c] Received: %s\n", role, buf.mtext);
        
        if (strcmp(buf.mtext, "exit") == 0) {
            break;
        }
    }
    
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s A|B\n", argv[0]);
        exit(1);
    }
    
    role = argv[1][0];

    if (role == 'A') {
        send_queue = msgget(MSG_KEY1, PERMS | IPC_CREAT);
        recv_queue = msgget(MSG_KEY2, PERMS | IPC_CREAT);
    } else if (role == 'B') {
        send_queue = msgget(MSG_KEY2, PERMS | IPC_CREAT);
        recv_queue = msgget(MSG_KEY1, PERMS | IPC_CREAT);
    } else {
        fprintf(stderr, "Invalid role. Use 'A' or 'B'.\n");
        exit(1);
    }

    if (send_queue == -1 || recv_queue == -1) {
        perror("msgget");
        exit(1);
    }

    pthread_t send_thread, recv_thread;
    
    pthread_create(&send_thread, NULL, send_message, NULL);
    pthread_create(&recv_thread, NULL, receive_message, NULL);
    
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    if (role == 'A') {
        msgctl(send_queue, IPC_RMID, NULL);
        msgctl(recv_queue, IPC_RMID, NULL);
    }

    return 0;
}
