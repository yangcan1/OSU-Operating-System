#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t myCond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t myCond2 = PTHREAD_COND_INITIALIZER;
int myCount = 0;

// Producer function only add 1 into myCount, and wait for consumer.
void* producer() {

    while (1) {

        pthread_mutex_lock(&myMutex); // Lock the mutex before adding 1.
        printf("PRODUCER: myMutex locked\n");

        myCount++;

        printf("PRODUCER: waiting on myCond1\n");
        pthread_cond_wait(&myCond1, &myMutex); // wait for condition variable 1 which will be signalled by consumer. 
    
        if (myCount == 10) {
            break;
        }
        pthread_mutex_unlock(&myMutex); // Unlock the mutex
        printf("PRODUCER: myMutex unlocked\n");

        
        printf("PRODUCER: signaling myCond2\n");
        pthread_cond_signal(&myCond2); // Signal consumer that myCount has been added.
    }

    return NULL;
}

// Consumer function only prints out myCount -> myCount. After that, wait for Producer.
void* consumer() { 
    printf("CONSUMER THREAD CREATED\n");

    pthread_mutex_lock(&myMutex); // Lock the mutex before checking if the buffer has data
    printf("CONSUMER: myMutex locked\n");

    while (1) {
        
        printf("myCount: %d -> %d\n", myCount - 1, myCount);

        pthread_mutex_unlock(&myMutex); // Unlock the mutex
        printf("CONSUMER: myMutex unlocked\n");

        printf("CONSUMER: signaling myCond1\n");
        pthread_cond_signal(&myCond1); // signaling cond1 to producer that count has been printed out.

        if (myCount == 10) {
            break;
        }
        printf("CONSUMER: waiting on myCond2\n");
        pthread_cond_wait(&myCond2, &myMutex); // wait for cond2 that will be signaled by producer.

    }
    
    return NULL;
}

int main() {
    pthread_t pTh, cTh;
    printf("PROGRAM START\n");
    
    // create two thread;
    pthread_create(&pTh, NULL, producer, NULL);
    pthread_create(&cTh, NULL, consumer, NULL);

    // wait for two thread to finish;
    pthread_join(pTh, NULL);
    pthread_join(cTh, NULL);

    printf("PROGRAM END\n"); // Program ends
    return 0;
}
