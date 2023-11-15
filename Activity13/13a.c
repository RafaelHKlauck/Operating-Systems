#include <stdio.h>
#include <string.h>

void checkFileSignature(const char *filename)
{
  FILE *file = fopen(filename, "rb");

  if (file == NULL)
  {
    printf("Erro ao abrir o arquivo.\n");
    return;
  }

  char buffer[8];

  fread(buffer, sizeof(char), 8, file);

  if (memcmp(buffer, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8) == 0)
  {
    printf("O arquivo é uma imagem PNG.\n");
  }
  else if (memcmp(buffer, "\xFF\xD8\xFF\xE0", 4) == 0 || memcmp(buffer, "\xFF\xD8\xFF\xE1", 4) == 0)
  {
    printf("O arquivo é uma imagem JPEG.\n");
  }
  else if (memcmp(buffer, "\x25\x50\x44\x46", 4) == 0)
  {
    printf("O arquivo é um documento PDF.\n");
  }
  else
  {
    printf("Tipo de arquivo não reconhecido.\n");
  }

  fclose(file);
}

int main()
{
  char filename[100];
  printf("Digite o caminho do arquivo e o nome: ");

  scanf("%s", filename);
  checkFileSignature(filename);

  return 0;
}