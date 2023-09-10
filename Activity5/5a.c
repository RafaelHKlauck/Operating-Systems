/*
  This code is rewrite a text using fliget. It should receive a signal
  that is going to execute a function that calls fliget in a child process.
  The text must be typed previously by the user
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

char user_text[100];

// Function that is going to be called when a signal is received
void signal_dealer(int signum)
{
  if (signum == SIGUSR1)
  {
    pid_t pid = fork();
    if (pid == 0)
    {
      char command[100];
      sprintf(command, "figlet %s", user_text);
      system(command);
    }
  }
  if (signum == SIGUSR2)
  {
    printf("You can now write another text: ");
    fflush(stdout);
    scanf("%s", user_text);
  }
}

int main()
{
  sigset_t mask;
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = &signal_dealer;

  sigaction(SIGUSR1, &action, NULL);
  sigaction(SIGUSR2, &action, NULL);

  printf("My PID: %d\n", getpid());
  fflush(stdout);

  sigfillset(&mask);
  sigdelset(&mask, SIGUSR1);
  sigdelset(&mask, SIGUSR2);
  sigdelset(&mask, SIGINT);

  printf("Type a text: ");
  fflush(stdout);
  scanf("%s", user_text);

  while (1)
  {
    sigsuspend(&mask);
    // Time to write the text on the screen
    sleep(1);
  }

  return 0;
}