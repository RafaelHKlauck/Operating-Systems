/*
  Implement a program that creates two children processes.
  The first process needs to create a file and print number 1 to 10
  The second process needs to create a file and print number A - Z
  The parent process needs to wait for both children to finish and then list the content of both files.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
  int number_of_process = 2;
  int status = 0;
  pid_t child_pid, wpid;

  for (int i = 0; i < number_of_process; i++)
  {
    child_pid = fork();
    if (child_pid == 0)
    {
      // Child code
      printf("Doing the command\n");
      if (i == 0)
      {
        // First child
        char *file_path = "/home/ec2-user/aula3/numbers.txt";
        FILE *file = fopen(file_path, "w");
        fprintf(file, "1 2 3 4 5 6 7 8 9 10\n");
        fclose(file);
        exit(0);
      }
      else
      {
        // Second child
        char *file_path = "/home/ec2-user/aula3/letters.txt";
        FILE *file = fopen(file_path, "w");
        fprintf(file, "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z\n");
        fclose(file);
        exit(0);
      }
    }
  }
  // Waiting for all children to finish
  while ((wpid = wait(&status)) > 0)
  {
    printf("Waiting for children processes to finish\n");
  }
  char *argv[] = {"/bin/cat", "/home/ec2-user/aula3/numbers.txt", "/home/ec2-user/aula3/letters.txt", NULL};
  execve("/bin/cat", argv, NULL);
}