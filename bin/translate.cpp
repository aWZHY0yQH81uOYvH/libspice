/*
	Translator program that can translate normal SPICE files into C++ for use with libspice
*/

#include "Parser/Parser.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstring>

using namespace spice;
using namespace parser;

int main(int argc, const char **argv) {
	
	Parser parser;
	
	// Always look in current directory
	parser.add_include_path(".");
	
	std::vector<std::filesystem::path> file_list, include_paths;
	
	std::filesystem::path output_prefix;
	bool gen_cmake = false;
	bool help = false;
	
	// Handle command line arguments
	for(int argn = 1; argn < argc; argn++) {
		const char *arg = argv[argn];
		
		if(strcmp(arg, "-h") == 0) {
			file_list.emplace_back(); // Suppress missing file warning
			help = true;
		}
		
		else if(strcmp(arg, "--cmake") == 0)
			gen_cmake = true;
		
		else if(strstr(arg, "-o") == arg) {
			if(strlen(arg) > 2)
				output_prefix = arg + 2;
			else if(++argn < argc)
				output_prefix = argv[argn];
			else {
				std::cerr << "Missing output prefix" << std::endl << std::endl;
				help = true;
			}
		}
		
		else if(strstr(arg, "-I") == arg) {
			if(strlen(arg) > 2)
				include_paths.emplace_back(arg + 2);
			else if(++argn < argc)
				include_paths.emplace_back(argv[argn]);
			else {
				std::cerr << "Missing include path" << std::endl << std::endl;
				help = true;
			}
		}
		
		else file_list.emplace_back(arg);
	}
	
	// Require some input files
	if(file_list.size() == 0) {
		std::cerr << "No input files" << std::endl << std::endl;
		help = true;
	}
	
	// Print help
	if(help) {
		std::cerr << "===== libspice translator =====" << std::endl
		          << "Translate standard SPICE files into C++ for use with libspice" << std::endl
		          << "Usage: " << argv[0] << " [-h] [-I include path] [-o output prefix] [--cmake] input files" << std::endl
		          << "\t-h        : print help" << std::endl
		          << "\t-I [path] : specify paths used to search for .included files" << std::endl
		          << "\t-o [path] : specify an output prefix to place .cpp and .hpp files" << std::endl
		          << "\t--cmake   : generate CMake files to compile generated C++" << std::endl
		          << std::endl;
		return 1;
	}
	
	// Add include paths and warn if they don't exist
	for(auto &path:include_paths) {
		try {
			parser.add_include_path(path);
		} catch(const std::filesystem::filesystem_error &fse) {
			std::cerr << "Warning: include path " << path << " does not exist; ignoring" << std::endl;
		}
	}
	
	// Canonicalize paths to avoid looking in include paths for files listed explicitly
	for(auto &file:file_list) {
		try {
			file = std::filesystem::canonical(file);
		} catch(const std::filesystem::filesystem_error &fse) {
			std::cerr << fse.what() << std::endl;
			return 1;
		}
		
		parser.add_file(file);
	}
	
	try {
		// Run parser
		parser.parse();
		
		// Generate ouptut files
		auto generated_files = parser.gen_cpp(output_prefix);
		if(gen_cmake)
			generated_files.push_back(parser.gen_cmake(output_prefix));
		
		for(auto &file:generated_files)
			std::cout << "Wrote " << file << std::endl;
		
	} catch(const std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	
	return 0;
}
