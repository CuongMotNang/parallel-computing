// sum_pthread.c - Tính tổng mảng bằng nhiều luồng (minh họa con trỏ trong lập trình song song)
// Biên dịch: gcc sum_pthread.c -o sum -pthread
// Chạy:      ./sum
#include <stdio.h>
#include <pthread.h>

#define N 8
#define T 2

int a[N] = {1, 2, 3, 4, 5, 6, 7, 8};

typedef struct {
    int *start;   // con trỏ tới đầu đoạn mảng
    int len;
    long sum;
} Task;

void *work(void *arg) {
    Task *t = (Task *)arg;
    t->sum = 0;
    for (int i = 0; i < t->len; i++)
        t->sum += *(t->start + i);
    return NULL;
}

int main(void) {
    pthread_t th[T];
    Task tk[T];

    for (int i = 0; i < T; i++) {
        tk[i].start = a + i * (N / T);   // số học con trỏ
        tk[i].len = N / T;
        pthread_create(&th[i], NULL, work, &tk[i]);
    }

    long total = 0;
    for (int i = 0; i < T; i++) {
        pthread_join(th[i], NULL);
        printf("Luong %d: sum = %ld\n", i, tk[i].sum);
        total += tk[i].sum;
    }
    printf("Tong = %ld\n", total);   // 36
    return 0;
}
