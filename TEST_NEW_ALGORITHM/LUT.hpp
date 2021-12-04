#pragma once
#ifdef AVR
#include <avr/pgmspace.h>   //Added by elioudakis on 2 Dec. To be able to use the flash memory
#else
#include <cstdint>
#include <array>
#include <utility>
#endif
#include "kallinteris.hpp"

/** \brief Generate std::pair<division, modulo> LUT
 * \author Kallinteris andreas
 */
template <int size, int divisor>
static kallinteris::array<kallinteris::pair<uint8_t, uint8_t>, size> const generate_division_lut(){
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
constexpr kallinteris::pair<uint8_t, uint8_t> div_9_LUT[81]
#ifdef AVR
PROGMEM
#endif
= {{'\000', '\000'}, {'\000', '\001'}, {'\000', '\002'}, {'\000', '\003'}, {'\000', '\004'}, {'\000', 
      '\005'}, {'\000', '\006'}, {'\000', '\a'}, {'\000', '\b'}, {'\001', '\000'}, {'\001', '\001'}, {'\001', 
      '\002'}, {'\001', '\003'}, {'\001', '\004'}, {'\001', '\005'}, {'\001', '\006'}, {'\001', '\a'}, {'\001', 
      '\b'}, {'\002', '\000'}, {'\002', '\001'}, {'\002', '\002'}, {'\002', '\003'}, {'\002', '\004'}, {'\002', 
      '\005'}, {'\002', '\006'}, {'\002', '\a'}, {'\002', '\b'}, {'\003', '\000'}, {'\003', '\001'}, {'\003', 
      '\002'}, {'\003', '\003'}, {'\003', '\004'}, {'\003', '\005'}, {'\003', '\006'}, {'\003', '\a'}, {'\003', 
      '\b'}, {'\004', '\000'}, {'\004', '\001'}, {'\004', '\002'}, {'\004', '\003'}, {'\004', '\004'}, {'\004', 
      '\005'}, {'\004', '\006'}, {'\004', '\a'}, {'\004', '\b'}, {'\005', '\000'}, {'\005', '\001'}, {'\005', 
      '\002'}, {'\005', '\003'}, {'\005', '\004'}, {'\005', '\005'}, {'\005', '\006'}, {'\005', '\a'}, {'\005', 
      '\b'}, {'\006', '\000'}, {'\006', '\001'}, {'\006', '\002'}, {'\006', '\003'}, {'\006', '\004'}, {'\006', 
      '\005'}, {'\006', '\006'}, {'\006', '\a'}, {'\006', '\b'}, {'\a', '\000'}, {'\a', '\001'}, {'\a', 
      '\002'}, {'\a', '\003'}, {'\a', '\004'}, {'\a', '\005'}, {'\a', '\006'}, {'\a', '\a'}, {'\a', 
      '\b'}, {'\b', '\000'}, {'\b', '\001'}, {'\b', '\002'}, {'\b', '\003'}, {'\b', '\004'}, {'\b', 
      '\005'}, {'\b', '\006'}, {'\b', '\a'}, {'\b', '\b'}};
