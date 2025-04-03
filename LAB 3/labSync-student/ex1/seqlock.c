/*
gcc -o seqlock seqlock.c -lpthread
./seqlock
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    volatile unsigned int sequence;     
    pthread_mutex_t writer_lock;          
    int data;                           
} pthread_seqlock_t;

// Create sequence lock
int pthread_seq_lock_init(pthread_seqlock_t* s) {
    s->sequence = 0;
    s->data = 0;
    return pthread_mutex_init(&s->writer_lock, NULL);
}

int pthread_seq_lock_destroy(pthread_seqlock_t* s) {
    return pthread_mutex_destroy(&s->writer_lock);
}

// Writer lock & unlock
void pthread_seq_lock_wrlock(pthread_seqlock_t* s) {
    pthread_mutex_lock(&s->writer_lock);
    s->sequence++; // move to odd number
}

void pthread_seq_lock_wrunlock(pthread_seqlock_t* s) {
    s->sequence++; // move to even number
    pthread_mutex_unlock(&s->writer_lock);
}

// Reader helper
unsigned int pthread_seq_lock_begin(pthread_seqlock_t* s) {
    unsigned int seq;
    do {
        seq = s->sequence;
    } while (seq & 1);  
    return seq;
}

// Reader helper: check after read
int pthread_seq_lock_validate(pthread_seqlock_t* s, unsigned int seq) {
    return (seq == s->sequence);
}

// sequence lock
pthread_seqlock_t seqlock;

void* writer_thread(void* arg) {
    for (int i = 1; i <= 10; i++) {
        pthread_seq_lock_wrlock(&seqlock);
        seqlock.data = i;  // write new data
        printf("Writer: updated data to %d\n", i);
        pthread_seq_lock_wrunlock(&seqlock);
        sleep(1);  
    }
    return NULL;
}

void* reader_thread(void* arg) {
    int local_data;
    for (int i = 0; i < 10; i++) {
        unsigned int seq;
        int valid;
        do {
            seq = pthread_seq_lock_begin(&seqlock);
            local_data = seqlock.data;  // data reader
            
            valid = pthread_seq_lock_validate(&seqlock, seq);
            if (!valid) {
                printf("Reader: inconsistent read, retrying...\n");
            }
        } while (!valid);
        printf("Reader: read data %d\n", local_data);
        usleep(1025000); 
    }
    return NULL;
}

int main() {
    pthread_t writer, reader;
    pthread_seq_lock_init(&seqlock);

    pthread_create(&writer, NULL, writer_thread, NULL);
    pthread_create(&reader, NULL, reader_thread, NULL);

    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    pthread_seq_lock_destroy(&seqlock);
    return 0;
}
