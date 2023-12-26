cmake_minimum_required(VERSION 3.23)
project(libspice CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)

# External libraries

find_package(GSL REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(
	./include
	
	${GSL_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIRS}
)

link_directories(
	${GSL_LIBRARY_DIRS}
)

# Source files

set(LIB_SOURCES
	lib/Core/Expression.cpp
	lib/Core/Circuit.cpp
	lib/Core/Node.cpp
	lib/Core/Component.cpp
	lib/Core/TwoTerminalComponent.cpp
	lib/Core/IntegratingComponent.cpp
	lib/Core/Modulator.cpp
	
	lib/Component/VSource.cpp
	lib/Component/ISource.cpp
	lib/Component/Resistor.cpp
	lib/Component/Capacitor.cpp
	lib/Component/Inductor.cpp
	
	lib/Modulator/PWM.cpp
	lib/Modulator/Sine.cpp
)

set(LIB_HEADERS
	include/Core/Expression.hpp
	include/Core/Circuit.hpp
	include/Core/Node.hpp
	include/Core/Component.hpp
	include/Core/TwoTerminalComponent.hpp
	include/Core/IntegratingComponent.hpp
	include/Core/Modulator.hpp
	
	include/Component/VSource.hpp
	include/Component/ISource.hpp
	include/Component/Resistor.hpp
	include/Component/Capacitor.hpp
	include/Component/Inductor.hpp
	
	include/Modulator/PWM.hpp
	include/Modulator/Sine.hpp
	
	include/SPICE.hpp
)

# Main library

add_library(spice SHARED
	${LIB_SOURCES}
)

target_sources(spice PUBLIC
	FILE_SET headers
	TYPE HEADERS
	BASE_DIRS include
	FILES "${LIB_HEADERS}"
)

target_link_libraries(spice
	${GSL_LIBRARIES}
)

target_compile_options(spice PRIVATE -Wall -Wextra)

# Test program

add_executable(test EXCLUDE_FROM_ALL
	bin/test.cpp
)

target_link_libraries(test spice)

# Generate CMake config file

set(libspice_INCLUDE_DIRS
	${GSL_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIRS}
	${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/libspice
)

set(libspice_LIBRARY_DIRS
	${GSL_LIBRARY_DIRS}
	${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

set(libspice_LIBRARIES
	${GSL_LIBRARIES}
	spice
)

configure_file(libspiceConfig.cmake.in libspiceConfig.cmake @ONLY)

# Installation

install(TARGETS
	spice
	
	FILE_SET headers
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libspice
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/libspiceConfig.cmake
	
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libspice
)
