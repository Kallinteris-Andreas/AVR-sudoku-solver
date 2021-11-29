#pragma once
#ifndef AVR
#include <cstdint>
#endif
#include "kallinteris.hpp"

/** \brief Generate std::pair<division, modulo> LUT
 * \author Kallinteris andreas
 */
template <int size, int divisor>
static kallinteris::array<kallinteris::pair<uint8_t, uint8_t>, size> generate_division_lut(){
	kallinteris::array<kallinteris::pair<uint8_t, uint8_t>, size> to_return;
	for (auto i = 0; i != size; i++){
		to_return[i].first = i/divisor;
		to_return[i].second = i%divisor;
		//to_return[i] = {i/divisor, i%divisor};
	}
	return to_return;
};

/** \brief encodes the operations {input}/9 and {input}%9
 * meant to be indexed by {number_of_solved_cells}
 * \author Kallinteris andreas
 */
const
#ifdef AVR
//__flash
#endif
kallinteris::array<kallinteris::pair<uint8_t, uint8_t>, 81> a = generate_division_lut<81, 9>();
