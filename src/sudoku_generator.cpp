#include "./sudoku_generator.h"

#include <pthread.h>
#include <qqwing.hpp>
#include <stdbool.h>
#include <string.h>
#include <time.h>

using qqwing::SudokuBoard;

unsigned int convert_difficulty(SudokuBoard::Difficulty difficulty) {
  switch (difficulty) {
  case SudokuBoard::SIMPLE:
    return sudoku_difficulty_simple;
  case SudokuBoard::EASY:
    return sudoku_difficulty_easy;
  case SudokuBoard::INTERMEDIATE:
    return sudoku_difficulty_intermediate;
  case SudokuBoard::EXPERT:
    return sudoku_difficulty_expert;
  default:
    return 0;
  }
}

void* sudoku_generator_worker(void* raw_params) {
  struct generator_params* params = (struct generator_params*) raw_params;

  SudokuBoard board = SudokuBoard();
  while (true) {
    board.generatePuzzle();

    board.setRecordHistory(true);
    board.setLogHistory(false);
    board.solve();
    unsigned int difficulty = convert_difficulty(board.getDifficulty());
    if (board.hasUniqueSolution()
        && difficulty & params->difficulty) {
      struct sudoku_grid grid;

      {
        const int* puzzle_data = board.getPuzzle();
        for (unsigned int i = 0; i < 9 * 9; ++i) {
          if (puzzle_data[i] > 0 && puzzle_data[i] <= 9) {
            grid.values[i] = TILE(puzzle_data[i]);
          } else {
            grid.values[i] = TILE_ALL;
          }
        }
      }

      bool enqueued_puzzle = false;
      while (!enqueued_puzzle) {
        pthread_mutex_lock(&params->mutex);

        if (*params->exit_thread) {
          pthread_mutex_unlock(&params->mutex);
          goto exit_loops;
        } else if (params->queue->used_size < QUEUE_SIZE) {
          unsigned int grid_offset = (params->queue->used_size
            + params->queue->first_unused) % QUEUE_SIZE;
          memcpy(params->queue->grids + grid_offset, &grid, sizeof(grid));

          ++params->queue->used_size;
          pthread_mutex_unlock(&params->mutex);
          enqueued_puzzle = true;
        } else {
          pthread_mutex_unlock(&params->mutex);

          struct timespec ts;
          ts.tv_sec = 0;
          ts.tv_nsec = 50 * 1000 * 1000;
          nanosleep(&ts, NULL);
        }
      }
    }
  }
  exit_loops:

  return NULL;
}
