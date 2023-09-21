# Project Structure

![Project Structure](/GAProject/assets/schema.png "Project Structure")

Explanation:

**Child Process**: Going to limit CPU and Memory. To do that, is being used _setrlimit_. Going to use PIPE to communicate with the parent process.

**Additional Metric**: Disk usage or priority level

**Simulation**:

- Going to simulate that the process is using the amount of memory and CPU that the task requires.
- Going to simulate that the process is using the amount of disk that the task requires.
- Going to update the disk free and used space when the process is finished.
- Going to use Round Robin to know which process is going to be executed next. Going to add a priority level to the metrics and based on that is going to be decided which task is going to be executed next.

**Data Structure**:

```
Struct POD {
  int id
  double cpu_required
  double mem_required
  double disk_required
  int priority
}

Struct Disk {
  double total_size
  double used_space
  double free_space
}
```
