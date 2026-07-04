#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define BUF_SIZE 100
#define THRESHOLD 50
#define TICK_MS 200
#define RUN_SECONDS 30

unsigned char buffer[BUF_SIZE];
int in = 0, out = 0, count = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

// Producer thread
void* producer(void* arg) {
    for (int t = 1; t <= RUN_SECONDS; t++) {
        usleep(TICK_MS * 1000);

        int bytes = rand() % 6;

        pthread_mutex_lock(&lock);

        for (int i = 0; i < bytes; i++) {
            while (count == BUF_SIZE) {
                pthread_cond_wait(&not_full, &lock);
            }

            buffer[in] = rand() % 256;
            in = (in + 1) % BUF_SIZE;
            count++;
        }

        printf("t=%2ds | Produced %d bytes | buffer=%d\n", t, bytes, count);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// Consumer thread
void* consumer(void* arg) {
    for (int t = 10; t <= RUN_SECONDS; t += 10) {
        usleep(TICK_MS * 1000 * 10);

        pthread_mutex_lock(&lock);

        if (count >= THRESHOLD) {
            printf("\nt=%2ds | Consuming %d bytes:\n", t, THRESHOLD);

            for (int i = 0; i < THRESHOLD; i++) {
                while (count == 0) {
                    pthread_cond_wait(&not_empty, &lock);
                }

                printf("%02X ", buffer[out]);
                out = (out + 1) % BUF_SIZE;
                count--;
            }
            printf("\n\n");

            pthread_cond_signal(&not_full);
        } else {
            printf("t=%2ds | Not enough data (%d bytes)\n", t, count);
        }

        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t p, c;

    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);

    pthread_join(p, NULL);
    pthread_join(c, NULL);

    return 0;
}