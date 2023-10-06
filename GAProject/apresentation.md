# Estrutura do Projeto

![Project Structure](/GAProject/assets/schema.png "Project Structure")

Como Funciona?

**Threads**: Duas threads que simulam os workers. Cada thread tem sua própria fila e suas próprias métricas. As threads vão rodar até que a fila esteja vazia.

**Métrica Adicional**: Uso de Disco

**Simulação**:

- O scheduler vai pegar a lista de pods e verificar qual worker pode fazer aquela tarefa, baseado nas métricas que o worker tem e nas métricas que a tarefa requer. Quando o scheduler achar um worker que pode fazer a tarefa, ele vai enviar a tarefa para a fila do worker.
- Vai ser simulado que a thread está usando a quantidade de Memória, CPU e Disco que a tarefa requer. Para fazer isso, o worker cria uma thread para executar a simulação da tarefa(adicionando um sleep). Essa nova thread é necessária porque o worker precisa ser capaz de receber novas tarefas enquanto está executando a tarefa atual.
- Antes de executar a tarefa(sleep), vai ser reduzido a quantidade de memória, cpu e disco que o worker tem. Depois que a tarefa terminar, o worker vai adicionar a quantidade de memória, cpu e disco que a tarefa requereu. Fazendo isso o scheduler pode verificar se o worker pode fazer outra tarefa ou não.

**O que está sendo usado**:

- Threads
- Mutex
- Condition
- ncurses.h

**ncurses.h**

- Biblioteca para criar interfaces de texto
- Ela cria uma janela e dentro dessa janela você pode colocar textos, bordas, etc.
- Para usar essa biblioteca, necessário compilar com a flag -lncurses

**Como rodar**

- compilar com a flag gcc _nome.c_ -o _nome_ -lpthread -lncurses
- rodar com ./nome passando os parametros necessários:
  - Primeiro parametro:
    - 0: escalonador do Kubernetes (não usa o disco)
    - 1: escalonador do projeto (usa o disco)
  - Segundo parametro:
    - 0: Pods aleatórios
    - 1: Pods fixos
