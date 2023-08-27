#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main()
{
  char user_input[100];
  char delimiter[] = " ";
  int status;

  while (1)
  {
    // Asking for the inputs:
    printf("Insert command: ");
    gets(user_input);

    // Splitting the input into command and parameter:
    char *command = strtok(user_input, delimiter);
    char *parameter = strtok(NULL, delimiter);

    // Creating the full command, to pass to execve:
    char fullCommand[200];
    snprintf(fullCommand, sizeof(fullCommand), "/bin/%s", command);
    char *argv[] = {fullCommand, parameter, "/", NULL};

    if (fork() != 0)
    {
      // Parent code
      waitpid(-1, &status, 0);
    }
    else
    {
      // Child code
      execve(fullCommand, argv, NULL);
    }
  }
}
