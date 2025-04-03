#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_MOVIES 2000  // Đảm bảo chứa đủ 1682 phim
#define FILE1 "movie-100k_1.txt"
#define FILE2 "movie-100k_2.txt"

typedef struct {
    double sum[MAX_MOVIES];  // Tổng điểm rating
    int count[MAX_MOVIES];   // Số lượng rating
} SharedData;

// Hàm đọc file và cập nhật vào shared memory
void compute_average(const char *filename, SharedData *shm_data) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int user_id, movie_id, rating, timestamp;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d %d %d %d", &user_id, &movie_id, &rating, &timestamp) == 4) {
            if (movie_id >= 1 && movie_id < MAX_MOVIES) { // Tránh truy cập ngoài mảng
                shm_data->sum[movie_id] += rating;
                shm_data->count[movie_id] += 1;
            }
        }
    }
    fclose(file);
}

int main() {
    int shmid;
    SharedData *shm_data;

    // Tạo shared memory
    shmid = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // Gắn shared memory vào tiến trình cha
    shm_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shm_data == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Khởi tạo shared memory
    for (int i = 0; i < MAX_MOVIES; i++) {
        shm_data->sum[i] = 0.0;
        shm_data->count[i] = 0;
    }

    // Tạo tiến trình con để xử lý FILE1
    pid_t pid1 = fork();
    if (pid1 == 0) {
        compute_average(FILE1, shm_data);
        exit(EXIT_SUCCESS);
    }

    // Tạo tiến trình con để xử lý FILE2
    pid_t pid2 = fork();
    if (pid2 == 0) {
        compute_average(FILE2, shm_data);
        exit(EXIT_SUCCESS);
    }

    // Đợi cả hai tiến trình con kết thúc
    wait(NULL);
    wait(NULL);

    // In kết quả
    printf("Movie Ratings from Shared Memory:\n");
    for (int i = 1; i <= 1682; i++) {
        if (shm_data->count[i] > 0) {
            double avg = shm_data->sum[i] / shm_data->count[i];
            printf("Movie %d - Average Rating: %.2f\n", i, avg);
        }
    }

    // Giải phóng shared memory
    shmdt(shm_data);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
