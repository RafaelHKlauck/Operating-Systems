/*
  This code is going to create three threads. The first is going to print AAAAA,
  the second is going to print BBBBB, and the third is going to print CCCCC.
  It should always print in the correct order: AAAAABBBBBCCCCC.
  Using sleep() to make sure the threads print in the correct order.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void *print_A(void *arg);
void *print_B(void *arg);
void *print_C(void *arg);

int main()
{
  pthread_t thread1, thread2, thread3;
  pthread_create(&thread1, NULL, print_A, NULL);
  pthread_create(&thread2, NULL, print_B, NULL);
  pthread_create(&thread3, NULL, print_C, NULL);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_join(thread3, NULL);
  printf("\nFinishing!\n");
  exit(EXIT_SUCCESS);
}

void *print_A(void *arg)
{
  printf("AAAAA");
  pthread_exit(0);
}
void *print_B(void *arg)
{
  printf("BBBBB");
  sleep(2);
  pthread_exit(0);
}
void *print_C(void *arg)
{
  printf("CCCCC");
  pthread_exit(0);
}