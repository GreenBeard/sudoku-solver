#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include "./sudoku_grid.h"

/* Eliminate impossible options due to row neighbors */
void row_eliminate(struct sudoku_grid* grid, unsigned int row);

/* Eliminate impossible options due to column neighbors */
void column_eliminate(struct sudoku_grid* grid, unsigned int column);

/* Eliminate impossible options due to box neighbors. Boxs are ordered in
  reading style. */
void box_eliminate(struct sudoku_grid* grid, unsigned int box);

#endif
