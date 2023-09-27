#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 2
#define NUM_TASKS 5

typedef struct
{
  int id;
  double cpu_required;
  double mem_required;
  double disk_required;
  int is_done;
  int is_running;
  char done_by;
} POD;
typedef struct
{
  char name;
  double cpu_capacity;
  double mem_capacity;
  double disk_capacity;
} WORKER;
typedef struct
{
  POD pod;
  char worker_name;
} WORKER_TASK_SIMULATION_ARGS;

// Simulating workers
WORKER workerA = {'A', 0.4, 2000, 1000};
WORKER workerB = {'B', 0.7, 2048, 1000};

// Array with jobs for workes
POD workerA_pods[NUM_TASKS];
POD workerB_pods[NUM_TASKS];
int workerA_number_of_pods = 0;
int workerB_number_of_pods = 0;

// Mutexs and conditions variables
pthread_mutex_t pod_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t workerA_pods_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for worker A pods
pthread_mutex_t workerB_pods_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for worker B pods
pthread_mutex_t tasks_completed_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t workerA_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex to update Worker A
pthread_mutex_t workerB_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex to update Worker B

pthread_cond_t workerA_pods_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t workerB_pods_condition = PTHREAD_COND_INITIALIZER;

int current_task = 0;
int tasks_completed = 0;
int should_use_disk = 0;

pthread_t workers_threads_simulations[NUM_TASKS];

POD pods[NUM_TASKS];

void initializePods()
{
  srand(time(NULL));
  for (int i = 0; i < NUM_TASKS; i++)
  {
    pods[i].id = i;
    pods[i].cpu_required = (rand() % 5 + 1) * 0.1;  // CPU required between 0.1 - 0.5
    pods[i].mem_required = (rand() % 1537) + 512;   // Memória requerida entre 512 e 2048
    pods[i].disk_required = (rand() % 5 + 1) * 100; // Espaço em disco requerido entre 100 e 500
    pods[i].is_done = 0;
    pods[i].is_running = 0;
    pods[i].done_by = ' ';

    printf("Pod: %d, CPU: %.2lf, Memória: %.2lf, Disco: %.2lf\n", pods[i].id, pods[i].cpu_required, pods[i].mem_required, pods[i].disk_required);
  }
}

void *worker_task_simulation(void *arg)
{
  WORKER_TASK_SIMULATION_ARGS *worker_task_simulation_args = (WORKER_TASK_SIMULATION_ARGS *)arg;
  POD pod = worker_task_simulation_args->pod;
  char worker_name = worker_task_simulation_args->worker_name;

  int pod_id = pod.id;
  double pod_cpu_required = pod.cpu_required;
  double pod_mem_required = pod.mem_required;
  double pod_disk_required = pod.disk_required;

  printf("\nWorker %c going to do task %d (CPU %.2lf, Memory %.2lf, Disk %.2lf)\n",
         worker_name, pod_id, pod_cpu_required, pod_mem_required, pod_disk_required);
  if (worker_name == 'A')
  {
    workerA.cpu_capacity -= pod_cpu_required;
    workerA.mem_capacity -= pod_mem_required;
    workerA.disk_capacity -= pod_disk_required;

    printf("Now worker A has CPU: %.2lf, Memory: %.2lf, Disk: %.2lf\n", workerA.cpu_capacity, workerA.mem_capacity, workerA.disk_capacity);
  }
  else if (worker_name == 'B')
  {
    workerB.cpu_capacity -= pod_cpu_required;
    workerB.mem_capacity -= pod_mem_required;
    workerB.disk_capacity -= pod_disk_required;

    printf("Now worker B has CPU: %.2lf, Memory: %.2lf, Disk: %.2lf\n", workerB.cpu_capacity, workerB.mem_capacity, workerB.disk_capacity);
  }

  sleep(pod_cpu_required * 10);
  printf("\nWorker %c finish task %d\n", worker_name, pod_id);

  pthread_mutex_lock(&pod_mutex);
  pods[pod_id].is_done = 1;
  pods[pod_id].done_by = worker_name;
  pthread_mutex_unlock(&pod_mutex);

  tasks_completed++;
  if (tasks_completed == NUM_TASKS)
  {
    // all tasks were completed!
    pthread_cond_signal(&workerA_pods_condition);
    pthread_cond_signal(&workerB_pods_condition);
  }

  if (worker_name == 'A')
  {
    pthread_mutex_lock(&workerA_mutex);
    workerA.cpu_capacity += pod_cpu_required;
    workerA.mem_capacity += pod_mem_required;
    workerA.disk_capacity += pod_disk_required;

    printf("Now worker A has CPU: %.2lf, Memória: %.2lf, Disco: %.2lf\n", workerA.cpu_capacity, workerA.mem_capacity, workerA.disk_capacity);
    pthread_mutex_unlock(&workerA_mutex);
  }
  else if (worker_name == 'B')
  {
    pthread_mutex_lock(&workerB_mutex);
    workerB.cpu_capacity += pod_cpu_required;
    workerB.mem_capacity += pod_mem_required;
    workerB.disk_capacity += pod_disk_required;

    printf("Now worker B has CPU: %.2lf, Memória: %.2lf, Disco: %.2lf\n", workerB.cpu_capacity, workerB.mem_capacity, workerB.disk_capacity);
    pthread_mutex_unlock(&workerB_mutex);
  }

  pthread_exit(0);
}

