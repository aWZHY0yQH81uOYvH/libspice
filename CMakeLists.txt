cmake_minimum_required(VERSION 3.23)
project(libspice CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)

find_package(GSL REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(
	./include
	
	${GSL_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
)

link_directories(
	${GSL_LIBRARY_DIRS}
)

set(LIB_SOURCES
	lib/Core/Circuit.cpp
	lib/Core/Node.cpp
	lib/Core/Component.cpp
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
	include/Core/Circuit.hpp
	include/Core/Node.hpp
	include/Core/Component.hpp
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
	gsl 
	gslcblas
)

target_compile_options(spice PRIVATE -Wall -Wextra)

add_executable(test EXCLUDE_FROM_ALL
	bin/test.cpp
)

target_link_libraries(test spice)

install(TARGETS
	spice
	
	FILE_SET headers
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libspice
)
