# Project Structure

![Project Structure](/GAProject/assets/schema.png "Project Structure")

Explanation:

**Threads**: There are two threads that are simulating the workers. Each thread have it's own queue and it's own metrics. The threads are going to be running until the queue is empty.

**Additional Metric**: Disk usage

**Simulation**:

- The scheduler is going to get the list of pods and check which worker can do that task, based on the metrics that the worker has and the metrics that the task requires. When the scheduler finds a worker that can do the task, it's going to send the task to the worker's queue.
- Going to simulate that the thread is using the amount of Memory, CPU and Disk that the task requires. To do that, the worker creates a thread to execute the task simulation(adding one sleep). This new thread is necessary because the worker needs to be able to receive new tasks while it's executing the current task.
- Before running the task(sleep), it will be reduced the amount of memory, cpu and disk that the worker has. After the task is done, the worker will add the amount of memory, cpu and disk that the task required. Doing that the scheduler can check if the worker can do another task or not.

**What is being used**:

- Threads
- Mutex
- Condition
