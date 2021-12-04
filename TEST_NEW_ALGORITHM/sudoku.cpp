/**\file sudoku.cpp
 * \brief Contains is a class SUDOKO class which includes a very fast solver
 * \target x86-64 and AVR8
 * \author Kallinteris Andreas
 */

//#include <cstdint>
#include <stdint.h>
#include "LUT.hpp"
#include "kallinteris.hpp"
#ifdef AVR
#else
#include <cassert>
#include <iostream>
#include <cstdint>
#endif

static const int8_t empty_cell = 0x00;
static const auto lenght = 9; //indicate the dimensions

class sudoku{
	private:
		uint8_t board[lenght][lenght]; //note should only be filled with 0x1->0x09 & \c empty_cell
		uint8_t solved_cell_counter; //\brief number of cell with a non \c empty_cell value
		volatile bool solving_barrier; // Added by elioudakis on 1 Dec. to support B command

		/**
		 * \brief check which value the {\c x_cord, \c y_cord} cell could have
		 * if cell at {\c x_cord, \c y_cord} is not a \c empty_cell then it it UB
		 *
		 * \author Kallinteris Andreas
		 * \returns an array which indicates if number at index+1 is a possible value at the cell
		 * 	e.g. if the returned[6] is true it means that 7 is a valid value for that cell
		 */
		[[gnu::pure, nodiscard]]
		[[gnu::optimize(3)]]
		kallinteris::bool_array<lenght> possible_values(const int8_t y_cord, const int8_t x_cord){
			if (x_cord < 0 or x_cord > lenght)
				__builtin_unreachable();
			if (y_cord < 0 or y_cord > lenght-1)
				__builtin_unreachable();

			assert(board[y_cord][x_cord] == empty_cell);
			kallinteris::bool_array<lenght> to_return;
			to_return.fill(true);
			
			//invalidate values based on it's horizontal line
			for (auto i = 0; i!=lenght; i++)
				if(board[y_cord][i] != empty_cell)
					to_return.set(board[y_cord][i]-1, false);
			//invalidate values based on it's vertical column
			for (auto i = 0; i!=lenght; i++)
				if(board[i][x_cord] != empty_cell)
					to_return.set(board[i][x_cord]-1, false);
			//invalidate values based on it's square block
			//xy_block_base is the cordinate of the top left part of the block
			auto x_block_base = x_cord/3 * 3;
			auto y_block_base = y_cord/3 * 3;
			for (auto i = 0; i!=3; i++)
				for (auto j = 0; j!=3; j++)
					if (board[y_block_base+j][x_block_base+i] != empty_cell)
						to_return.set(board[y_block_base+j][x_block_base+i]-1, false);

			return to_return;
		}

		/**
		 * \brief checks which value remains
		 * if cell at {last_row, \c x_cord} is not a \c empty_cell then it it UB
		 *
		 * \author Kallinteris Andreas
		 * \returns the an integer indicating which number is missing from last column in \c x_cord
		 */
		[[gnu::pure, nodiscard]]
		[[gnu::optimize(3)]]
		uint8_t last_row_possible_value(const int8_t x_cord){
			if (x_cord < 0 or x_cord > lenght)
				__builtin_unreachable();

			const auto y_cord = lenght-1;
			assert(board[y_cord][x_cord] == empty_cell);
			kallinteris::bool_array<lenght> to_return;
			to_return.fill(true);
			
			//invalidate values based on it's vertical column
			for (auto i = 0; i!=lenght; i++)
				if(board[i][x_cord] != empty_cell)
					to_return.set(board[i][x_cord]-1, false);

			//return valid value
			for (auto i = 0; i!=lenght; i++)
				if (to_return.get(i) == true)
					return i+1;

			assert(false);
			__builtin_unreachable();
		}

		/**
		 * \brief finds a solution for all the rows except the last row
		 * if the board is unsolveable it is UB
		 * must be called with solve(0)
		 * \author Kallinteris Andreas
		 */
		[[gnu::optimize(2)]]
		bool solve_first_8_rows(const int8_t start_index = 0){
			if(solving_barrier == false) // Added by elioudakis on 1 Dec. to support B command
				return false;
			
			if (start_index < 0 or start_index > lenght*lenght)
				__builtin_unreachable();

			for(int8_t i = start_index; i!=lenght*(lenght-1); i++){
				#ifdef AVR
				const uint8_t y_cord = pgm_read_byte(&div_9_LUT[i].first);
				const uint8_t x_cord = pgm_read_byte(&div_9_LUT[i].second);
				#else
				const uint8_t y_cord = i/lenght;
				const uint8_t x_cord = i%lenght;
				#endif
				assert(y_cord == i/lenght and x_cord == i%lenght);

				if (board[y_cord][x_cord] == empty_cell){
					const auto pv = possible_values(y_cord, x_cord);
					for (auto k = 0; k!=lenght; k++){
						if (pv.get(k) == true){
							board[y_cord][x_cord] = k+1;
							solved_cell_counter++;
							if (solve_first_8_rows(i+1))
								return true;
							solved_cell_counter--;
							board[y_cord][x_cord] = empty_cell;
						}
					}
					return false;
				}
			}

			//this place is only reached the board has be solved
			assert(solved_cell_counter != 0);
			assert(solved_cell_counter == lenght*(lenght-1));
			return true;
		}

		void solve_last_row(){
			assert(solved_cell_counter == lenght*(lenght-1));
			const auto y_cord = lenght-1;
			for (auto i = 0; i!=lenght; i++){
				board[y_cord][i] = last_row_possible_value(i);
				solved_cell_counter++;
			}
			assert(solved_cell_counter == lenght*lenght);
		}
	public:
		void set_cell(const int8_t y_cord, const int8_t x_cord, const int8_t value){
			board[y_cord-1][x_cord-1] = value;
			solved_cell_counter++;
		}
		uint8_t get_cell(const int8_t y_cord, const int8_t x_cord){
			return board[y_cord-1][x_cord-1];
		}
		uint8_t get_solved_cell_counter(){
			return solved_cell_counter;
		}
		
		void set_solving_barrier(const bool new_value){ // Added by elioudakis on 1 Dec. to support B command
			solving_barrier = new_value;
		}
		
		bool get_solving_barrier(){ // Added by elioudakis on 1 Dec. to support B command
			return solving_barrier;
		}

		/// \brief set all cells to @c empty_cell
		void clear(){
			for (auto i = 0; i!=lenght; i++)
				for (auto j = 0; j!=lenght; j++)
					board[i][j] = empty_cell;
			solved_cell_counter = 0;
		}

		#ifndef AVR
		void print(){//forked from: https://www.tutorialspoint.com/sudoku-solver-in-cplusplus
			for (int row = 0; row < lenght; row++){
				for (int col = 0; col < lenght; col++){
					if(col == 3 || col == 6)
						std::cout << " | ";
					if (board[row][col] != empty_cell)
						std::cout << (int) board[row][col] <<" ";
					else
						std::cout << "- ";
				}
				if(row == 2 || row == 5){
					std::cout << std::endl;
					for(int i = 0; i<lenght; i++)
						std::cout << "---";
				}
				std::cout << std::endl;
			}
		}
		#endif

		[[gnu::optimize(1)]]
		bool solve(){
			if(solve_first_8_rows() == false)
				return false;
			solve_last_row();
			return true;
		}
};
