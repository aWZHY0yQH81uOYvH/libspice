cmake_minimum_required(VERSION 3.23)
project(libspice CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)

# External libraries

find_package(GSL REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(
	./lib
	
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
	
	lib/Parser/Parser.cpp
	lib/Parser/ASTNode.cpp
	lib/Parser/Nodes/ASTComment.cpp
	lib/Parser/Nodes/ASTEquals.cpp
	lib/Parser/Nodes/ASTNumericLiteral.cpp
	lib/Parser/Nodes/ASTIdentifier.cpp
	lib/Parser/Nodes/ASTExpression.cpp
	lib/Parser/Nodes/ASTExprParentheses.cpp
	lib/Parser/Nodes/ASTExprCurlyBraces.cpp
	lib/Parser/Nodes/ASTExprOperator.cpp
	lib/Parser/Nodes/ASTFunction.cpp
	lib/Parser/Nodes/ASTDotInclude.cpp
	lib/Parser/Nodes/ASTDotParam.cpp
	lib/Parser/Nodes/ASTDotFunc.cpp
)

set(LIB_HEADERS
	lib/Core/Expression.hpp
	lib/Core/Circuit.hpp
	lib/Core/Node.hpp
	lib/Core/Component.hpp
	lib/Core/TwoTerminalComponent.hpp
	lib/Core/IntegratingComponent.hpp
	lib/Core/Modulator.hpp
	lib/Core/BuiltinFunctions.hpp
	
	lib/Component/VSource.hpp
	lib/Component/ISource.hpp
	lib/Component/Resistor.hpp
	lib/Component/Capacitor.hpp
	lib/Component/Inductor.hpp
	
	lib/Modulator/PWM.hpp
	lib/Modulator/Sine.hpp
	
	lib/Parser/Parser.hpp
	lib/Parser/ASTNode.hpp
	lib/Parser/Nodes/ASTNewline.hpp
	lib/Parser/Nodes/ASTComment.hpp
	lib/Parser/Nodes/ASTEquals.hpp
	lib/Parser/Nodes/ASTNumericLiteral.hpp
	lib/Parser/Nodes/ASTIdentifier.hpp
	lib/Parser/Nodes/ASTExpression.hpp
	lib/Parser/Nodes/ASTExprParentheses.hpp
	lib/Parser/Nodes/ASTExprCurlyBraces.hpp
	lib/Parser/Nodes/ASTExprOperator.hpp
	lib/Parser/Nodes/ASTFunction.hpp
	lib/Parser/Nodes/ASTDotInclude.hpp
	lib/Parser/Nodes/ASTDotParam.hpp
	lib/Parser/Nodes/ASTDotFunc.hpp
	
	lib/SPICE.hpp
)

# Main library

add_library(spice SHARED
	${LIB_SOURCES}
)

target_sources(spice PUBLIC
	FILE_SET headers
	TYPE HEADERS
	BASE_DIRS lib
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

# Translator program

add_executable(translate
	bin/translate.cpp
)

target_link_libraries(translate spice)
set_target_properties(translate PROPERTIES OUTPUT_NAME libspice-translate)

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
	translate
	
	FILE_SET headers
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libspice
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/libspiceConfig.cmake
	
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libspice
)
