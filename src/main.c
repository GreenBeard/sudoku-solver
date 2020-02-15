#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

bool solve_puzzle(char* file_name) {
  struct sudoku_grid grid;

  FILE* file = fopen(file_name, "r");
  assert(file != NULL);
  bool status = read_in_puzzle(file, &grid);
  assert(status);
  fclose(file);

  /* Change to event based in order to improve speed (on change to tile queue
    rechecking row, column, etc.). Have a backup full check on no events for
    simplicity. */
  unsigned int complexity_level = 0;

  /*struct sudoku_grid solution;
  return recursive_solve(&solution, &grid, 0);*/

  bool grid_changed;
  while (true) {
    print_sudoku_grid(&grid);
    printf("\n");

    struct sudoku_grid grid_before;
    memcpy(&grid_before, &grid, sizeof(grid_before));

    if (complexity_level >= 0) {
      for (unsigned int i = 0; i < 9; ++i) {
        row_eliminate(&grid, i);
        column_eliminate(&grid, i);
        box_eliminate(&grid, i);
      }

      for (unsigned int i = 0; i < 9; ++i) {
        row_single_tile(&grid, i);
        column_single_tile(&grid, i);
        box_single_tile(&grid, i);
      }
    }

    if (complexity_level >= 1) {
      for (unsigned int i = 0; i < 9; ++i) {
        row_box_elimination(&grid, i);
        column_box_elimination(&grid, i);
      }
    }

    if (complexity_level >= 2) {
      for (unsigned int i = 0; i < 9; ++i) {
        group_elimination(&grid, 0, i, 3);
        group_elimination(&grid, 1, i, 3);
        group_elimination(&grid, 2, i, 3);
      }
    }

    grid_changed = memcmp(grid.values, grid_before.values,
      9 * 9 * sizeof(unsigned short)) != 0;

    if (!grid_changed) {
      if (complexity_level == 2) {
        break;
      } else {
        ++complexity_level;
      }
    }
  }

  return complete_valid_puzzle(&grid);
}

int main(int argc, char** argv) {
  if (argc == 2) {
    /*for (unsigned int i = 0; i < 1000; ++i) {
      bool arg = solve_puzzle(argv[1]);
      printf("%u\n", arg);
    }*/
    solve_puzzle(argv[1]);

    return 0;
  } else {
    printf("%s FILE\n", argc >= 1 ? argv[0] : "sudoku_solver");
    return 1;
  }
}
