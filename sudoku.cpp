/**\file sudoku.cpp
 * \brief Contains is a class SUDOKO class which includes a very fast solver
 * \target x86-64 and AVR8
 * \author Kallinteris Andreas
 */





#include "kallinteris.hpp"
#ifdef AVR
#else
#include <cassert>
#include <iostream>
#include <cstdint>
#endif

const int8_t empty_cell = 0x00;
const auto lenght = 9; //indicate the dimensions

class sudoku{
	private:
		int8_t board[lenght][lenght]; //note should only be filled with 0x1->0x09 & \c empty_cell
		int8_t solved_cell_counter; //\brief number of cell with a non \c empty_cell value

		/**
		 * \brief check which value the {\c x_cord, \c y_cord} cell could have
		 * if cell at {\c x_cord, \c y_cord} is a \c empty_cell then it it UB
		 *
		 * \author Kallinteris Andreas
		 * \returns an array which which indicates if number at index+1 is a possible value at the cell
		 * 	e.g. if the returned[6] is true it means that 7 is a valid value for that cell
		 */
		[[gnu::pure, nodiscard]]
		kallinteris::array<bool, lenght> possible_values(const int8_t y_cord, const int8_t x_cord){
			if (x_cord < 0 || x_cord > lenght)
				__builtin_unreachable();
			if (y_cord < 0 || y_cord > lenght)
				__builtin_unreachable();

			assert(board[y_cord][x_cord] == empty_cell);
			kallinteris::array<bool, lenght> to_return = {};
			to_return.fill(true);

			//invalidate values based on it's horizontal line
			for (auto i = 0; i!=lenght; i++)
				if(board[y_cord][i] != empty_cell)
					to_return[board[y_cord][i]-1] = false;
			//invalidate values based on it's vertical column
			for (auto i = 0; i!=lenght; i++)
				if(board[i][x_cord] != empty_cell)
					to_return[board[i][x_cord]-1] = false;
			//invalidate values based on it's square block
			//xy_block_base is the cordinate of the top left part of the block
			auto x_block_base = x_cord/3 * 3;
			auto y_block_base = y_cord/3 * 3;
			for (auto i = 0; i!=3; i++)
				for (auto j = 0; j!=3; j++)
					if (board[y_block_base+j][x_block_base+i] != empty_cell)
						to_return[board[y_block_base+j][x_block_base+i]-1] = false;

			return to_return;
		}

		/**
		 * \brief finds a solution for the current board
		 * if the board is unsolveable it is UB
		 * must be called with solve(0)
		 * TODO replace with division and modulo with _Flash LUT
		 * TODO may need to use nimbels instead of bytes
		 * \author Kallinteris Andreas
		 */
		bool solve(const int8_t start_index){
			if (start_index < 0 || start_index > lenght*lenght)
				__builtin_unreachable();

			for(int8_t i = start_index; i!=lenght*lenght; i++)
				if (board[i/lenght][i%lenght] == empty_cell){
					for (auto k = 0; k!=lenght; k++){
						auto pv = possible_values(i/lenght, i%lenght);
						if (pv[k] == true){
							board[i/lenght][i%lenght] = k+1;
							solved_cell_counter++;
							if (solve(i+1))
								return true;
							solved_cell_counter--;
							board[i/lenght][i%lenght] = empty_cell;
						}
					}
					return false;
				}

			//this place is only reached the board has be solved
			assert(solved_cell_counter);
			return true;
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

		/// \brief set all cells to @c empty_cell
		void clear(){
			for (auto i = 0; i!=lenght; i++)
				for (auto j = 0; j!=lenght; j++)
					board[i][j] = empty_cell;
			solved_cell_counter = 0;
		}

		#ifndef AVR
		//forked from: https://www.tutorialspoint.com/sudoku-solver-in-cplusplus
		void print(){
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

		//TODO
		//check if the current board is valid
		[[nodiscard]]
		bool is_board_valid(){
			for (int row = 0; row < lenght; row++){
				for (int col = 0; col < lenght; col++){
				}
			}
			return false;
		}

		[[nodiscard]]
		bool is_board_solved(){
			if (solved_cell_counter != lenght*lenght)
				return false;
			return false;
		}
		#endif

		bool solve(){return solve(0);}
};
