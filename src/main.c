#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./sudoku_grid.h"
#include "./sudoku_solver.h"

int main(int argc, char** argv) {
  struct sudoku_grid grid;

  const unsigned short* puzzle = (const unsigned short[]) {
    TILE_ALL, TILE_ALL, TILE_ALL,   TILE_ALL,   TILE_3,   TILE_7,     TILE_4, TILE_ALL, TILE_ALL,
    TILE_ALL,   TILE_6, TILE_ALL,     TILE_5,   TILE_8,   TILE_9,   TILE_ALL, TILE_ALL,   TILE_2,
    TILE_ALL,   TILE_2, TILE_ALL,   TILE_ALL, TILE_ALL, TILE_ALL,   TILE_ALL,   TILE_7, TILE_ALL,

    TILE_ALL, TILE_ALL,   TILE_3,   TILE_ALL, TILE_ALL,   TILE_4,     TILE_6,   TILE_8, TILE_ALL,
      TILE_9, TILE_ALL, TILE_ALL,   TILE_ALL, TILE_ALL, TILE_ALL,   TILE_ALL, TILE_ALL,   TILE_7,
    TILE_ALL,   TILE_8,   TILE_2,     TILE_3, TILE_ALL, TILE_ALL,     TILE_9, TILE_ALL, TILE_ALL,

    TILE_ALL,   TILE_7, TILE_ALL,   TILE_ALL, TILE_ALL, TILE_ALL,   TILE_ALL,   TILE_4, TILE_ALL,
      TILE_5, TILE_ALL, TILE_ALL,     TILE_6,   TILE_9,   TILE_3,   TILE_ALL,   TILE_2, TILE_ALL,
    TILE_ALL, TILE_ALL,   TILE_8,     TILE_7,   TILE_4, TILE_ALL,   TILE_ALL, TILE_ALL, TILE_ALL
  };
  memcpy(grid.values, puzzle, 9 * 9 * sizeof(unsigned short));

  print_sudoku_grid(&grid);
  printf("\n");

  bool grid_changed;
  do {
    struct sudoku_grid grid_before;
    memcpy(&grid_before, &grid, sizeof(grid_before));

    for (unsigned int i = 0; i < 9; ++i) {
      row_eliminate(&grid, i);
    }

    for (unsigned int i = 0; i < 9; ++i) {
      column_eliminate(&grid, i);
    }

    for (unsigned int i = 0; i < 9; ++i) {
      box_eliminate(&grid, i);
    }

    grid_changed = memcmp(grid.values, grid_before.values,
      9 * 9 * sizeof(unsigned short)) != 0;

    print_sudoku_grid(&grid);
    printf("\n");
  } while (grid_changed);

  return 0;
}
