/*
  This code is to implement processes that are going to do a task together.
  It was given to us a code that calculates all the prime numbers
  We need to split the task in different processes and make them work together
  It should also have an option to do it with threads
  We also need to check the time that the processes take to finish the task
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>

// Function declarations
int check_is_prime_number(long int);
void check_prime_numbers(int begin, int end, int should_print);
void concurrent_prime_numbers(long int number, int should_print);
void thread_prime_numbers(long int number, int should_print);
void *thread_prime_numbers_function(void *arg);
void serial_prime_numbers(long int number, int should_print);

int main(int argc, char *argv[])
{
  long int number = 0;
  short int result, should_print, method;

  if (argc != 4)
  {
    printf("Incorrect: %s <number> <print=1,no_print=0> <threads=2 fork=1, serial=0>\n\n", argv[0]);
    return 1;
  }

  number = atol(argv[1]);
  should_print = atoi(argv[2]);
  method = atoi(argv[3]);

  if (method == 2)
    thread_prime_numbers(number, should_print);
  else if (method == 1)
    concurrent_prime_numbers(number, should_print);
  else
    serial_prime_numbers(number, should_print);
  return 0;
}

void thread_prime_numbers(long int number, int should_print)
{
  int number_of_threads;
  struct timespec start_time, end_time;

  printf("Number of threads: ");
  scanf("%d", &number_of_threads);
  while (number_of_threads <= 1)
  {
    printf("Please, provide valid number of threads: ");
    scanf("%d", &number_of_threads);
  }

  pthread_t threads[number_of_threads];
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  char args[number_of_threads][100];

  for (int i = 0; i < number_of_threads; i++)
  {
    int begin = i * (number / number_of_threads) + 1;
    int end = (i + 1) * (number / number_of_threads);
    printf("Thread %d: %d to %d\n", i, begin, end);

    sprintf(args[i], "%d-%d-%d", begin, end, should_print);

    pthread_create(&(threads[i]), NULL, thread_prime_numbers_function, (void *)args[i]);
  }
  for (int i = 0; i < number_of_threads; i++)
  {
    printf("waiting: %d\n", i);
    pthread_join(threads[i], NULL);
    printf("done: %d\n", i);
  }

  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

  printf("Time spent threads: %f\n", elapsed_time);
}

void *thread_prime_numbers_function(void *arg)
{
  char *args = (char *)arg;
  char *begin_str = strtok(args, "-");
  char *end_str = strtok(NULL, "-");
  char *should_print_str = strtok(NULL, "-");

  long int begin = atoi(begin_str);
  long int end = atoi(end_str);
  int should_print = atoi(should_print_str);

  check_prime_numbers(begin, end, should_print);
  pthread_exit(0);
}

void concurrent_prime_numbers(long int number, int should_print)
{
  int number_of_processes;
  struct timespec start_time, end_time;

  printf("Number of processes: ");
  scanf("%d", &number_of_processes);
  while (number_of_processes <= 1)
  {
    printf("Please, provide valid number of processes: ");
    scanf("%d", &number_of_processes);
  }

  clock_gettime(CLOCK_MONOTONIC, &start_time);

  for (int i = 0; i < number_of_processes; i++)
  {
    pid_t fork_result;
    fork_result = fork();
    if (fork_result == 0)
    {
      int begin = i * (number / number_of_processes) + 1;
      int end = (i + 1) * (number / number_of_processes);
      printf("Process %d: %d to %d\n", i, begin, end);
      check_prime_numbers(begin, end, should_print);
      _Exit(0);
    }
  }

  for (int i = 0; i < number_of_processes; i++)
  {
    wait(NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

  printf("Time spent concurrent: %f\n", elapsed_time);
}

void serial_prime_numbers(long int number, int should_print)
{
  struct timespec start_time, end_time;

  clock_gettime(CLOCK_MONOTONIC, &start_time);

  check_prime_numbers(1, number, should_print);

  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
  printf("Time spent serial: %f\n", elapsed_time);
}

void check_prime_numbers(int begin, int end, int should_print)
{
  short int result;
  for (long int num_int = begin; num_int <= end; num_int++)
  {
    result = check_is_prime_number(num_int);
    if (should_print == 1 && result == 1)
      printf("%ld eh primo.\n", num_int);
  }
}

int check_is_prime_number(long int number)
{
  long int ant;
  for (ant = 2; ant <= (long int)sqrt(number); ant++)
  {
    if (number % ant == 0)
      return 0;
  }
  if (ant * ant >= number)
    return 1;
}
