#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include <stdbool.h>

#include "./sudoku_grid.h"

bool complete_valid_puzzle(const struct sudoku_grid* grid);

bool valid_puzzle_tile(const struct sudoku_grid* grid, unsigned int offset);

/* Eliminate impossible options due to row neighbors */
void row_eliminate(struct sudoku_grid* grid, unsigned int row);

/* Eliminate impossible options due to column neighbors */
void column_eliminate(struct sudoku_grid* grid, unsigned int column);

/* Eliminate impossible options due to box neighbors. Boxs are ordered in
  reading style. */
void box_eliminate(struct sudoku_grid* grid, unsigned int box);


/* If there is only one tile that contains an option in a section that tile
  must be that option */
void row_single_tile(struct sudoku_grid* grid, unsigned int row);

void column_single_tile(struct sudoku_grid* grid, unsigned int column);

void box_single_tile(struct sudoku_grid* grid, unsigned int box);


void row_box_elimination(struct sudoku_grid* grid, unsigned int index);

void column_box_elimination(struct sudoku_grid* grid, unsigned int index);

/* TODO START */
void box_row_elimination(struct sudoku_grid* grid, unsigned int index);

void box_column_elimination(struct sudoku_grid* grid, unsigned int index);

/* TODO END */

/* Check if N spots are the only N to have N numbers in a section. That
  almost seems obfuscated even though it is alas trying to be clear.

  (1, 2, 9), and (1, 5, 6, 9) hypothetically could be the only two to have
  both a 1, and 9 in a row.

  Another example is (2, 3), (3, 4), (4, 2) which are the only three to have
  three.
*/
void group_elimination(struct sudoku_grid* grid, unsigned int type,
    unsigned int index, unsigned int bound);

#endif
