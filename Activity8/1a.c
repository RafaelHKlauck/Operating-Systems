/*
    This code of the Dining Philosophers Problem was provided by the teacher
    and modified by me to show the statistics of the philosophers' dinner.
*/

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>

#define N 5
#define LEFT (i + N - 1) % N
#define RIGHT (i + 1) % N
#define THINKING 0
#define HUNGRY 1
#define EATING 2

int state[N], i, int_rand, tried_to_eat[N], ate[N];
float float_rand;
void mostrar(void);
void pensar(int);
void pegar_garfo(int);
void por_garfo(int);
void comer(int);
void test(int);
void *acao_filosofo(void *);
void statistic();

sem_t mutex;
sem_t sem_fil[N];

int main()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &statistic;
    sigaction(SIGINT, &sa, NULL);
    for (i = 0; i < N; i++)
    {
        state[i] = 0;
        tried_to_eat[i] = 0;
        ate[i] = 0;
    }
    mostrar();
    int res;
    pthread_t thread[N];
    void *thread_result;
    res = sem_init(&mutex, 0, 1);

    if (res != 0)
    {
        perror("Erro na inicialização do semaforo");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++)
    {
        res = sem_init(&sem_fil[i], 0, 0);
        if (res != 0)
        {
            perror("Erro na inicialização do semaforo!");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < N; i++)
    {
        int *filosofo_id = malloc(sizeof(int));
        *filosofo_id = i;
        res = pthread_create(&thread[i], NULL, acao_filosofo, filosofo_id);
        if (res != 0)
        {
            perror("Erro na inicialização das threads!");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < N; i++)
    {

        res = pthread_join(thread[i], &thread_result);
        if (res != 0)
        {
            perror("Erro ao fazer join nas threads!");
            exit(EXIT_FAILURE);
        }
    }
    statistic();
    return 0;
}

void mostrar()
{
    for (i = 1; i <= N; i++)
    {
        if (state[i - 1] == THINKING)
        {
            printf("O filosofo %d esta pensando!\n", i);
        }
        if (state[i - 1] == HUNGRY)
        {
            printf("O filosofo %d esta com fome!\n", i);
        }
        if (state[i - 1] == EATING)
        {
            printf("O filosofo %d esta comendo!\n", i);
        }
    }
    printf("\n");
    usleep(1000000 * 1);
}
void *acao_filosofo(void *j)
{
    int i = *((int *)j);
    free(j);
    while (1)
    {
        pensar(i);
        pegar_garfo(i);
        comer(i);
        por_garfo(i);
    }
}
void pensar(int i)
{
    float_rand = 0.001 * random();
    int_rand = float_rand;
    usleep(int_rand);
}
void pegar_garfo(int i)
{
    sem_wait(&mutex);

    state[i] = HUNGRY;
    mostrar();
    test(i);
    sem_post(&mutex);
    sem_wait(&sem_fil[i]);
}
void test(int i)
{
    tried_to_eat[i] = tried_to_eat[i] + 1;
    if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
    {
        state[i] = EATING;
        mostrar();
        ate[i] = ate[i] + 1;
        sem_post(&sem_fil[i]);
    }
}
void comer(int i)
{
    float_rand = 0.001 * random();
    int_rand = float_rand;
    usleep(int_rand);
}
void por_garfo(int i)
{
    sem_wait(&mutex);
    state[i] = THINKING;
    mostrar();
    test(LEFT);
    test(RIGHT);
    // state[i] = THINKING;
    sem_post(&mutex);
}
void statistic()
{
    printf("\n\nEstatistica do jantar dos filosofos\n\n");
    for (int i = 0; i < N; i++)
        printf("Filosofo %d tentou comer %d mas só comeu %d refeições.\n", i + 1,
               tried_to_eat[i], ate[i]);
    printf("\n");
    exit(EXIT_SUCCESS);
}