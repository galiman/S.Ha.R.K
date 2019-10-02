#ifndef _I386_SEMAPHORE_H
#define _I386_SEMAPHORE_H

#include <linux/linkage.h>

#ifdef __KERNEL__

#include <asm/system.h>
#include <asm/atomic.h>
#include <linux/wait.h>
#include <linux/rwsem.h>

typedef int sem_t;  /* sem.h */
int sem_wait(sem_t *sem);
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_post(sem_t *sem);
int sem_trywait(sem_t *s);

struct semaphore {
	int init;
	int start_value;
	sem_t shark_sem;
};

#define DECLARE_MUTEX(name) struct semaphore name = { .init=0, .start_value=1}
#define DECLARE_MUTEX_LOCKED(name) struct semaphore name= { .init=0, .start_value=0}

static inline void down(struct semaphore * sem)
{
/*        if (sem->init == 0)
	{
		sem_init(&sem->shark_sem, 0, sem->start_value);
		sem->init = 1;
	}
        sem_wait(&sem->shark_sem);
*/
}

static inline void up(struct semaphore * sem)
{
/*
        sem_post(&sem->shark_sem);
*/
}

static inline void init_MUTEX (struct semaphore *sem)
{
/*
        sem_init(&sem->shark_sem, 0, 1);
*/
} 

static inline void init_MUTEX_LOCKED (struct semaphore *sem) 
{	
/*
        sem_init(&sem->shark_sem, 0, 0);
*/
}

static inline int down_trylock(struct semaphore * sem)
{	
        return 0;
/*
        return ( sem_trywait(&sem->shark_sem) );
*/
}

#endif
#endif
