#pragma once
#include <cstddef>
#include <cstdint>
#include <array>

/** \brief encodes the operation {input}/10
 * meant to be indexed by {number_of_solved_cells}
 * \author Kallinteris andreas
 */
const
#ifdef AVR
__flash 
#endif
std::array<uint8_t, 81> led_encoding = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf8,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00};


/** \brief Generate std::pair<division, modulo> LUT
 * \author Kallinteris andreas
 */
template <size_t size, auto divisor>
static std::array<std::pair<uint8_t, uint8_t>, size> generate_division_lut(){
	std::array<std::pair<uint8_t, uint8_t>, size> to_return;
	for (auto i = 0; i != size; i++)
		to_return[i] = {i/divisor, i%divisor};
	return to_return;
};

/** \brief encodes the operations {input}/9 and {input}%9
 * meant to be indexed by {number_of_solved_cells}
 * \author Kallinteris andreas
 */
const
#ifdef AVR
__flash 
#endif
std::array<std::pair<uint8_t, uint8_t>, 81> a = generate_division_lut<81, 9>();
