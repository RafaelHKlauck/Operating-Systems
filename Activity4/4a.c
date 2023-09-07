/*
  This code is to implement two processes that are going to do a task together.
  One process must download pages from Internet, using curl
  The second process is going to read the pages and count how many times a word is repeated.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_SIZE 1000000

int main()
{
  int file_pipes[2], status;
  pid_t fork_result;
  if (pipe(file_pipes) == 0)
  {
    fork_result = fork();
    if (fork_result == 0)
    {
      // Child code
      close(0);
      close(file_pipes[1]);
      dup(file_pipes[0]);
      system("egrep -oi 'Nova onda' | wc -l");
      close(file_pipes[0]);
      exit(0);
    }
    else
    {
      // Parent code
      close(file_pipes[0]);
      system("curl https://jornalnh.com.br/ --output page.txt -s");
      FILE *page_file = fopen("page.txt", "r");
      char buffer[MAX_SIZE];

      int i = 0;
      while (1)
      {
        buffer[i] = fgetc(page_file);
        if (buffer[i] == EOF)
          break;
        ++i;
      }
      pclose(page_file);
      write(file_pipes[1], buffer, strlen(buffer));
      close(file_pipes[1]);
    }
  }
  exit(EXIT_SUCCESS);
}
