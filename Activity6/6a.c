/*
  Create a program to create N threads(beyond the parent). The first thread must create the second thread,
  the second thread must create the third thread, and so on.
  Each thread must print its id and the id of the thread that created it.
  The print order must be from the N thread to the first thread.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int number_of_threads;
int thread_counter = 0;

void *create_thread_function(void *arg);

int main()
{
  printf("Type the number of threads: ");
  scanf("%d", &number_of_threads);
  pthread_t a_thread;

  pthread_create(&a_thread, NULL, create_thread_function, (void *)thread_counter);
  printf("Parent thread id: %lu - ", pthread_self());
  sleep(3); // Avoid ending the program before the child threads finish
  printf("Finishing!\n");
  exit(EXIT_SUCCESS);
}

void *create_thread_function(void *arg)
{
  int thread_counter = (int)arg;
  printf("Child thread id: %lu\n", pthread_self());
  if (thread_counter < number_of_threads)
  {
    pthread_t a_thread;
    pthread_create(&a_thread, NULL, create_thread_function, (void *)++thread_counter);
    printf("Parent thread id: %lu -", pthread_self());
    pthread_join(a_thread, NULL);
  }
  pthread_exit(0);
}