/*
gcc -o aggsum aggsum.c -lpthread
./aggsum 10000 4 12345
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Shared sum buffer
long sumbuf = 0;
pthread_mutex_t mutex;  

// Structure for range
struct range {
    int start;
    int end;
};

// Global array pointer
int* shrdarrbuf;

// Function to generate random array data
void generate_array_data(int* buf, int arraysize, int seednum) {
    srand(seednum);  
    for (int i = 0; i < arraysize; i++) {
        buf[i] = rand() % 100;  
    }
}

// Worker function for sum computation
void* sum_worker(void* arg) {
    struct range* idx_range = (struct range*)arg;
    long local_sum = 0;

    // Compute sum in the assigned range
    for (int i = idx_range->start; i <= idx_range->end; i++) {
        local_sum += shrdarrbuf[i];
    }

    // Lock mutex before updating global sum
    pthread_mutex_lock(&mutex);
    sumbuf += local_sum;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        printf("Usage: %s <arrsz> <tnum> [seednum]\n", argv[0]);
        return 1;
    }

    // Parse arguments
    int arrsz = atoi(argv[1]);
    int tnum = atoi(argv[2]);
    int seednum = (argc == 4) ? atoi(argv[3]) : time(NULL);  

    if (arrsz < tnum || tnum <= 0 || arrsz <= 0) {
        printf("Invalid arguments: array size must be >= thread count, both > 0\n");
        return 1;
    }

    // Allocate array
    shrdarrbuf = (int*)malloc(arrsz * sizeof(int));
    if (!shrdarrbuf) {
        perror("Memory allocation failed");
        return 1;
    }

    // Generate random array data
    generate_array_data(shrdarrbuf, arrsz, seednum);

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    // Create threads
    pthread_t threads[tnum];
    struct range thread_ranges[tnum];
    int chunk_size = arrsz / tnum;
    int remainder = arrsz % tnum;

    int start = 0;
    for (int i = 0; i < tnum; i++) {
        thread_ranges[i].start = start;
        thread_ranges[i].end = start + chunk_size - 1;
        if (remainder > 0) {
            thread_ranges[i].end++;
            remainder--;
        }
        start = thread_ranges[i].end + 1;

        pthread_create(&threads[i], NULL, sum_worker, &thread_ranges[i]);
    }

    // Join threads
    for (int i = 0; i < tnum; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destroy mutex and free memory
    pthread_mutex_destroy(&mutex);
    free(shrdarrbuf);

    // Print results
    printf("Parallel sum result: %ld\n", sumbuf);

    return 0;
}