void *worker(void *arg)
{
  int thread_id = *(int *)arg;
  char worker_name = thread_id == 0 ? 'A' : 'B';

  while (1)
  {
    if (thread_id == 0)
    {
      // Worker A
      int should_break = 0;
      pthread_mutex_lock(&workerA_pods_mutex);
      while (workerA_number_of_pods == 0)
      {
        pthread_cond_wait(&workerA_pods_condition, &workerA_pods_mutex);
        if (tasks_completed == NUM_TASKS)
        {
          should_break = 1;
          break;
        }
      }
      if (should_break)
      {
        pthread_mutex_unlock(&workerA_pods_mutex);
        break;
      };
      for (int i = 0; i < workerA_number_of_pods; i++)
      {
        if (workerA_pods[i].is_done == 1 || workerA_pods[i].is_running == 1)
          continue;
        POD *pod = &workerA_pods[i];
        int pod_id = pod->id;

        pthread_mutex_lock(&pod_mutex);
        pods[pod_id].is_running = 1;
        pthread_mutex_unlock(&pod_mutex);

        WORKER_TASK_SIMULATION_ARGS worker_task_simulation_args;
        worker_task_simulation_args.pod = *pod;
        worker_task_simulation_args.worker_name = worker_name;

        pthread_t thread;
        pthread_create(&thread, NULL, worker_task_simulation, &worker_task_simulation_args);
        workerA_number_of_pods--;
      }
      pthread_mutex_unlock(&workerA_pods_mutex);
    }
    else
    {
      // Worker B
      int should_break = 0;
      pthread_mutex_lock(&workerB_pods_mutex);
      while (workerB_number_of_pods == 0 && tasks_completed < NUM_TASKS)
      {
        pthread_cond_wait(&workerB_pods_condition, &workerB_pods_mutex);
        if (tasks_completed == NUM_TASKS)
        {
          should_break = 1;
          break;
        }
      }
      if (should_break)
      {
        pthread_mutex_unlock(&workerB_pods_mutex);
        break;
      }
      for (int i = 0; i < workerB_number_of_pods; i++)
      {
        if (workerB_pods[i].is_done == 1 || workerB_pods[i].is_running == 1)
          continue;
        POD *pod = &workerB_pods[i];
        int pod_id = pod->id;

        pthread_mutex_lock(&pod_mutex);
        pods[pod_id].is_running = 1;
        pthread_mutex_unlock(&pod_mutex);

        WORKER_TASK_SIMULATION_ARGS worker_task_simulation_args;
        worker_task_simulation_args.pod = *pod;
        worker_task_simulation_args.worker_name = worker_name;

        pthread_t thread;
        pthread_create(&thread, NULL, worker_task_simulation, &worker_task_simulation_args);
        workerB_number_of_pods--;
      }
      pthread_mutex_unlock(&workerB_pods_mutex);
    }
  }

  for (int i = 0; i < NUM_TASKS; i++)
  {
    pthread_join(workers_threads_simulations[i], NULL);
  };

  pthread_exit(0);
}

void scheduler()
{
  pthread_mutex_lock(&pod_mutex);
  while (current_task < NUM_TASKS)
  {
    POD next_pod;
    next_pod.id = -1;
    int worker_id;

    // Find next task available that meets resource requirements
    for (int i = 0; i < NUM_TASKS; i++)
    {
      int is_running = pods[i].is_running;
      int is_done = pods[i].is_done;

      if (is_done == 1 || is_running == 1)
        continue;

      double cpu_required = pods[i].cpu_required;
      double mem_required = pods[i].mem_required;
      double disk_required = pods[i].disk_required;

      if (cpu_required <= workerA.cpu_capacity &&
          mem_required <= workerA.mem_capacity &&
          (should_use_disk == 1 ? disk_required <= workerA.disk_capacity : 1))
      {
        next_pod = pods[i];
        worker_id = 0;
        break;
      }

      if (cpu_required <= workerB.cpu_capacity &&
          mem_required <= workerB.mem_capacity &&
          (should_use_disk == 1 ? disk_required <= workerB.disk_capacity : 1))
      {
        next_pod = pods[i];
        worker_id = 1;
        break;
      }
    }

    if (next_pod.id != -1)
    {
      if (worker_id == 0)
      {
        // worker A
        pthread_mutex_lock(&workerA_pods_mutex);
        workerA_pods[workerA_number_of_pods] = next_pod;
        workerA_number_of_pods++;
        if (workerA_number_of_pods == 1)
        { // it was 0 before
          pthread_cond_signal(&workerA_pods_condition);
        }

        pthread_mutex_unlock(&workerA_pods_mutex);
      }
      else
      {
        // worker B
        pthread_mutex_lock(&workerB_pods_mutex);
        workerB_pods[workerB_number_of_pods] = next_pod;
        workerB_number_of_pods++;
        if (workerB_number_of_pods == 1)
        { // it was 0 before
          pthread_cond_signal(&workerB_pods_condition);
        }

        pthread_mutex_unlock(&workerB_pods_mutex);
      }

      current_task++;
    }

    pthread_mutex_unlock(&pod_mutex);
    usleep(100000); // 100 ms
  }
}

void history()
{
  printf("\n\n");
  system("figlet -f digital PODS history");
  for (int i = 0; i < NUM_TASKS; i++)
  {
    printf("Pod %d done by %c\n", pods[i].id, pods[i].done_by);
  }
  printf("\n");
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Incorrect: %s <number> <1=disk, 0=no disk> <1=random pods, 2=fixed pods>\n\n", argv[0]);
    return 1;
  }

  should_use_disk = atol(argv[1]);
  initializePods();

  pthread_t threads[NUM_THREADS];
  int thread_ids[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++)
  {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, worker, &thread_ids[i]);
  }

  scheduler();

  for (int i = 0; i < NUM_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }

  history();
  return 0;
}