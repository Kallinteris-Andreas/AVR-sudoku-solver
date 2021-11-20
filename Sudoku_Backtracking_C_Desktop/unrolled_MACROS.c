#include <stdio.h>
/* UNROLLED LOOPS, BASED ON AVR4027-Section 4.3 */
/* MACROS based on AVR035, page 20 */

#define exists_in_row(row, value) \
{ \
  (grid[row][0] == value || grid[row][1] == value || grid[row][2] == value \
      || grid[row][3] == value || grid[row][4] == value || grid[row][5] == value \
      || grid[row][6] == value || grid[row][7] == value || grid[row][8] ==value); \
}

#define exists_in_column(column, value) \
{ \
  (grid[0][column] == value || grid[1][column] == value || grid[2][column] == value \
      || grid[3][column] == value || grid[4][column] == value || grid[5][column] == value \
      || grid[6][column] == value || grid[7][column] == value || grid[8][column] ==value); \
}

#define exists_in_box(row, column, value) \
{ \
  int row_of_box = row - row%3; \
  int column_of_box = column - column %3; \
  (grid[row_of_box][column_of_box] == value || \
    grid[row_of_box][column_of_box + 1] == value || \
    grid[row_of_box][column_of_box + 2] == value || \
    grid[row_of_box + 1][column_of_box] == value || \
    grid[row_of_box + 1][column_of_box + 1] == value || \
    grid[row_of_box + 1][column_of_box + 2] == value || \
    grid[row_of_box + 2][column_of_box] == value || \
    grid[row_of_box + 2][column_of_box + 1] == value || \
    grid[row_of_box + 2][column_of_box + 2] == value); \
}

//#define can_assign_it(row, column, value) { (!(exists_in_row(row, value))) && (!(exists_in_column(column, value))) && (!(exists_in_box(row, column, value))); }


/* 1 for TRUE, 0 for FALSE */

/*
* Possible improvents:
* 1) Convert to macros -- DONE
* 2) Pre-calculate and update them while solving
* 3) Unroll the loops, because there are only 9 iterations -- DONE
* 4);
*/
int grid[9][9];

int print_sudoku(){
  for(int i=0; i<9; i++){
    for(int j=0; j<9; j++){
      printf("%d - ", grid[i][j]);
    }
    printf("\n");
  }
}

int solve_it(){
  for(int i=0; i<9; i++){
    for(int j=0; j<9; j++){
      if(grid[i][j] == 0){
        for(int value=1; value<=9; value++){
          if((!(exists_in_row(i, value))) && (!(exists_in_column(j, value))) && (!(exists_in_box(i, j, value)))){
            grid[i][j] = value;
            if(solve_it()){
              return 1;
            }
            //else{
              grid[i][j] = 0;
            //}
          }
        }
        return 0; /*backtrack */
      }
    }
  }
  return 1;
}




int main(){
  /* A simple test with the given sudoku of the assignment */

  /* Initialize */
  for(int i=0; i<9; i++){
		for(int j=0; j<9; j++){
			grid[i][j] = 0;
		}
	}

  /* Fill the non-blank cells */
  grid[0][0] = 5;
	grid[0][2] = 6;
	grid[0][4] = 4;
	grid[0][5] = 3;
	grid[0][8] = 9;

	grid[1][5] = 1;

	grid[2][0] = 3;
	grid[2][3] = 9;
	grid[2][5] = 8;
	grid[2][6] = 7;
	grid[2][7] = 4;

	grid[3][1] = 4;
	grid[3][2] = 5;
	grid[3][4] = 1;
	grid[3][6] = 3;
	grid[3][7] = 8;
	grid[3][8] = 6;

	grid[4][0] = 6;
	grid[4][8] = 1;

	grid[5][0] = 8;
	grid[5][1] = 1;
	grid[5][2] = 2;
	grid[5][4] = 3;
	grid[5][6] = 5;
	grid[5][7] = 9;

	grid[6][1] = 7;
	grid[6][2] = 8;
	grid[6][3] = 1;
	grid[6][5] = 2;
	grid[6][8] = 3;

	grid[7][3] = 3;

	grid[8][0] = 2;
	grid[8][3] = 6;
	grid[8][4] = 8;
	grid[8][6] = 1;
	grid[8][8] = 4;
  printf("\nGiven:\n");
  print_sudoku();
  printf("\n\n");


  /* Solve and print */
  if(solve_it() == 1){
    printf("\nSolved:\n");
    print_sudoku();
  }
  else{
    print_sudoku();
    printf("\n Problem! \n");
  }

  return 0;
}
