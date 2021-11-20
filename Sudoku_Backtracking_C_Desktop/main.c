#include <stdio.h>

/* 1 for TRUE, 0 for FALSE */

/*
* Possible improvents:
* 1) Convert to macros
* 2) Pre-calculate and update them while solving
* 3) Unroll the loops, because there are only 9 iterations
* 4);
*/
int grid[9][9];
int exist_box_CALLS;
int exist_row_CALLS;
int exist_col_CALLS;
int can_assign_CALLS;
int solve_CALLS;
int bacltracking_TIMES;

int exists_in_row(int row, int value){
  exist_row_CALLS++;
  for(int i=0; i<9; i++){
    if(grid[row][i] == value){
      return 1;
    }
  }
  return 0;
}

int exists_in_column(int column, int value){
  exist_col_CALLS++;
  for(int i=0; i<9; i++){
    if(grid[i][column] == value){
      return 1;
    }
  }
  return 0;
}

int exists_in_box(int row, int column, int value){
  exist_box_CALLS++;
  int row_of_box = row - row%3;
  int column_of_box = column - column %3;

  for(int i=row_of_box; i<row_of_box+3; i++){
    for(int j=column_of_box; j<column_of_box+3; j++){
      if(grid[i][j] == value){
        return 1;
      }
    }
  }
  return 0;
}

int can_assign_it(int row, int column, int value){
  can_assign_CALLS++;
  return (!(exists_in_row(row, value))) && (!(exists_in_column(column, value))) && (!(exists_in_box(row, column, value)));
}

int print_sudoku(){
  for(int i=0; i<9; i++){
    for(int j=0; j<9; j++){
      printf("%d - ", grid[i][j]);
    }
    printf("\n");
  }
}

int solve_it(){
  solve_CALLS++;
  for(int i=0; i<9; i++){
    for(int j=0; j<9; j++){
      if(grid[i][j] == 0){
        for(int value=1; value<=9; value++){
          if(can_assign_it(i, j, value) == 1){
            grid[i][j] = value;
            if(solve_it()){
              return 1;
            }
            //else{
              grid[i][j] = 0;
            //}
          }
        }
        bacltracking_TIMES++;
        return 0; /*backtrack */
      }
    }
  }
  return 1;
}




int main(){
  /* A simple test with the given sudoku of the assignment */
  exist_box_CALLS = 0;
  exist_row_CALLS = 0;
  exist_col_CALLS = 0;
  can_assign_CALLS = 0;
  solve_CALLS = 0;
  bacltracking_TIMES = 0;


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

  printf("\n ****CALLS: \n");
  printf("exists_in_row: %d\n", exist_row_CALLS);
  printf("exists_in_column: %d\n", exist_col_CALLS);
  printf("exists_in_box: %d\n", exist_box_CALLS);
  printf("can_assign_it: %d\n", can_assign_CALLS);
  printf("solve_CALLS: %d\n", solve_CALLS);
  printf("BACKTRACKING TIMES: %d\n", bacltracking_TIMES);

  return 0;
}
