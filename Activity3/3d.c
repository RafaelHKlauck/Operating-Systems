/*
  Create a program to create N processes(beyond the parent). The first process must create the second process,
  the second process must create the third process, and so on.
  Each process must print its PID and the PID of its parent.
  The print order must be from the N process to the first process.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void create_process(int i, int number_of_processes)
{
  pid_t child_pid = fork();
  int status;
  if (child_pid == 0)
  {
    // Child process
    if (i != number_of_processes)
      create_process(i + 1, number_of_processes);
    printf("PID = %d, PPID = %d\n", getpid(), getppid());
    _Exit(0);
  }
  else
  {
    // Parent process
    waitpid(child_pid, &status, 0);
  }
}

int main()
{
  int number_of_processes;
  printf("Enter number of processes to create: ");
  scanf("%d", &number_of_processes);
  int i = 1;
  create_process(i, number_of_processes);
  return 0;
}