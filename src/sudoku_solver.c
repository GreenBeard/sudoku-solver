#include "./sudoku_solver.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>

#include "./sudoku_grid.h"

unsigned int tile_count(unsigned short tile) {
  unsigned short count;
  asm(
    "popcnt %1, %0"
    : "=r" (count)
    : "g" (tile)
    : "cc"
  );
  return count;
}

/* returns 0 for 1
   1 for 2, etc. invalid if more than one one
*/
unsigned int single_one_position(unsigned short num) {
  unsigned short offset;
  asm(
    "lzcnt %1, %0"
    : "=r" (offset)
    : "g" (num)
    : "cc"
  );
  return sizeof(num) * CHAR_BIT - 1 - offset;
}

bool single_tile(unsigned short tile) {
  return tile_count(tile) == 1;
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

void section_single_option(struct sudoku_grid* grid, unsigned int type,
    unsigned int index) {

  unsigned char count[9] = { 0 };

  for (unsigned int i = 0; i < 9; ++i) {
    unsigned int offset = grid_offset(type, index, i);
    for (unsigned int j = 0; j < 9; ++j) {
      if (TILE(1 + j) & grid->values[offset]) {
        ++count[j];
      }
    }
  }

  for (unsigned int i = 0; i < 9; ++i) {
    if (count[i] == 1) {
      for (unsigned int j = 0; j < 9; ++j) {
        unsigned int offset = grid_offset(type, index, j);
        if (TILE(1 + i) & grid->values[offset]) {
          grid->values[offset] = TILE(1 + i);
        }
      }
    }
  }
}

unsigned int box_offset_reverse_lookup(unsigned int offset) {
  unsigned int horizontal = (offset / 3) % 3;
  unsigned int vertical = (offset / 27);

  return horizontal + 3 * vertical;
}

unsigned int line_offset_reverse_lookup(unsigned int type,
    unsigned int offset) {
  assert(type == 0 || type == 1);

  switch (type) {
  case 0:
    return offset / 9;
  case 1:
    return offset % 9;
  }
  assert(false);
}

unsigned int reverse_lookup(unsigned int type, unsigned int offset) {
  assert(type < 3);

  if (type < 2) {
    return line_offset_reverse_lookup(type, offset);
  } else {
    return box_offset_reverse_lookup(offset);
  }
}

bool complete_valid_puzzle(const struct sudoku_grid* grid) {
  for (unsigned int type = 0; type < 3; ++type) {
    for (unsigned int i = 0; i < 9; ++i) {
      unsigned short tile = 0;
      for (unsigned int j = 0; j < 9; ++j) {
        unsigned int offset = grid_offset(type, i, j);
        if (single_tile(grid->values[offset])) {
          tile |= grid->values[offset];
        }
      }
      if (tile != TILE_ALL) {
        return false;
      }
    }
  }
  return true;
}

bool valid_puzzle_tile(const struct sudoku_grid* grid, unsigned int offset) {
  unsigned short tile = grid->values[offset];
  for (unsigned int i = 0; i < 3; ++i) {
    unsigned int index = reverse_lookup(i, offset);
    for (unsigned int j = 0; j < 9; ++j) {
      unsigned int tmp_offset = grid_offset(i, index, j);
      if (tmp_offset != offset
          && grid->values[tmp_offset] == tile) {
        return false;
      }
    }
  }
  return true;
}

/* All of X in a row/column is in one box. Therefore X cannot be anywhere in
  that box other than that row/column. The reverse can also be true. */
void line_box_elimination(struct sudoku_grid* grid, unsigned int type,
    unsigned int index) {
  assert(type == 0 || type == 1);

  unsigned char count[3][9] = { 0 };

  for (unsigned int i = 0; i < 9; ++i) {
    unsigned int offset = grid_offset(type, index, i);
    for (unsigned int j = 0; j < 9; ++j) {
      if (TILE(1 + j) & grid->values[offset]) {
        ++count[i / 3][j];
      }
    }
  }

  for (unsigned int i = 0; i < 9; ++i) {
    unsigned int zeroes = 0;
    for (unsigned int j = 0; j < 3; ++j) {
      if (count[j][i] == 0) {
        ++zeroes;
      }
    }
    if (zeroes == 2) {
      /* Only one box has all of a certain number */

      unsigned int offset;
      for (unsigned int k = 0; k < 9; ++k) {
        offset = grid_offset(type, index, k);
        if (TILE(1 + i) & grid->values[offset]) {
          break;
        }
      }
      unsigned int box = box_offset_reverse_lookup(offset);
      for (unsigned int k = 0; k < 9; ++k) {
        unsigned int offset = grid_offset(2, box, k);
        unsigned int line = line_offset_reverse_lookup(type, offset);
        if (line != index) {
          grid->values[offset] &= ~TILE(1 + i);
        }
      }
    }
  }
}

struct elimination_data {
  struct sudoku_grid* grid;
  unsigned int type;
  unsigned int inverse_type;
  unsigned int index;
};

void group_elimination_tile_check(const struct elimination_data* data,
    unsigned short tile_nums) {
  unsigned int match_count = 0;
  unsigned int num_count = tile_count(tile_nums);

  unsigned int sections = 0;

  for (unsigned int i = 0; i < 9; ++i) {
    unsigned int offset = grid_offset(data->type, data->index, i);
    if (tile_nums & data->grid->values[offset]) {
      ++match_count;
      sections |= (1 << reverse_lookup(data->inverse_type, offset));
    }
  }

  if (match_count == num_count && tile_count(sections) == 1) {
    unsigned int section_index = single_one_position(sections);
    for (unsigned int i = 0; i < 9; ++i) {
      unsigned int offset = grid_offset(data->inverse_type, section_index, i);
      if (reverse_lookup(data->type, offset) != data->index) {
        data->grid->values[offset] &= ~tile_nums;
      } else {
        if (data->grid->values[offset] & tile_nums) {
          data->grid->values[offset] &= tile_nums;
        }
      }
    }
  }
}

/* Top caller should use (data, count, bound, 1, 0, TILE_NONE) */
void group_elimination_helper(const struct elimination_data* data,
  const unsigned char count[9], unsigned int bound,
  unsigned int min_bound, unsigned int start, unsigned short tile_nums) {
  if (tile_count(tile_nums) == min_bound) {
    group_elimination_tile_check(data, tile_nums);
  }

  for (unsigned int i = start; i < 9; ++i) {
    if (count[i] <= bound && count[i] != 1) {
      unsigned int new_min_bound = min_bound > count[i] ? min_bound : count[i];
      group_elimination_helper(data, count, bound, new_min_bound, i + 1,
        tile_nums | TILE(1 + i));
    }
  }
}

void group_elimination(struct sudoku_grid* grid, unsigned int type,
    unsigned int index, unsigned int bound) {
  assert(type < 3);

  unsigned char count[9] = { 0 };

  for (unsigned int i = 0; i < 9; ++i) {
    unsigned int offset = grid_offset(type, index, i);
    for (unsigned int j = 0; j < 9; ++j) {
      if (TILE(1 + j) & grid->values[offset]) {
        ++count[j];
      }
    }
  }

  struct elimination_data data;
  data.grid = grid;
  data.type = type;
  data.index = index;

  if (type == 0 || type == 1) {
    data.inverse_type = 2;
    group_elimination_helper(&data, count, bound, 1, 0, TILE_NONE);
  } else {
    data.inverse_type = 0;
    group_elimination_helper(&data, count, bound, 1, 0, TILE_NONE);
    data.inverse_type = 1;
    group_elimination_helper(&data, count, bound, 1, 0, TILE_NONE);
  }
}

void row_eliminate(struct sudoku_grid* grid, unsigned int row) {
  section_eliminate(grid, 0, row);
}

void column_eliminate(struct sudoku_grid* grid, unsigned int column) {
  section_eliminate(grid, 1, column);
}

void box_eliminate(struct sudoku_grid* grid, unsigned int box) {
  section_eliminate(grid, 2, box);
}

void row_single_tile(struct sudoku_grid* grid, unsigned int row) {
  section_single_option(grid, 0, row);
}

void column_single_tile(struct sudoku_grid* grid, unsigned int column) {
  section_single_option(grid, 1, column);
}

void box_single_tile(struct sudoku_grid* grid, unsigned int box) {
  section_single_option(grid, 2, box);
}

void row_box_elimination(struct sudoku_grid* grid, unsigned int index) {
  line_box_elimination(grid, 0, index);
}

void column_box_elimination(struct sudoku_grid* grid, unsigned int index) {
  line_box_elimination(grid, 1, index);
}
