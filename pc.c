#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "eventbuf.h"

sem_t* p_sem;
sem_t* c_sem;
sem_t* buf_sem;
struct eventbuf* buffer;

typedef struct producer_arg
{
    int id;
    int event_count;
} producer_arg;

typedef struct consumer_arg
{
    int id;
} consumer_arg;

sem_t *sem_open_temp(const char *name, int value)
{
    sem_t *sem;

    // Create the semaphore
    if ((sem = sem_open(name, O_CREAT, 0600, value)) == SEM_FAILED)
        return SEM_FAILED;

    // Unlink it so it will go away after this process exits
    if (sem_unlink(name) == -1) {
        sem_close(sem);
        return SEM_FAILED;
    }

    return sem;
}

producer_arg* create_producers_args(int count, int event_count)
{
    producer_arg* args = malloc(sizeof(producer_arg) * count);
    for (int i = 0; i < count; i++)
    {
        args[i].id = i;
        args[i].event_count = event_count;
    }

    return args;
}

consumer_arg* create_consumer_args(int count)
{
    consumer_arg* args = malloc(sizeof(consumer_arg) * count);
    for (int i = 0; i < count; i++)
    {
        args[i].id = i;
    }

    return args;
}

void* produce(void* arg)
{
    int id = ((producer_arg*)arg)->id;
    int event_count = ((producer_arg*)arg)->event_count;


    for (int i = 0; i < event_count; i++)
    {
        // Wait for consumers to signal.
        sem_wait(p_sem);
        // Lock buffer.
        sem_wait(buf_sem);

        int value = id * 100 + i;
        printf("P%d: adding event %d\n", id, value);
        eventbuf_add(buffer, value);

        // Unlock buffer.
        sem_post(buf_sem);
        // And signal to consumers.
        sem_post(c_sem);
    }

    printf("P%d: exiting\n", id);

    return NULL;
}

void* consume(void* arg)
{
    int id = ((consumer_arg*)arg)->id;
    
    // Infinite loop to consume until producers are finished.
    while(1)
    {
        // Wait for producers to signal
        sem_wait(c_sem);
        // Lock buffer.
        sem_wait(buf_sem);

        // End consuming if buffer is empty.
        if (eventbuf_empty(buffer))
        {
            sem_post(buf_sem);
            printf("C%d: exiting\n", id);

            return NULL;
        }

        int value = eventbuf_get(buffer);
        printf("C%d: got event %d\n", id, value);

        // Unlock buffer.
        sem_post(buf_sem);
        // And signal to producers.
        sem_post(p_sem);
    }
}

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        printf("Usage: %s <producer count> <consumer count> <event count> <max events>\n", argv[0]);
        exit(1);
    }

    int producer_count = atoi(argv[1]);
    int consumer_count = atoi(argv[2]);
    int event_count = atoi(argv[3]);
    int max_events = atoi(argv[4]);

    // p_sem is initialized with max_events since the queue starts empty.
    // c_sem is initialized with 0 because there is nothing to consume yet.
    // buf_sem is initialized with a 1 to act as a mutex for the buffer.
    p_sem = sem_open_temp("pc-project-p-sem", max_events);
    c_sem = sem_open_temp("pc-project-c-sem", 0);
    buf_sem = sem_open_temp("pc-project-buf_sem", 1);

    buffer = eventbuf_create();

    pthread_t* producers = malloc(sizeof(pthread_t) * producer_count);
    producer_arg* producer_args = create_producers_args(producer_count, event_count);

    pthread_t* consumers = malloc(sizeof(pthread_t) * consumer_count);
    consumer_arg* consumer_args = create_consumer_args(consumer_count);

    // Launch producers.
    for (int i = 0; i < producer_count; i++)
    {
        pthread_create(producers + i, NULL, produce, producer_args + i);
    }

    // Launch consumers.
    for (int i = 0; i < consumer_count; i++)
    {
        pthread_create(consumers + i, NULL, consume, consumer_args + i);
    }

    // Wait for producers.
    for (int i = 0; i < producer_count; i++)
    {
        pthread_join(producers[i], NULL);
    }

    // Wake all consumers.
    for (int i = 0; i < consumer_count; i++)
    {
        sem_post(c_sem);
    }

    // Wait for consumers.
    for (int i = 0; i < consumer_count; i++)
    {
        pthread_join(consumers[i], NULL);
    }

    free(producers);
    free(producer_args);
    free(consumers);
    free(consumer_args);

    eventbuf_free(buffer);

    sem_close(p_sem);
    sem_close(c_sem);
    sem_close(buf_sem);
}
