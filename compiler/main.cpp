#include "arg_parser.hpp"
#include "codegen.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "validator.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

void register_options(ArgParser &parser);
std::string read_file(const std::string &path);
void write_file(const std::string &path, const std::string &content);
int compile_schema(const std::string &input_file, const std::string &output_dir,
                   const std::string &filename, bool verbose);

int main(int argc, char **argv) {
  ArgParser parser;
  register_options(parser);

  try {
    parser.parse(argc, argv);

    if (parser.is_set("help")) {
      std::cout << "SerialKit Compiler - Generate C++ code from .skit schema "
                   "files\n\n";
      std::cout << "Usage: serialkit [options] <input.skit>\n\n";
      parser.print_help(std::cout);
      std::cout << "\nExample:\n";
      std::cout << "  serialkit -o output/ input.skit\n";
      std::cout << "  serialkit --verbose --output=gen/ schema.skit\n";
      return 0;
    }

    if (parser.is_set("version")) {
      std::cout << "SerialKit Compiler version 1.0.0\n";
      return 0;
    }

    const auto &positional = parser.positional();
    if (positional.empty()) {
      std::cerr << "Error: No input file specified\n";
      std::cerr << "Use --help for usage information\n";
      return 1;
    }

    std::string input_file = positional[0];
    std::string output_dir = parser.value_of("output");
    std::string filename = parser.value_of("filename");
    bool verbose = parser.is_set("verbose");

    return compile_schema(input_file, output_dir, filename, verbose);

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}

void register_options(ArgParser &parser) {
  parser.add_flag('h', "help", "Show this help message");
  parser.add_flag('v', "verbose", "Enable verbose output");
  parser.add_flag(0, "version", "Show version information");
  parser.add_option('o', "output", "Output directory for generated files", true,
                    ".");
  parser.add_option('f', "filename",
                    "Base filename for generated files (without extension)",
                    true, "");
}

std::string read_file(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + path);
  }

  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

void write_file(const std::string &path, const std::string &content) {
  std::ofstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to create file: " + path);
  }
  file << content;
}

int compile_schema(const std::string &input_file, const std::string &output_dir,
                   const std::string &filename, bool verbose) {
  if (verbose) {
    std::cout << "Reading input file: " << input_file << "\n";
  }

  std::string source = read_file(input_file);

  if (verbose) {
    std::cout << "Tokenizing...\n";
  }
  serialkit::Lexer lexer(source);

  if (verbose) {
    std::cout << "Parsing...\n";
  }
  serialkit::Parser parser(lexer);
  auto schema = parser.parse_schema();

  if (!schema) {
    std::cerr << "Error: Failed to parse schema\n";
    return 1;
  }

  if (verbose) {
    std::cout << "Validating...\n";
  }
  serialkit::Validator validator;

  if (!validator.validate(*schema)) {
    std::cerr << "Validation errors:\n";
    for (const auto &error : validator.get_errors()) {
      std::cerr << "  [" << error.location.line << ":" << error.location.column
                << "] " << error.message << "\n";
    }
    return 1;
  }

  if (verbose) {
    std::cout << "Generating code...\n";
  }
  serialkit::CodeGenerator codegen(*schema);

  std::string header_content = codegen.generate_header();
  std::string source_content = codegen.generate_source();

  std::string base_name;
  if (!filename.empty()) {
    base_name = filename;
  } else {
    base_name = schema->namespace_name;
  }

  std::string header_file = output_dir + "/" + base_name + ".hpp";
  std::string source_file = output_dir + "/" + base_name + ".cpp";

  std::error_code ec;
  if (!std::filesystem::exists(output_dir, ec)) {
    if (verbose) {
      std::cout << "Creating output directory: " << output_dir << "\n";
    }
    if (!std::filesystem::create_directories(output_dir, ec) && ec) {
      std::cerr << "Error: Cannot create output directory '" << output_dir
                << "': " << ec.message() << "\n";
      return 1;
    }
  } else if (ec) {
    std::cerr << "Error: Cannot check existence of output directory '"
              << output_dir << "': " << ec.message() << "\n";
    return 1;
  }

  if (verbose) {
    std::cout << "Writing header: " << header_file << "\n";
  }
  write_file(header_file, header_content);

  if (verbose) {
    std::cout << "Writing source: " << source_file << "\n";
  }
  write_file(source_file, source_content);

  std::cout << "Successfully generated:\n";
  std::cout << "  " << header_file << "\n";
  std::cout << "  " << source_file << "\n";

  return 0;
}