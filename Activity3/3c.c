/*
  Create a program to create N processes. All processes must be created by the same parent process.
  Each process must print its PID and the PID of its parent.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
  int number_of_processes;
  printf("Enter number of processes to create: ");
  scanf("%d", &number_of_processes);

  for (int i = 0; i < number_of_processes; i++)
  {
    pid_t pid = fork();

    if (pid == 0)
    {
      // Child process
      printf("Child %d: PID = %d, PPID = %d\n", i + 1, getpid(), getppid());
      _Exit(0);
    }
    }

  // Parent process waits for all children to finish
  for (int i = 0; i < number_of_processes; i++)
  {
    wait(NULL);
  }

  return 0;
}