#include "./sudoku_grid.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

/* The VT-100 escape character */
#define VT100_ESCAPE 0x1B

#define print_vert_char() printf("%c(0%c%c(B", VT100_ESCAPE, 0x78, VT100_ESCAPE)

#define vt100_fg_gray() printf("%c[49m", VT100_ESCAPE)
#define vt100_fg_black() printf("%c[102m", VT100_ESCAPE)
#define vt100_fg_reset() printf("%c[49m", VT100_ESCAPE)

/*
  Box using VT100 box-drawing characters
  ┌───┬
  │123│
  │456│
  │789│
  ├───┼
*/

enum sudoku_row {
  sudoku_row_top,
  sudoku_row_middle,
  sudoku_row_bottom
};

void print_sudoku_row(enum sudoku_row row, bool bold_row) {
  assert(row == sudoku_row_top || row == sudoku_row_middle
    || row == sudoku_row_bottom);
  printf("  ");
  /* Enable box-drawing alternative character set */
  printf("%c(0", VT100_ESCAPE);

  char corner_char_left;
  char corner_char_center;
  char corner_char_right;
  char line_char = 0x71;
  switch (row) {
  case sudoku_row_top:
    corner_char_left = 0x6c;
    corner_char_center = 0x77;
    corner_char_right = 0x6b;
    break;
  case sudoku_row_middle:
    corner_char_left = 0x74;
    corner_char_center = 0x6e;
    corner_char_right = 0x75;
    break;
  case sudoku_row_bottom:
    corner_char_left = 0x6d;
    corner_char_center = 0x76;
    corner_char_right = 0x6a;
    break;
  }

  vt100_fg_black();
  printf("%c", corner_char_left);
  /* 9 boxes per row */
  for (unsigned int i = 0; i < 9; ++i) {
    if (!bold_row) {
      vt100_fg_gray();
    }
    /* 3 chars per box */
    for (unsigned int j = 0; j < 3; ++j) {
      printf("%c", line_char);
    }
    if ((i + 1) % 3 == 0) {
      vt100_fg_black();
    }
    if (i != 8) {
      printf("%c", corner_char_center);
    }
  }
  printf("%c", corner_char_right);

  /* Disable box-drawing alternative character set */
  printf("%c(B", VT100_ESCAPE);
  /* Reset foreground color */
  vt100_fg_reset();
  printf("\n");
}

void print_sudoku_row_numbers(struct sudoku_grid* grid,
    unsigned char sudoku_row) {
  assert(sudoku_row < 9);

  /* 3 rows between separator lines */
  for (unsigned int i = 0; i < 3; ++i) {
    printf("  ");
    vt100_fg_black();
    print_vert_char();
    /* 9 boxes in a row */
    for (unsigned int j = 0; j < 9; ++j) {
      vt100_fg_gray();
      /* 3 adjacent in a box */
      for (unsigned int k = 0; k < 3; ++k) {
        unsigned short box_value =
          grid->values[9 * sudoku_row + j];
        unsigned char box_index = 3 * i + k;
        char c;
        if (box_value & TILE(1 + box_index)) {
          c = '1' + box_index;
        } else {
          c = ' ';
        }
        printf("%c", c);
      }
      if ((j + 1) % 3 == 0) {
        vt100_fg_black();
      }
      print_vert_char();
    }
    vt100_fg_reset();
    printf("\n");
  }
}

void print_sudoku_grid(struct sudoku_grid* grid) {
  print_sudoku_row(sudoku_row_top, true);
  for (unsigned int i = 0; i < 9; ++i) {
    print_sudoku_row_numbers(grid, i);
    if (i != 8) {
      print_sudoku_row(sudoku_row_middle, (i + 1) % 3 == 0);
    }
  }
  print_sudoku_row(sudoku_row_bottom, true);
}
