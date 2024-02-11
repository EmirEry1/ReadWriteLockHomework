#include <stdio.h>

#include <stdlib.h>

#include <pthread.h> 

#include <semaphore.h>



//My submission includes a make file. You can run "make all" command and the code will compile automatically.



pthread_t* thread_handles;

int thread_count = 4;

struct my_rwlock_t {

    int num_readers;

    char lock_type;

    pthread_mutex_t* writer_mutex;

    pthread_mutex_t* reader_mutex;

    pthread_cond_t* can_write;

};



struct my_rwlock_t* lock;  

void *Thread_Work(void* rank);



int my_wrlock(struct my_rwlock_t* lock){

    pthread_mutex_lock(lock->writer_mutex);

    while(lock->lock_type != 'n'){

        pthread_cond_wait(lock->can_write ,lock->writer_mutex);

    }

    printf("Acquired Mutex For Write Lock\n");

    lock->lock_type = 'w';

    printf("Locked. New Lock type: %c \n", lock->lock_type);

    pthread_mutex_unlock(lock->writer_mutex);

    return 1;

}



int my_rdlock(struct my_rwlock_t* lock)

{

    pthread_mutex_lock(lock->reader_mutex);

    pthread_mutex_lock(lock->writer_mutex);

    printf("Acquired Mutex For Read Lock\n");

    while(lock->lock_type == 'w'){

        pthread_cond_wait(lock->can_write, lock->writer_mutex);

    }

    

    lock->num_readers++;

    lock->lock_type = 'r';

    

    printf("Locked. New Lock type: %c \n", lock->lock_type);

    pthread_mutex_unlock(lock->writer_mutex);

    pthread_mutex_unlock(lock->reader_mutex);

    printf("Unlocked Mutex\n");

    return 1;

}



int my_unlock(struct my_rwlock_t* lock){

    pthread_mutex_lock(lock->writer_mutex);

    

    if (lock->lock_type == 'w'){

        printf("Acquired Mutex To Unlock Write Lock\n");

        lock->lock_type = 'n';

        pthread_cond_broadcast(lock->can_write);

        printf("Unlocked. New Lock type: %c \n", lock->lock_type);

    }

    else if(lock->lock_type == 'r'){

        printf("Acquired Mutex To Unlock Read Lock\n");

        lock->num_readers --;

        if(lock->num_readers == 0){

            lock->lock_type = 'n';

            pthread_cond_broadcast(lock->can_write);

        }

        printf("Unlocked. New Lock type: %c \n", lock->lock_type);

    }

    pthread_mutex_unlock(lock->writer_mutex);

    printf("Unlocked Mutex\n");

    return 1;

}



void *Thread_Work(void* rank){

    int my_rank = (int) rank;

    for(int i = 0; i < 100; i++){

        printf("I am thread %d and I am getting a read lock.\n", my_rank);

        my_rdlock(lock);

        sleep(1);

        printf("I am thread %d and I am unlocking the read lock.\n", my_rank);

        my_unlock(lock);

        printf("I am thread %d and I am getting a write lock.\n", my_rank);

        my_wrlock(lock);

        printf("I am thread %d and I am unlocking the write lock.\n", my_rank);

        my_unlock(lock);

        printf("I am thread %d and I am getting a read lock.\n", my_rank);

        my_rdlock(lock);

        printf("I am thread %d and I am unlocking the read lock.\n", my_rank);

        my_unlock(lock);

    }



};



int main(int argc, char* argv[]){

    lock = (struct my_rwlock_t*) malloc(sizeof(struct my_rwlock_t));

    lock->writer_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

    lock->reader_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

    lock->num_readers = 0;

    lock->lock_type = 'n';

    lock->can_write = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));

    pthread_mutex_init((lock->writer_mutex), NULL);

    pthread_mutex_init((lock->reader_mutex), NULL);

    pthread_cond_init(lock->can_write, NULL);

    thread_handles = (pthread_t*) malloc(sizeof(pthread_t)*thread_count);

    for(int thread = 0; thread < thread_count; thread++){

        pthread_create(&thread_handles[thread], NULL, Thread_Work, (void*) thread);

    }

    for(int thread = 0; thread < thread_count; thread++){

        pthread_join(thread_handles[thread], NULL);

    }



    free(thread_handles);

    free(lock);

    return 0;

}

