#ifndef SUDOKU_GENERATOR_H
#define SUDOKU_GENERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <stdbool.h>

#include "./sudoku_grid.h"

#define sudoku_difficulty_simple (1 << 0)
#define sudoku_difficulty_easy (1 << 1)
#define sudoku_difficulty_intermediate (1 << 2)
#define sudoku_difficulty_expert (1 << 3)

#define QUEUE_SIZE 1024

struct fixed_queue_sudoku {
  unsigned int used_size;
  unsigned int first_unused;
  struct sudoku_grid grids[QUEUE_SIZE];
};

struct generator_params {
  unsigned int difficulty;

  pthread_mutex_t mutex;
  bool* exit_thread;
  struct fixed_queue_sudoku* queue;
};

void* sudoku_generator_worker(void* raw_params);

#ifdef __cplusplus
}
#endif

#endif
