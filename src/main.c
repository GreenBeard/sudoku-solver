#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./sudoku_generator.h"
#include "./sudoku_grid.h"
#include "./sudoku_solver.h"

bool read_in_puzzle(FILE* file, struct sudoku_grid* grid) {
  unsigned int tiles_read = 0;
  do {
    char c = getc(file);
    if (c != EOF) {
      if (c == ' ') {
        grid->values[tiles_read] = TILE_ALL;
        ++tiles_read;
      } else if (c <= '9' && c >= '1') {
        grid->values[tiles_read] = TILE(c - '0');
        ++tiles_read;
      } else {
        /* do nothing */
      }
    } else {
      return false;
    }
  } while (tiles_read < 9 * 9);
  return true;
}

bool recursive_solve(struct sudoku_grid* solution,
  const struct sudoku_grid* grid, unsigned int offset) {
  if (offset != 0 && !valid_puzzle_tile(grid, offset - 1)) {
    return false;
  } else if (offset == 9 * 9) {
    if (complete_valid_puzzle(grid)) {
      memcpy(solution, grid, sizeof(*grid));
      return true;
    } else {
      return false;
    }
  } else if (grid->values[offset] != TILE_ALL) {
    return recursive_solve(solution, grid, offset + 1);
  } else {
    struct sudoku_grid grid_copy;
    memcpy(&grid_copy, grid, sizeof(grid_copy));
    for (unsigned int i = 0; i < 9; ++i) {
      grid_copy.values[offset] = TILE(1 + i);
      if (recursive_solve(solution, &grid_copy, offset + 1)) {
        return true;
      }
    }
    return false;
  }
}

bool solve_puzzle(const struct sudoku_grid* grid) {
  struct sudoku_grid grid_copy;

  memcpy(&grid_copy, grid, sizeof(*grid));

  /* Change to event based in order to improve speed (on change to tile queue
    rechecking row, column, etc.). Have a backup full check on no events for
    simplicity. */
  unsigned int complexity_level = 0;

  /*struct sudoku_grid solution;
  return recursive_solve(&solution, &grid_copy, 0);*/

  bool grid_changed;
  while (true) {
    print_sudoku_grid(&grid_copy);
    printf("\n");

    struct sudoku_grid grid_before;
    memcpy(&grid_before, &grid_copy, sizeof(grid_before));

    if (complexity_level >= 0) {
      for (unsigned int i = 0; i < 9; ++i) {
        row_eliminate(&grid_copy, i);
        column_eliminate(&grid_copy, i);
        box_eliminate(&grid_copy, i);
      }

      for (unsigned int i = 0; i < 9; ++i) {
        row_single_tile(&grid_copy, i);
        column_single_tile(&grid_copy, i);
        box_single_tile(&grid_copy, i);
      }
    }

    if (complexity_level >= 1) {
      for (unsigned int i = 0; i < 9; ++i) {
        row_box_elimination(&grid_copy, i);
        column_box_elimination(&grid_copy, i);
      }
    }

    if (complexity_level >= 2) {
      for (unsigned int i = 0; i < 9; ++i) {
        group_elimination(&grid_copy, 0, i, 3);
        group_elimination(&grid_copy, 1, i, 3);
        group_elimination(&grid_copy, 2, i, 3);
      }
    }

    grid_changed = memcmp(grid_copy.values, grid_before.values,
      9 * 9 * sizeof(unsigned short)) != 0;

    if (!grid_changed) {
      if (complexity_level == 2) {
        break;
      } else {
        ++complexity_level;
      }
    }
  }

  return complete_valid_puzzle(&grid_copy);
}

bool solve_puzzle_file_name(char* file_name) {
  struct sudoku_grid grid;

  FILE* file = fopen(file_name, "r");
  assert(file != NULL);
  bool status = read_in_puzzle(file, &grid);
  assert(status);
  fclose(file);

  return solve_puzzle(&grid);
}

#define THREAD_COUNT 4

int main(int argc, char** argv) {
  if (argc == 2) {
    if (strcmp("stats", argv[1]) == 0) {
      pthread_t threads[THREAD_COUNT];
      struct generator_params params;
      params.difficulty = sudoku_difficulty_expert;
      params.exit_thread = malloc(sizeof(bool));
      params.queue = malloc(sizeof(struct fixed_queue_sudoku));
      int mutex_status = pthread_mutex_init(&params.mutex, NULL);
      assert(mutex_status == 0);
      assert(params.exit_thread != NULL);
      assert(params.queue != NULL);

      params.queue->used_size = 0;
      params.queue->first_unused = 0;

      for (unsigned int i = 0; i < THREAD_COUNT; ++i) {
        int status = pthread_create(threads + i, NULL, sudoku_generator_worker,
          &params);
        assert(status == 0);
      }

      unsigned int solved = 0;
      unsigned int attempted = 0;

      while (attempted < 2 * 1024) {
        pthread_mutex_lock(&params.mutex);

        if (params.queue->used_size > 0) {
          struct sudoku_grid* grid
            = params.queue->grids + params.queue->first_unused;
          pthread_mutex_unlock(&params.mutex);
          if (solve_puzzle(grid)) {
            ++solved;
          }
          ++attempted;
          pthread_mutex_lock(&params.mutex);
          params.queue->first_unused = (params.queue->first_unused + 1)
            % QUEUE_SIZE;
          --params.queue->used_size;
          unsigned int used_size = params.queue->used_size;
          pthread_mutex_unlock(&params.mutex);

          printf("Attempted: %4u Solved: %4u Used Size: %4u\n", attempted,
            solved, used_size);
        } else {
          pthread_mutex_unlock(&params.mutex);

          struct timespec ts;
          ts.tv_sec = 0;
          ts.tv_nsec = 50 * 1000 * 1000;
          nanosleep(&ts, NULL);
        }
      }

      pthread_mutex_lock(&params.mutex);
      *params.exit_thread = true;
      pthread_mutex_unlock(&params.mutex);

      for (unsigned int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], NULL);
      }

      return 0;
    } else {
      /*for (unsigned int i = 0; i < 1000; ++i) {
        bool arg = solve_puzzle_file_name(argv[1]);
        printf("%u\n", arg);
      }*/
      solve_puzzle_file_name(argv[1]);
    }

    return 0;
  } else {
    printf("%s FILE - For attempting to solve a puzzle\n", argc >= 1 ? argv[0]
      : "sudoku_solver");
    printf("%s stats - For solving complexity statistics (recommend disabling printing in source code first)\n", argc >= 1 ? argv[0]
      : "sudoku_solver");
    return 1;
  }
}
