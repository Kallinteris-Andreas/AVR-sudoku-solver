/** \file kallinteris.cpp
 * \brief a collection of utility classes and functions
 * This is library is meant to be used when you do not have access to libcpp STL
 * \author Kallinteris Andreas
 */
#pragma once
#ifndef AVR
#include <cassert>
#else
static void assert([[maybe_unused]] bool a){};
#endif



namespace kallinteris{
	enum class byte : unsigned char {} ;
	// \brief a std::array replacement
	template<typename T, int length>
	class array{
		private:
			T arr[length];
		public:
			constexpr array(){};
			
			constexpr int size() const {return length;}

			constexpr T& operator[](const int n){
				return arr[n];
			};
			const T& operator[](const int n) const {
				return arr[n];
			};
			
			constexpr void fill(const T& value){
				for (auto i = 0; i != this->size(); i++)
					arr[i] = value;
			}

			constexpr T get(const int index){return arr[index];}
			constexpr void set(const int index, const int value){arr[index] = value;}
	};

	// \brief and array of int4_t
	template<int length>
	class nimble_array{
		private:
			unsigned char arr[(length+1)/2];
		public:
			constexpr nimble_array(){};
			
			constexpr int size() const {return length;}
			constexpr char get(const int index){
				if ((index%2) == 0)
					return arr[index/2] & 0x0F;
				else if ((index%2) == 1)
					return arr[index/2] >> 4;
				assert(false);
				//__builtin_unreachable();
			}
			constexpr char get(const int index) const{
				if ((index%2) == 0)
					return arr[index/2] & 0x0F;
				else if ((index%2) == 1)
					return arr[index/2] >> 4;
				assert(false);
				__builtin_unreachable();//TODO make sure
			}
			constexpr void set(const int index, const int value){
				if (value > 0x0F)
					__builtin_unreachable();

				if (index%2 == 0)
					arr[index/2] = (arr[index/2]&0xF0) | value;
				else if (index%2 == 1)
					arr[index/2] = (arr[index/2]&0x0F) | (value << 4);
			}

			constexpr void fill(const unsigned char& value){
				if (value > 0x0F)
					__builtin_unreachable();
				const unsigned char nv = value + (value << 4);
				for (auto i = 0; i != length; i++)
					arr[i] = nv;
				//for (auto i = 0; i != this->size(); i++)
					//this->set(i, value);
			}
	};

	// \brief and array of bool
	template<int length>
	class bool_array{
		private:
			unsigned char arr[(length+7)/8];
		public:
			constexpr bool_array() = default;
			
			constexpr int size() const {return length;}
			constexpr bool get(const int index){
				return arr[index/8] & (1 << index%8);
			}
			constexpr bool get(const int index) const{
				return arr[index/8] & (1 << index%8);
			}
			constexpr void set(const int index, const bool value){
				arr[index/8] = arr[index/8] & ~(1 << index%8);//remove the old value
				arr[index/8] = arr[index/8] | (value << index%8);
			}

			constexpr void fill(const unsigned char& value){
				if (value == false)
					for (auto i = 0; i != (length+7)/8; i++)
						arr[i] = 0x00;
				else if (value == true)
					for (auto i = 0; i != (length+7)/8; i++)
						arr[i] = 0xFF;
			}
	};



	// \brief a re implemention of std::pair
	template<typename _T1, typename _T2>
	struct pair
	{
		typedef _T1 first_type;    ///< The type of the `first` member
		typedef _T2 second_type;   ///< The type of the `second` member
	
		_T1 first;                 ///< The first member
		_T2 second;                ///< The second member
	};
}


//#define KALLINTERIS_TESTING
#ifdef AVR
#undef KALLINTERIS_TESTING
#endif
#ifdef KALLINTERIS_TESTING
#include <cassert>
#include <cstdlib>
int main(){
	{
		kallinteris::array<int, 5> a;
		a[0] = 1;
		a[1] = 2;
		a[2] = 3;
		a[3] = 4;
		a[4] = 5;
		assert(a[0] == 1);
		assert(a[1] == 2);
		assert(a[2] == 3);
		assert(a[3] == 4);
		assert(a[4] == 5);
		const kallinteris::array<int, 5> b = a;
		assert(b[0] == 1);
		assert(b[1] == 2);
		assert(b[2] == 3);
		assert(b[3] == 4);
		assert(b[4] == 5);
	}
	{
		kallinteris::nimble_array<5> a;
		a.set(0, 1);
		a.set(1, 2);
		a.set(2, 3);
		a.set(3, 4);
		a.set(4, 5);
		assert(a.get(0) == 1);
		assert(a.get(1) == 2);
		assert(a.get(2) == 3);
		assert(a.get(3) == 4);
		assert(a.get(4) == 5);
		const kallinteris::nimble_array<5> b = a;
		assert(b.get(0) == 1);
		assert(b.get(1) == 2);
		assert(b.get(2) == 3);
		assert(b.get(3) == 4);
		assert(b.get(4) == 5);
	}
}
#undef KALLINTERIS_TESTING
#endif
