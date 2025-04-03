/*
make
./mypool
*/
#include "bktpool.h"
#include <signal.h>
#include <stdio.h>

#define _GNU_SOURCE
#include <linux/sched.h>
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>

//#define DEBUG
#define INFO
#define WORK_THREAD

void * bkwrk_worker(void * arg) {
  sigset_t set;
  int sig;
  int s;
  int i = * ((int * ) arg); // Default arg is integer of workid
  struct bkworker_t * wrk = & worker[i];

  /* Taking the mask for waking up */
  sigemptyset( & set);
  sigaddset( & set, SIGUSR1);
  sigaddset( & set, SIGQUIT);

#ifdef DEBUG
  fprintf(stderr, "worker %i start living tid %d \n", i, getpid());
  fflush(stderr);
#endif

  while (1) {
    /* wait for signal */
    s = sigwait( & set, & sig);
    if (s != 0)
      continue;

#ifdef INFO
    fprintf(stderr, "worker wake %d up\n", i);
#endif

    /* Busy running */
    if (wrk -> func != NULL)
      wrk -> func(wrk -> arg);

    /* Advertise I DONE WORKING */
    wrkid_busy[i] = 0;
    worker[i].func = NULL;
    worker[i].arg = NULL;
    worker[i].bktaskid = -1;
  }
}

int bktask_assign_worker(unsigned int bktaskid, unsigned int wrkid) {
  if (wrkid < 0 || wrkid > MAX_WORKER)
    return -1;

  struct bktask_t * tsk = bktask_get_byid(bktaskid);

  if (tsk == NULL)
    return -1;

  /* Advertise I AM WORKING */
  wrkid_busy[wrkid] = 1;

  worker[wrkid].func = tsk -> func;
  worker[wrkid].arg = tsk -> arg;
  worker[wrkid].bktaskid = bktaskid;

  printf("Assign tsk %d wrk %d \n", tsk -> bktaskid, wrkid);
  return 0;
}

int bkwrk_create_worker() {
  unsigned int i;

  for (i = 0; i < MAX_WORKER; i++) {
#ifdef WORK_THREAD
    void ** child_stack = (void ** ) malloc(STACK_SIZE);
    unsigned int wrkid = i;
    pthread_t threadid;

    sigset_t set;
    int s;

    sigemptyset( & set);
    sigaddset( & set, SIGQUIT);
    sigaddset( & set, SIGUSR1);
    sigprocmask(SIG_BLOCK, & set, NULL);

    /* Stack grow down - start at top*/
    void * stack_top = child_stack + STACK_SIZE;

    wrkid_tid[i] = clone( & bkwrk_worker, stack_top,
      CLONE_VM | CLONE_FILES,
      (void * ) & i);
#ifdef INFO
    fprintf(stderr, "bkwrk_create_worker got worker %u\n", wrkid_tid[i]);
#endif

    usleep(100);

    #else
        pid_t pid = fork();
        if (pid < 0) {
            return -1;
        } else if (pid == 0) {
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGUSR1);
            sigaddset(&set, SIGQUIT);
            sigprocmask(SIG_BLOCK, &set, NULL);

            while (1) {
                int sig;
                if (sigwait(&set, &sig) != 0)
                    continue;
                if (worker[i].func != NULL)
                    worker[i].func(worker[i].arg);
                wrkid_busy[i] = 0;
                worker[i].func = NULL;
                worker[i].arg = NULL;
                worker[i].bktaskid = -1;
            }
            //_exit(0);
        } else {
#ifdef INFO
            fprintf(stderr, "bkwrk_create_worker got worker %u\n", pid);
#endif
            wrkid_tid[i] = pid;
            usleep(100);
        }
#endif

  }

  return 0;
}

int bkwrk_get_worker() {
    for (int i = 0; i < MAX_WORKER; i++) {
        if (wrkid_busy[i] == 0) {
            return i;
        }
    }
    return -1;
}

int bkwrk_dispatch_worker(unsigned int wrkid) {

#ifdef WORK_THREAD
  unsigned int tid = wrkid_tid[wrkid];

  /* Invalid task */
  if (worker[wrkid].func == NULL)
    return -1;

#ifdef DEBUG
  fprintf(stderr, "brkwrk dispatch wrkid %d - send signal %u \n", wrkid, tid);
#endif

  syscall(SYS_tkill, tid, SIG_DISPATCH);
  #else
  /* TODO: Implement fork version to signal worker process here */

#endif
}