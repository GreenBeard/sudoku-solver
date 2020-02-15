#ifndef SUDOKU_GRID_H
#define SUDOKU_GRID_H

#define TILE_1 (1 << 0)
#define TILE_2 (1 << 1)
#define TILE_3 (1 << 2)
#define TILE_4 (1 << 3)
#define TILE_5 (1 << 4)
#define TILE_6 (1 << 5)
#define TILE_7 (1 << 6)
#define TILE_8 (1 << 7)
#define TILE_9 (1 << 8)

#define TILE_ALL (TILE_1 | TILE_2 | TILE_3 | TILE_4 | TILE_5 | TILE_6 | TILE_7\
 | TILE_8 | TILE_9)
#define TILE_NONE 0

#define TILE(N) (1 << (N - 1))

struct sudoku_grid {
  unsigned short values[9 * 9];
};

void print_sudoku_grid(struct sudoku_grid* grid);

#endif
