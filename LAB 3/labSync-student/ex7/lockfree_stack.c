/*
gcc -o lockfree_stack lockfree_stack.c -lpthread
./lockfree_stack
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>  

// Node của stack
typedef struct Node {
    int value;
    struct Node *next;
} Node;

// Lock-Free Stack
typedef struct {
    atomic_intptr_t head; 
} LockFreeStack;

// Hàm push vào stack (lock-free)
bool push(LockFreeStack *stack, int value) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) return false;
    new_node->value = value;

    Node *old_head;
    do {
        old_head = (Node *)atomic_load_explicit(&stack->head, memory_order_relaxed);
        new_node->next = old_head; 
    } while (!atomic_compare_exchange_weak_explicit(
        &stack->head, (intptr_t *)&old_head, (intptr_t)new_node,
        memory_order_release, memory_order_relaxed));

    return true;
}

// Hàm pop từ stack (lock-free)
bool pop(LockFreeStack *stack, int *value) {
    Node *old_head;
    do {
        old_head = (Node *)atomic_load_explicit(&stack->head, memory_order_acquire);
        if (!old_head) return false; 
    } while (!atomic_compare_exchange_weak_explicit(
        &stack->head, (intptr_t *)&old_head, (intptr_t)old_head->next,
        memory_order_release, memory_order_relaxed));

    *value = old_head->value;
    free(old_head);
    return true;
}

// Hàm kiểm tra stack có rỗng không
bool is_empty(LockFreeStack *stack) {
    return atomic_load_explicit(&stack->head, memory_order_acquire) == (intptr_t)NULL;
}

// Test đa luồng (Push và Pop đồng thời)
#define NUM_THREADS 4
#define NUM_OPERATIONS 5

void *thread_func(void *arg) {
    LockFreeStack *stack = (LockFreeStack *)arg;

    for (int i = 0; i < NUM_OPERATIONS; i++) {
        int value = rand() % 100;
        push(stack, value);
        printf("[+] Thread %lu: Push %d\n", pthread_self(), value);
        sleep(3);

        int popped_value;
        if (pop(stack, &popped_value)) {
            printf("[-] Thread %lu: Pop %d\n", pthread_self(), popped_value);
        }
    }
    return NULL;
}

int main() {
    LockFreeStack stack;
    atomic_store_explicit(&stack.head, (intptr_t)NULL, memory_order_relaxed);

    pthread_t threads[NUM_THREADS];

    // Tạo nhiều thread để push/pop cùng lúc
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_func, &stack);
    }
    // Chờ tất cả thread hoàn thành
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("✅ Tất cả các thread đã hoàn thành!\n");
    return 0;
}
