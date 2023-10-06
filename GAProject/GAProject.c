#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

#define NUM_THREADS 2
#define NUM_TASKS 20

typedef struct
{
  int id;
  double cpu_required;
  double mem_required;
  double disk_required;
  int is_done;
  int is_running;
  char doing_by;
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
WORKER workerA = {'A', 0.4, 1500, 1000};
WORKER workerB = {'B', 0.7, 2048, 800};

// Array with jobs for workes
POD workerA_pods[NUM_TASKS];
POD workerB_pods[NUM_TASKS];
int workerA_number_of_pods = 0;
int workerB_number_of_pods = 0;

// Mutexs and conditions variables
pthread_mutex_t pod_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t workerA_pods_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for worker A pods
pthread_mutex_t workerB_pods_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for worker B pods
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;        // Mutex to print on screen

pthread_cond_t workerA_pods_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t workerB_pods_condition = PTHREAD_COND_INITIALIZER;

int current_task = 0;
int tasks_completed = 0;
int is_custom_scheduler = 0;
int is_fixed_pods = 0;
int can_finish_workers_prints = 0;

pthread_t workers_threads_simulations[NUM_TASKS];

POD pods[NUM_TASKS];

// Used only in kube_scheduler
double cpuWeight = 1.0;
double memWeight = 2.0;

WINDOW *pods_initialized_win;
WINDOW *worker_metrics_win;
WINDOW *pods_win;
WINDOW *scheduler_time;

void getFixedPods()
{
  pods[0] = (POD){0, 0.1, 512, 100, 0, 0, ' '};
  pods[1] = (POD){1, 0.1, 512, 200, 0, 0, ' '};
  pods[2] = (POD){2, 0.2, 1024, 300, 0, 0, ' '};
  pods[3] = (POD){3, 0.4, 2048, 400, 0, 0, ' '};
  pods[4] = (POD){4, 0.4, 512, 500, 0, 0, ' '};
  pods[5] = (POD){5, 0.2, 1024, 100, 0, 0, ' '};
  pods[6] = (POD){6, 0.3, 1536, 200, 0, 0, ' '};
  pods[7] = (POD){7, 0.4, 1024, 300, 0, 0, ' '};
  pods[8] = (POD){8, 0.3, 1000, 400, 0, 0, ' '};
  pods[9] = (POD){9, 0.5, 1024, 500, 0, 0, ' '};
  pods[10] = (POD){10, 0.1, 512, 100, 0, 0, ' '};
  pods[11] = (POD){11, 0.1, 512, 200, 0, 0, ' '};
  pods[12] = (POD){12, 0.2, 1024, 300, 0, 0, ' '};
  pods[13] = (POD){13, 0.4, 2048, 400, 0, 0, ' '};
  pods[14] = (POD){14, 0.4, 512, 500, 0, 0, ' '};
  pods[15] = (POD){15, 0.2, 1024, 100, 0, 0, ' '};
  pods[16] = (POD){16, 0.3, 1536, 200, 0, 0, ' '};
  pods[17] = (POD){17, 0.4, 1024, 300, 0, 0, ' '};
  pods[18] = (POD){18, 0.3, 1000, 400, 0, 0, ' '};
  pods[19] = (POD){19, 0.5, 1024, 500, 0, 0, ' '};
}

void initializePods()
{
  srand(time(NULL));
  werase(pods_initialized_win);
  mvwprintw(pods_initialized_win, 0, 1, "Pods initialized:");
  for (int i = 0; i < NUM_TASKS; i++)
  {
    if (is_fixed_pods == 0)
    {
      pods[i].id = i;
      pods[i].cpu_required = (rand() % 5 + 1) * 0.1;  // CPU required between 0.1 - 0.5
      pods[i].mem_required = (rand() % 1537) + 512;   // Memory required between 512 - 2048
      pods[i].disk_required = (rand() % 5 + 1) * 100; // Disk required between 100 - 500
      pods[i].is_done = 0;
      pods[i].is_running = 0;
      pods[i].doing_by = ' ';
    }
    else
      getFixedPods();

    mvwprintw(pods_initialized_win, 1, 1, "Pod Name");
    mvwprintw(pods_initialized_win, 1, 10, "CPU");
    mvwprintw(pods_initialized_win, 1, 15, "Memory");
    if (is_custom_scheduler == 1)
      mvwprintw(pods_initialized_win, 1, 25, "Disk");

    mvwprintw(pods_initialized_win, i + 2, 1, "Pod %d", pods[i].id);
    mvwprintw(pods_initialized_win, i + 2, 10, "%.2lf", pods[i].cpu_required);
    mvwprintw(pods_initialized_win, i + 2, 15, "%.2lf", pods[i].mem_required);
    if (is_custom_scheduler == 1)
      mvwprintw(pods_initialized_win, i + 2, 25, "%.2lf", pods[i].disk_required);
  }
  wrefresh(pods_initialized_win);
}

void render_worker_metrics()
{
  werase(worker_metrics_win);
  mvwprintw(worker_metrics_win, 0, 1, "Worker Name");
  mvwprintw(worker_metrics_win, 0, 15, "CPU");
  mvwprintw(worker_metrics_win, 0, 20, "Memory");
  if (is_custom_scheduler == 1)
    mvwprintw(worker_metrics_win, 0, 30, "Disk");

  mvwprintw(worker_metrics_win, 1, 1, "Worker A");
  mvwprintw(worker_metrics_win, 1, 15, "%.2lf", workerA.cpu_capacity);
  mvwprintw(worker_metrics_win, 1, 20, "%.2lf", workerA.mem_capacity);
  if (is_custom_scheduler == 1)
    mvwprintw(worker_metrics_win, 1, 30, "%.2lf", workerA.disk_capacity);

  mvwprintw(worker_metrics_win, 2, 1, "Worker B");
  mvwprintw(worker_metrics_win, 2, 15, "%.2lf", workerB.cpu_capacity);
  mvwprintw(worker_metrics_win, 2, 20, "%.2lf", workerB.mem_capacity);
  if (is_custom_scheduler == 1)
    mvwprintw(worker_metrics_win, 2, 30, "%.2lf", workerB.disk_capacity);

  wrefresh(worker_metrics_win);
}

void *worker_metrics_print(void *arg)
{
  while (can_finish_workers_prints == 0)
  {
    pthread_mutex_lock(&print_mutex);
    render_worker_metrics();
    pthread_mutex_unlock(&print_mutex);
    sleep(2);
  }
  return NULL;
}

void *running_pods_print(void *arg)
{
  while (can_finish_workers_prints == 0)
  {
    pthread_mutex_lock(&print_mutex);
    POD running_pods[NUM_TASKS];
    int running_pods_count = 0;

    for (int i = 0; i < NUM_TASKS; i++)
    {
      POD pod = pods[i];
      if (pod.is_running == 1 && pod.is_done == 0)
      {
        running_pods[running_pods_count] = pod;
        running_pods_count++;
      }
    }

    werase(pods_win);
    mvwprintw(pods_win, 0, 1, "Pod Name");
    mvwprintw(pods_win, 0, 10, "Worker Name");
    mvwprintw(pods_win, 0, 25, "CPU");
    mvwprintw(pods_win, 0, 30, "Memory");
    if (is_custom_scheduler == 1)
      mvwprintw(pods_win, 0, 40, "Disk");

    for (int i = 0; i < running_pods_count; i++)
    {
      POD pod = running_pods[i];

      mvwprintw(pods_win, i + 1, 1, "Pod %d", pod.id);
      mvwprintw(pods_win, i + 1, 10, "Worker %c", pod.doing_by);
      mvwprintw(pods_win, i + 1, 25, "%.2lf", pod.cpu_required);
      mvwprintw(pods_win, i + 1, 30, "%.2lf", pod.mem_required);
      if (is_custom_scheduler == 1)
        mvwprintw(pods_win, i + 1, 40, "%.2lf", pod.disk_required);
    }
    wrefresh(pods_win);
    pthread_mutex_unlock(&print_mutex);
    sleep(1);
  }
  return NULL;
}

void *pods_done_print(void *arg)
{
  while (can_finish_workers_prints == 0)
  {
    pthread_mutex_lock(&print_mutex);
    mvwprintw(pods_initialized_win, 1, is_custom_scheduler == 1 ? 35 : 25, "Done by");

    for (int i = 0; i < NUM_TASKS; i++)
    {
      POD pod = pods[i];
      if (pod.is_done == 0)
        continue;
      mvwprintw(pods_initialized_win, i + 2, is_custom_scheduler == 1 ? 35 : 25, "%c", pod.doing_by);
    }

    wrefresh(pods_initialized_win);
    pthread_mutex_unlock(&print_mutex);
    sleep(2);
  }
  return NULL;
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

  if (worker_name == 'A')
  {
    workerA.cpu_capacity -= pod_cpu_required;
    workerA.mem_capacity -= pod_mem_required;
    workerA.disk_capacity -= pod_disk_required;
  }
  else if (worker_name == 'B')
  {
    workerB.cpu_capacity -= pod_cpu_required;
    workerB.mem_capacity -= pod_mem_required;
    workerB.disk_capacity -= pod_disk_required;
  }

  sleep(pod_cpu_required * 20);

  pthread_mutex_lock(&pod_mutex);
  pods[pod_id].is_done = 1;
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
    workerA.cpu_capacity += pod_cpu_required;
    workerA.mem_capacity += pod_mem_required;
    workerA.disk_capacity += pod_disk_required;
  }
  else if (worker_name == 'B')
  {
    workerB.cpu_capacity += pod_cpu_required;
    workerB.mem_capacity += pod_mem_required;
    workerB.disk_capacity += pod_disk_required;
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
        pods[pod_id].doing_by = worker_name;
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
        pods[pod_id].doing_by = worker_name;
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

void kube_scheduler()
{
  pthread_mutex_lock(&pod_mutex);
  while (current_task < NUM_TASKS)
  {
    POD next_pod;
    next_pod.id = -1;
    int worker_id;

    for (int i = 0; i < NUM_TASKS; i++)
    {
      int is_running = pods[i].is_running;
      int is_done = pods[i].is_done;

      if (is_done == 1 || is_running == 1)
        continue;

      POD pod = pods[i];

      double cpu_required = pod.cpu_required;
      double mem_required = pod.mem_required;

      if (cpu_required <= workerA.cpu_capacity &&
          mem_required <= workerA.mem_capacity &&
          cpu_required <= workerB.cpu_capacity &&
          mem_required <= workerB.mem_capacity)
      {
        double cpuDiffWorkerA = workerA.cpu_capacity - pod.cpu_required;
        double memDiffWorkerA = workerA.mem_capacity - pod.mem_required;
        double cpuDiffWorkerB = workerB.cpu_capacity - pod.cpu_required;
        double memDiffWorkerB = workerB.mem_capacity - pod.mem_required;

        double normalizedCpuDiffA = cpuDiffWorkerA / workerA.cpu_capacity;
        double normalizedMemDiffA = memDiffWorkerA / workerA.mem_capacity;
        double normalizedCpuDiffB = cpuDiffWorkerB / workerB.cpu_capacity;
        double normalizedMemDiffB = memDiffWorkerB / workerB.mem_capacity;

        double weightedDiffA = (cpuWeight * normalizedCpuDiffA) + (memWeight * normalizedMemDiffA);
        double weightedDiffB = (cpuWeight * normalizedCpuDiffB) + (memWeight * normalizedMemDiffB);

        if (weightedDiffA > weightedDiffB)
        {
          next_pod = pods[i];
          worker_id = 0;
          break;
        }
        else if (weightedDiffB > weightedDiffA)
        {
          next_pod = pods[i];
          worker_id = 1;
          break;
        }
        else
        {
          int randomNum = rand() % 2;
          next_pod = pods[i];
          worker_id = randomNum;
          break;
        }
      }
      else if (cpu_required <= workerA.cpu_capacity &&
               mem_required <= workerA.mem_capacity)
      {
        next_pod = pods[i];
        worker_id = 0;
        break;
      }
      else if (cpu_required <= workerB.cpu_capacity &&
               mem_required <= workerB.mem_capacity)
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

void custom_scheduler()
{
  pthread_mutex_lock(&pod_mutex);
  while (current_task < NUM_TASKS)
  {
    POD next_pod;
    next_pod.id = -1;
    int worker_id;

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
          disk_required <= workerA.disk_capacity)
      {
        next_pod = pods[i];
        worker_id = 0;
        break;
      }

      if (cpu_required <= workerB.cpu_capacity &&
          mem_required <= workerB.mem_capacity &&
          disk_required <= workerB.disk_capacity)
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
  mvwprintw(pods_initialized_win, 1, is_custom_scheduler == 1 ? 35 : 25, "Done by");
  for (int i = 0; i < NUM_TASKS; i++)
  {
    mvwprintw(pods_initialized_win, i + 2, is_custom_scheduler == 1 ? 35 : 25, "%c", pods[i].doing_by);
  }
  wrefresh(pods_initialized_win);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("Incorrect: %s <number> <1=custom, 0=kubernetes> <0=random pods, 1=fixed pods>\n\n", argv[0]);
    return 1;
  }

  initscr(); // Initialize ncurses
  cbreak();  // Disable line buffering
  noecho();  // Disable echoing
  struct timespec start_time, end_time;

  int pods_initialized_win_num_lines = NUM_TASKS + 3;
  int worker_metrics_win_start_line = pods_initialized_win_num_lines + 1;
  int pods_win_start_line = worker_metrics_win_start_line + 4;

  pods_initialized_win = newwin(NUM_TASKS + 3, 60, 0, 0);
  worker_metrics_win = newwin(4, 40, worker_metrics_win_start_line, 0);
  pods_win = newwin(NUM_TASKS + 1, 60, worker_metrics_win_start_line + 4, 0);
  scheduler_time = newwin(1, 60, worker_metrics_win_start_line + 4, 0);

  werase(pods_initialized_win);
  werase(worker_metrics_win);
  werase(pods_win);
  werase(scheduler_time);

  is_custom_scheduler = atol(argv[1]);
  is_fixed_pods = atol(argv[2]);
  initializePods();

  pthread_t threads[NUM_THREADS];
  int thread_ids[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++)
  {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, worker, &thread_ids[i]);
  }

  pthread_t thread_metrics, thread_running_pods, thread_pods_done;
  pthread_create(&thread_metrics, NULL, worker_metrics_print, NULL);    // 2 seconds
  pthread_create(&thread_running_pods, NULL, running_pods_print, NULL); // 1 second
  pthread_create(&thread_pods_done, NULL, pods_done_print, NULL);       // 2 seconds

  clock_gettime(CLOCK_MONOTONIC, &start_time);
  if (is_custom_scheduler)
    custom_scheduler();
  else
    kube_scheduler();
  clock_gettime(CLOCK_MONOTONIC, &end_time);

  double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

  for (int i = 0; i < NUM_THREADS; i++)
  {
    pthread_join(threads[i], NULL);
  }
  can_finish_workers_prints = 1;
  pthread_join(thread_metrics, NULL);
  pthread_join(thread_running_pods, NULL);
  pthread_join(thread_pods_done, NULL);

  render_worker_metrics();
  werase(pods_win);

  history();
  mvwprintw(scheduler_time, 0, 1, "Time spent in scheduler %.2lf seconds", elapsed_time);
  wrefresh(scheduler_time);

  return 0;
}