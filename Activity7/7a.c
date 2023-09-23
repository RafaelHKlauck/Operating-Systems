/*
  This code is going to create a thread(thread_C) that is going to check the temperature of a farm(random number 0-40)
  every second. Besides that, it is going to create another thread (thread_P) that is going to receive the temperature
  and make an average of the last 10 temperatures. Another thread(thread_M) is going to receive the average temperature
  and check if the average is below a defined number and active irrigation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MIN_TEMP 18
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t temperature_average_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t counter_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t temperature_average_condition = PTHREAD_COND_INITIALIZER;

int temperature_total = 0;
int counter = 0;
int average = 0;
int seconds = 0;
int irrigation_min_temp = 0;

void *thread_C(void *arg)
{
  while (1)
  {
    int temperature = rand() % 40;
    pthread_mutex_lock(&counter_mutex);
    counter++;
    temperature_total += temperature;
    if (counter == 10)
    {
      pthread_cond_signal(&counter_condition);
    }
    pthread_mutex_unlock(&counter_mutex);
    sleep(1);
  }
};

void *thread_P(void *arg)
{
  while (1)
  {
    pthread_mutex_lock(&counter_mutex);

    while (counter < 10)
    {
      pthread_cond_wait(&counter_condition, &counter_mutex);
    }

    average = temperature_total / 10;
    seconds += 10;
    counter = 0;           // reset counter
    temperature_total = 0; // reset temperature_total

    pthread_mutex_unlock(&counter_mutex);

    if (average != 0)
    {
      pthread_mutex_lock(&temperature_average_mutex);
      pthread_cond_signal(&temperature_average_condition);
      pthread_mutex_unlock(&temperature_average_mutex);
    }
    printf("Average %ds: %.2d\n", seconds, average);
  }
};

void *thread_M(void *arg)
{
  while (1)
  {
    pthread_mutex_lock(&temperature_average_mutex);
    while (average == 0)
    {
      pthread_cond_wait(&temperature_average_condition, &temperature_average_mutex);
    }
    if (average < irrigation_min_temp)
      printf("IRRIGATION ACTIVED!!\n");
    average = 0;
    pthread_mutex_unlock(&temperature_average_mutex);
  }
};

int main()
{
  printf("Please, insert the minimum temperature for irrigation: ");
  scanf("%d", &irrigation_min_temp);
  pthread_t threads[3];
  srand(time(NULL));

  pthread_create(&threads[0], NULL, thread_C, NULL);
  pthread_create(&threads[1], NULL, thread_P, NULL);
  pthread_create(&threads[2], NULL, thread_M, NULL);

  pthread_exit(NULL);
  return 0;
}
