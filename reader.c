#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>


#define    READER 0
#define    WRITER 1
#define    LOCK 2 
#define    FREEDATA 3

void err_sys(const char* error);


int main(int argc, const char* argv[]) {
    if(argc != 1) {
        printf("INCORRECT INPUT");
        exit(0);
    }

    int semid = semget(0x111, 4, IPC_CREAT | 0666);
    if (semid < 0)
      err_sys("SEMGET ERROR");

    int shmid = shmget(0x111, 4096, IPC_CREAT | 0666);
    if(shmid < 0)
      err_sys("SHMGET ERROR");

    void* shm_ptr = shmat(shmid, NULL, 0);
    if(shm_ptr == (void*) -1)
      err_sys("SHMAT ERROR");
    int stat = 0;


    struct sembuf sop[16];
    int nop = 0;
    do {
      sop[nop].sem_num  = WRITER; 
      sop[nop].sem_op   = -1; 
      sop[nop].sem_flg  = 0; 
      nop++;
    } while(0);
    do {
      sop[nop].sem_num  = WRITER; 
      sop[nop].sem_op   = 0; 
      sop[nop].sem_flg  = 0; 
      nop++;
    } while(0);
    do {
      sop[nop].sem_num  = WRITER; 
      sop[nop].sem_op   = 2; 
      sop[nop].sem_flg  = 0; 
      nop++;
    } while(0);
    do {
      sop[nop].sem_num  = READER; 
      sop[nop].sem_op   = 0; 
      sop[nop].sem_flg  = 0; 
      nop++;
    } while(0);
    do {
      sop[nop].sem_num  = READER; 
      sop[nop].sem_op   = 1; 
      sop[nop].sem_flg  = SEM_UNDO; 
      nop++;
    } while(0);

    stat = 0;
    assert(nop < 16);
    stat = semop(semid, sop, nop);
    nop = 0;
   ////////////////////////////////////////////////

    int32_t read = 0;
    do {
      do {
        sop[nop].sem_num  = WRITER; 
        sop[nop].sem_op   = -2; 
        sop[nop].sem_flg  = IPC_NOWAIT; 
        nop++;
      } while(0);
      do {
        sop[nop].sem_num  = WRITER; 
        sop[nop].sem_op   = 0; 
        sop[nop].sem_flg  = IPC_NOWAIT; 
        nop++;
      } while(0);
      do {
        sop[nop].sem_num  = WRITER; 
        sop[nop].sem_op   = 2; 
        sop[nop].sem_flg  = 0; 
        nop++;
      } while(0);
      do {
        sop[nop].sem_num  = FREEDATA; 
        sop[nop].sem_op   = -1; 
        sop[nop].sem_flg  = 0; 
        nop++;
      } while(0);
      do {
        sop[nop].sem_num  = FREEDATA; 
        sop[nop].sem_op   = 0; 
        sop[nop].sem_flg  = IPC_NOWAIT; 
        nop++;
      } while(0);
      do {
        sop[nop].sem_num  = LOCK; 
        sop[nop].sem_op   = 0; 
        sop[nop].sem_flg  = 0; 
        nop++;
      } while(0);
      do {
        sop[nop].sem_num  = LOCK; 
        sop[nop].sem_op   = 1; 
        sop[nop].sem_flg  = SEM_UNDO; 
        nop++;
      } while(0);

      stat = 0;
      assert(nop < 16);
      stat = semop(semid, sop, nop);
      nop = 0;

      if(stat < 0)
        err_sys("WRITER DIED");
       
       ///////////////////////////////////////////////////
      read = *((int32_t*) shm_ptr);
      if(read < 0)
        err_sys("READ ERROR");

      if(write(STDOUT_FILENO, shm_ptr + sizeof(int32_t), read) < 0)
        err_sys("WRITE ERROR");
        
	do {
          sop[nop].sem_num  = LOCK; 
          sop[nop].sem_op   = -1; 
          sop[nop].sem_flg  = SEM_UNDO; 
          nop++;
        } while(0);
        do {
          sop[nop].sem_num  = LOCK; 
          sop[nop].sem_op   = 0; 
          sop[nop].sem_flg  = IPC_NOWAIT; 
          nop++;
        } while(0);

        stat = 0;
        assert(nop < 16);
        stat = semop(semid, sop, nop);
        nop = 0;
     
        if(stat < 0)
          err_sys("LOCK SEM WAS CHANGED");
    } while(read > 0);
    do {
      sop[nop].sem_num  = READER; 
      sop[nop].sem_op   = -1; 
      sop[nop].sem_flg  = SEM_UNDO; 
      nop++;
    } while(0);
    stat = 0;
    assert(nop < 16);
    stat = semop(semid, sop, nop);
    nop = 0;
   /////////////////////////////////////
       
}

void err_sys(const char* error)
{
	perror(error);
	exit(1);
}
