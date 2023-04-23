#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
// status: 0 - cutter free; 1 - cuttting
// buf: conatins ids of new client
// when cutter is cutting he will close sem
// when free, open it
// first client to access open sem will change status of 
// cutter to cutting and close sem

#define CHILDREN_MAX 16

static volatile int keepRunning = 1;

typedef struct shared_memory {
  sem_t mutex;
  double arr[8];
}shared_memory;


void intHandler(int dummy) {
    printf("[Cutter] SIGINT Detected!\n");
    keepRunning = 0;
}


void child(shared_memory* shmptr, double part, int sum, int i)  {
  printf("I am a client #%d!\n", getpid());
  double value = sum * part;
  shmptr->arr[i] = value;
  sem_post(&shmptr->mutex);
  exit(0);
}



int main(int argc, char ** argv) {
  if (argc < 4) {
    printf("usage: ./main <sum parts>");
    return -1;
  }
  double part;
  //char sem_name[] = "sem-mutex";
  part = atof(argv[2]);
  signal(SIGINT, intHandler);
  int sum = atoi(argv[1]);
  int index = atoi(argv[3]);
  int children[8];
  char memn[] = "shared-memory"; //  имя объекта
  int mem_size = sizeof(shared_memory);
  int shm;
  //sem_t *mutex;

  // СОздать память
  if ((shm = shm_open(memn, O_CREAT | O_RDWR, 0666)) == -1) {
      printf("Object is already open\n");
      perror("shm_open");
      return 1;
  } else {
      printf("Object is open: name = %s, id = 0x%x\n", memn, shm);
  }
  if (ftruncate(shm, mem_size) == -1) {
      printf("Memory sizing error\n");
      perror("ftruncate");
      return 1;
  } else {
      printf("Memory size set and = %d\n", mem_size);
  }

  //получить доступ к памяти
  void* addr = mmap(0, mem_size, PROT_WRITE, MAP_SHARED, shm, 0);
  if (addr == (int * ) - 1) {
      printf("Error getting pointer to shared memory\n");
      return 1;
  }

  shared_memory* shmem = addr;
  // create child processes
  
  child(shmem, part, sum, index);

  double counter_sum;
  
  //printf("%lf \n %d", counter_sum, sum);
  close(shm);
  // удалить выделенную память
  
  return 0;
}
