/*
  This code is to implement two processes that are going to do a task together.
  One process create a matrix and send it to the other process.
  The second process is going to read the matrix, create a new one and multiply them.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MATRIX_COLUMN 3
#define MATRIX_ROW 3

// Function declarations
void create_matrix(int matrix[MATRIX_ROW][MATRIX_COLUMN]);
void calculate_matrix(int parentMatrix[MATRIX_ROW][MATRIX_COLUMN], int childMatrix[MATRIX_ROW][MATRIX_COLUMN]);
void print_matrix(int matrix[MATRIX_ROW][MATRIX_COLUMN]);

int main()
{
  int file_pipes[2], status;
  pid_t fork_result;

  if (pipe(file_pipes) == 0)
  {

    fork_result = fork();
    if (fork_result == 0)
    {
      // Child Process
      close(0);
      dup(file_pipes[0]);
      close(file_pipes[1]);

      int parentMatrix[MATRIX_ROW][MATRIX_COLUMN];
      read(file_pipes[0], parentMatrix, sizeof(parentMatrix));
      close(file_pipes[0]);

      int childMatrix[MATRIX_ROW][MATRIX_COLUMN];
      srand(getpid());
      create_matrix(childMatrix);
      printf("Second matrix:\n");
      print_matrix(childMatrix);
      printf("\n");

      calculate_matrix(parentMatrix, childMatrix);
      exit(0);
    }
    else
    {
      // Parent Process
      srand(getpid());
      close(file_pipes[0]);
      int matrix[MATRIX_ROW][MATRIX_COLUMN];
      create_matrix(matrix);

      printf("First matrix:\n");
      print_matrix(matrix);
      printf("\n");

      write(file_pipes[1], matrix, sizeof(matrix));
      close(file_pipes[1]);
      waitpid(fork_result, &status, 0);
    }
  }
  exit(EXIT_SUCCESS);
}

// Function definitions
void create_matrix(int matrix[MATRIX_ROW][MATRIX_COLUMN])
{
  int i, j;
  for (i = 0; i < MATRIX_ROW; ++i)
  {
    for (j = 0; j < MATRIX_COLUMN; ++j)
      // Generate a random number between 1 and 10
      matrix[i][j] = (rand() % 10) + 1;
  }
}

void calculate_matrix(int parentMatrix[MATRIX_ROW][MATRIX_COLUMN], int childMatrix[MATRIX_ROW][MATRIX_COLUMN])
{
  // Calculate the matrix with the size
  int resultMatrix[MATRIX_ROW][MATRIX_COLUMN];
  for (int i = 0; i < MATRIX_ROW; i++)
  {
    for (int j = 0; j < MATRIX_COLUMN; j++)
    {
      resultMatrix[i][j] = 0;
      for (int k = 0; k < MATRIX_ROW; k++)
      {
        resultMatrix[i][j] += parentMatrix[i][k] * childMatrix[k][j];
      }
    }
  }

  printf("Resulting matrix:\n");
  print_matrix(resultMatrix);
}

void print_matrix(int matrix[MATRIX_ROW][MATRIX_COLUMN])
{
  int i, j;
  for (i = 0; i < MATRIX_ROW; ++i)
  {
    for (j = 0; j < MATRIX_COLUMN; ++j)
      printf("%d ", matrix[i][j]);
    printf("\n");
  }
}