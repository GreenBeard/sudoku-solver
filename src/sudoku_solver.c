#include "./sudoku_solver.h"

#include <assert.h>
#include <stdbool.h>

#include "./sudoku_grid.h"

static inline bool single_tile(unsigned short tile) {
  unsigned short count;
  asm(
    "popcnt %1, %0"
    : "=r" (count)
    : "g" (tile)
    : "cc"
  );
  return count == 1;
}

/*
  type:
    0 - row
    1 - column
    2 - box
*/
unsigned int grid_offset(unsigned int type, unsigned int main_index,
    unsigned int index) {
  assert(type < 3);
  switch (type) {
    case 0:
      return main_index * 9 + index;
    case 1:
      return index * 9 + main_index;
    case 2:
      return 27 * (main_index / 3) + 3 * (main_index % 3)
        + 9 * (index / 3) + (index % 3);
  }
  assert(false);
}

void section_eliminate(struct sudoku_grid* grid, unsigned int type,
    unsigned int index) {
  assert(index < 9);
  assert(type < 3);

  unsigned short tile = TILE_NONE;
  for (unsigned int i = 0; i < 9; ++i) {
    unsigned short value = grid->values[grid_offset(type, index, i)];
    if (single_tile(value)) {
      tile |= value;
    }
  }

  unsigned short tile_inverse = ~tile;
  for (unsigned int i = 0; i < 9; ++i) {
    unsigned int offset = grid_offset(type, index, i);
    if (!single_tile(grid->values[offset])) {
      grid->values[offset] &= tile_inverse;
    }
  }
}

void row_eliminate(struct sudoku_grid* grid, unsigned int row) {
  assert(row < 9);

  unsigned short tile = TILE_NONE;
  for (unsigned int i = 0; i < 9; ++i) {
    unsigned short value = grid->values[row * 9 + i];
    if (single_tile(value)) {
      tile |= value;
    }
  }

  unsigned short tile_inverse = ~tile;
  for (unsigned int i = 0; i < 9; ++i) {
    if (!single_tile(grid->values[row * 9 + i])) {
      grid->values[row * 9 + i] &= tile_inverse;
    }
  }
}

void column_eliminate(struct sudoku_grid* grid, unsigned int column) {
  assert(column < 9);

  unsigned short tile = TILE_NONE;
  for (unsigned int i = 0; i < 9; ++i) {
    unsigned short value = grid->values[i * 9 + column];
    if (single_tile(value)) {
      tile |= value;
    }
  }

  unsigned short tile_inverse = ~tile;
  for (unsigned int i = 0; i < 9; ++i) {
    if (!single_tile(grid->values[i * 9 + column])) {
      grid->values[i * 9 + column] &= tile_inverse;
    }
  }
}

void box_eliminate(struct sudoku_grid* grid, unsigned int box) {
  assert(box < 9);

  /*
    0, 3, 6
    27, 30, 33,
    ...
  */
  unsigned int box_base = 3 * (box % 3) + 27 * (box / 3);

  unsigned short tile = TILE_NONE;
  for (unsigned int i = 0; i < 3; ++i) {
    for (unsigned int j = 0; j < 3; ++j) {
      unsigned short value = grid->values[box_base + 9 * i + j];
      if (single_tile(value)) {
        tile |= value;
      }
    }
  }

  unsigned short tile_inverse = ~tile;
  for (unsigned int i = 0; i < 3; ++i) {
    for (unsigned int j = 0; j < 3; ++j) {
      if (!single_tile(grid->values[box_base + 9 * i + j])) {
        grid->values[box_base + 9 * i + j] &= tile_inverse;
      }
    }
  }
}

//void row_single_option
