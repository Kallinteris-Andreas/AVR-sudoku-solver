cmake_minimum_required( VERSION 3.21 )
project( avr-test )

set( CMAKE_CXX_STANDARD 17 )
set(
	CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} \
	-mmcu=atmega16 \
	-DF_CPU=10000000 \
	"
)


add_subdirectory( avr-libcpp )
#target_link_libraries( avr-libcpp )
link_libraries( avr-libcpp )

add_executable(main main.cpp)
