#include "arg_parser.hpp"
#include <gtest/gtest.h>
#include <sstream>

class ArgParserTest : public ::testing::Test {
protected:
  ArgParser parser;
};

TEST_F(ArgParserTest, AddSimpleFlag) {
  EXPECT_NO_THROW(parser.add_flag('h', "help", "Show help"));
  EXPECT_NO_THROW(parser.add_flag('v', "verbose", "Verbose output"));
}

TEST_F(ArgParserTest, AddOptionWithArgument) {
  EXPECT_NO_THROW(
      parser.add_option('o', "output", "Output file", true, "out.cpp"));
  EXPECT_NO_THROW(
      parser.add_option('i', "input", "Input file", true, "input.skit"));
}

TEST_F(ArgParserTest, DuplicateLongNameThrows) {
  parser.add_flag('h', "help", "Show help");
  EXPECT_THROW(parser.add_flag('x', "help", "Another help"),
               std::invalid_argument);
}

TEST_F(ArgParserTest, DuplicateShortNameThrows) {
  parser.add_flag('h', "help", "Show help");
  EXPECT_THROW(parser.add_flag('h', "hello", "Say hello"),
               std::invalid_argument);
}

TEST_F(ArgParserTest, ParseSimpleFlag) {
  parser.add_flag('h', "help", "Show help");
  char *argv[] = {(char *)"program", (char *)"-h"};
  parser.parse(2, argv);
  EXPECT_TRUE(parser.is_set("help"));
}

TEST_F(ArgParserTest, ParseLongFlag) {
  parser.add_flag('v', "verbose", "Verbose output");
  char *argv[] = {(char *)"program", (char *)"--verbose"};
  parser.parse(2, argv);
  EXPECT_TRUE(parser.is_set("verbose"));
}

TEST_F(ArgParserTest, ParseShortOptionWithValue) {
  parser.add_option('o', "output", "Output file", true, "");
  char *argv[] = {(char *)"program", (char *)"-o", (char *)"result.cpp"};
  parser.parse(3, argv);
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "result.cpp");
}

TEST_F(ArgParserTest, ParseLongOptionWithValue) {
  parser.add_option('o', "output", "Output file", true, "");
  char *argv[] = {(char *)"program", (char *)"--output", (char *)"result.cpp"};
  parser.parse(3, argv);
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "result.cpp");
}

TEST_F(ArgParserTest, ParseLongOptionWithEquals) {
  parser.add_option('o', "output", "Output file", true, "");
  char *argv[] = {(char *)"program", (char *)"--output=result.cpp"};
  parser.parse(2, argv);
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "result.cpp");
}

TEST_F(ArgParserTest, ParseShortOptionWithEquals) {
  parser.add_option('o', "output", "Output file", true, "");
  char *argv[] = {(char *)"program", (char *)"-o=result.cpp"};
  parser.parse(2, argv);
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "result.cpp");
}

TEST_F(ArgParserTest, ParseShortOptionAttached) {
  parser.add_option('o', "output", "Output file", true, "");
  char *argv[] = {(char *)"program", (char *)"-oresult.cpp"};
  parser.parse(2, argv);
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "result.cpp");
}

TEST_F(ArgParserTest, ParseMultipleShortFlags) {
  parser.add_flag('v', "verbose", "Verbose");
  parser.add_flag('d', "debug", "Debug");
  parser.add_flag('q', "quiet", "Quiet");
  char *argv[] = {(char *)"program", (char *)"-vdq"};
  parser.parse(2, argv);
  EXPECT_TRUE(parser.is_set("verbose"));
  EXPECT_TRUE(parser.is_set("debug"));
  EXPECT_TRUE(parser.is_set("quiet"));
}

TEST_F(ArgParserTest, DefaultValue) {
  parser.add_option('o', "output", "Output file", true, "default.cpp");
  char *argv[] = {(char *)"program"};
  parser.parse(1, argv);
  EXPECT_FALSE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "default.cpp");
}

TEST_F(ArgParserTest, OverrideDefaultValue) {
  parser.add_option('o', "output", "Output file", true, "default.cpp");
  char *argv[] = {(char *)"program", (char *)"-o", (char *)"custom"};
  parser.parse(3, argv);
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "custom");
}

TEST_F(ArgParserTest, PositionalArguments) {
  parser.add_flag('v', "verbose", "Verbose");
  char *argv[] = {(char *)"program", (char *)"file1.skit", (char *)"file2.skit",
                  (char *)"-v", (char *)"file3.skit"};
  parser.parse(5, argv);
  const auto &positional = parser.positional();
  EXPECT_EQ(positional.size(), 3);
  EXPECT_EQ(positional[0], "file1.skit");
  EXPECT_EQ(positional[1], "file2.skit");
  EXPECT_EQ(positional[2], "file3.skit");
}

TEST_F(ArgParserTest, DoubleDashSeparator) {
  parser.add_flag('v', "verbose", "Verbose");
  char *argv[] = {(char *)"program", (char *)"-v", (char *)"--",
                  (char *)"-file.txt", (char *)"--another"};
  parser.parse(5, argv);
  EXPECT_TRUE(parser.is_set("verbose"));
  const auto &positional = parser.positional();
  EXPECT_EQ(positional.size(), 2);
  EXPECT_EQ(positional[0], "-file.txt");
  EXPECT_EQ(positional[1], "--another");
}

TEST_F(ArgParserTest, UnknownLongOptionThrows) {
  parser.add_flag('h', "help", "Help");
  char *argv[] = {(char *)"program", (char *)"--unknown"};
  EXPECT_THROW(parser.parse(2, argv), std::runtime_error);
}

TEST_F(ArgParserTest, UnknownShortOptionThrows) {
  parser.add_flag('h', "help", "Help");
  char *argv[] = {(char *)"program", (char *)"-x"};
  EXPECT_THROW(parser.parse(2, argv), std::runtime_error);
}

TEST_F(ArgParserTest, MissingRequiredArgumentThrows) {
  parser.add_option('o', "output", "Output file", true, "");
  char *argv[] = {(char *)"program", (char *)"-o"};
  EXPECT_THROW(parser.parse(2, argv), std::runtime_error);
}

TEST_F(ArgParserTest, FlagWithEqualsThrows) {
  parser.add_flag('v', "verbose", "Verbose");
  char *argv[] = {(char *)"program", (char *)"--verbose=true"};
  EXPECT_THROW(parser.parse(2, argv), std::runtime_error);
}

TEST_F(ArgParserTest, IsSetUnknownOptionThrows) {
  EXPECT_THROW(parser.is_set("unknown"), std::runtime_error);
}

TEST_F(ArgParserTest, ValueOfUnknownOptionThrows) {
  EXPECT_THROW(parser.value_of("unknown"), std::runtime_error);
}

TEST_F(ArgParserTest, ValueOfFlagThrows) {
  parser.add_flag('v', "verbose", "Verbose");
  EXPECT_THROW(parser.value_of("verbose"), std::runtime_error);
}

TEST_F(ArgParserTest, PrintHelp) {
  parser.add_flag('h', "help", "Show help message");
  parser.add_flag('v', "verbose", "Enable verbose output");
  parser.add_option('o', "output", "Output file path", true, "out.cpp");

  std::ostringstream oss;
  parser.print_help(oss);
  std::string help = oss.str();

  EXPECT_NE(help.find("-h"), std::string::npos);
  EXPECT_NE(help.find("--help"), std::string::npos);
  EXPECT_NE(help.find("-v"), std::string::npos);
  EXPECT_NE(help.find("--verbose"), std::string::npos);
  EXPECT_NE(help.find("-o"), std::string::npos);
  EXPECT_NE(help.find("--output"), std::string::npos);
  EXPECT_NE(help.find("default: \"out.cpp\""), std::string::npos);
}

TEST_F(ArgParserTest, ComplexCommand) {
  parser.add_flag('h', "help", "Show help");
  parser.add_flag('v', "verbose", "Verbose");
  parser.add_option('o', "output", "Output", true, "out.cpp");
  parser.add_option('i', "input", "Input", true, "in.skit");
  parser.add_option('n', "namespace", "Namespace", true, "default");

  char *argv[] = {
      (char *)"serialkit",  (char *)"-v",          (char *)"--input=test.skit",
      (char *)"-otest.cpp", (char *)"--namespace", (char *)"myapp",
      (char *)"extra1",     (char *)"extra2"};
  parser.parse(8, argv);

  EXPECT_TRUE(parser.is_set("verbose"));
  EXPECT_TRUE(parser.is_set("input"));
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_TRUE(parser.is_set("namespace"));
  EXPECT_FALSE(parser.is_set("help"));

  EXPECT_EQ(parser.value_of("input"), "test.skit");
  EXPECT_EQ(parser.value_of("output"), "test.cpp");
  EXPECT_EQ(parser.value_of("namespace"), "myapp");

  const auto &positional = parser.positional();
  EXPECT_EQ(positional.size(), 2);
  EXPECT_EQ(positional[0], "extra1");
  EXPECT_EQ(positional[1], "extra2");
}

TEST_F(ArgParserTest, EmptyParse) {
  parser.add_flag('h', "help", "Help");
  char *argv[] = {(char *)"program"};
  EXPECT_NO_THROW(parser.parse(1, argv));
  EXPECT_FALSE(parser.is_set("help"));
  EXPECT_EQ(parser.positional().size(), 0);
}

TEST_F(ArgParserTest, OnlyPositionalArgs) {
  char *argv[] = {(char *)"program", (char *)"file1", (char *)"file2",
                  (char *)"file3"};
  EXPECT_NO_THROW(parser.parse(4, argv));
  const auto &positional = parser.positional();
  EXPECT_EQ(positional.size(), 3);
  EXPECT_EQ(positional[0], "file1");
  EXPECT_EQ(positional[1], "file2");
  EXPECT_EQ(positional[2], "file3");
}

TEST_F(ArgParserTest, OnlyShortName) {
  parser.add_flag('h', "", "Help");
  char *argv[] = {(char *)"program", (char *)"-h"};
  EXPECT_NO_THROW(parser.parse(2, argv));
  EXPECT_TRUE(parser.is_set("h"));
}

TEST_F(ArgParserTest, MultipleParses) {
  parser.add_flag('v', "verbose", "Verbose");
  parser.add_option('o', "output", "Output", true, "");

  char *argv1[] = {(char *)"program", (char *)"-v", (char *)"-o",
                   (char *)"out1.cpp"};
  parser.parse(4, argv1);
  EXPECT_TRUE(parser.is_set("verbose"));
  EXPECT_EQ(parser.value_of("output"), "out1.cpp");

  char *argv2[] = {(char *)"program", (char *)"--output=out2.cpp"};
  parser.parse(2, argv2);
  EXPECT_FALSE(parser.is_set("verbose"));
  EXPECT_EQ(parser.value_of("output"), "out2.cpp");
}

TEST_F(ArgParserTest, ShortFlagsWithOptionAtEnd) {
  parser.add_flag('v', "verbose", "Verbose");
  parser.add_flag('d', "debug", "Debug");
  parser.add_option('o', "output", "Output", true, "");

  char *argv[] = {(char *)"program", (char *)"-vdo", (char *)"file.cpp"};
  parser.parse(3, argv);
  EXPECT_TRUE(parser.is_set("verbose"));
  EXPECT_TRUE(parser.is_set("debug"));
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "file.cpp");
}

TEST_F(ArgParserTest, ShortFlagsWithOptionAttached) {
  parser.add_flag('v', "verbose", "Verbose");
  parser.add_flag('d', "debug", "Debug");
  parser.add_option('o', "output", "Output", true, "");

  char *argv[] = {(char *)"program", (char *)"-vdofile.cpp"};
  parser.parse(2, argv);
  EXPECT_TRUE(parser.is_set("verbose"));
  EXPECT_TRUE(parser.is_set("debug"));
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "file.cpp");
}

TEST_F(ArgParserTest, FilenameOptionShort) {
  parser.add_option('f', "filename", "Filename", true, "");
  char *argv[] = {(char *)"program", (char *)"-f", (char *)"myfile"};
  parser.parse(3, argv);
  EXPECT_TRUE(parser.is_set("filename"));
  EXPECT_EQ(parser.value_of("filename"), "myfile");
}

TEST_F(ArgParserTest, FilenameOptionLong) {
  parser.add_option('f', "filename", "Filename", true, "");
  char *argv[] = {(char *)"program", (char *)"--filename=custom_name"};
  parser.parse(2, argv);
  EXPECT_TRUE(parser.is_set("filename"));
  EXPECT_EQ(parser.value_of("filename"), "custom_name");
}

TEST_F(ArgParserTest, FilenameWithOutputOption) {
  parser.add_option('o', "output", "Output directory", true, ".");
  parser.add_option('f', "filename", "Filename", true, "");
  char *argv[] = {(char *)"program", (char *)"-o", (char *)"gen", (char *)"-f",
                  (char *)"myschema"};
  parser.parse(5, argv);
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_TRUE(parser.is_set("filename"));
  EXPECT_EQ(parser.value_of("output"), "gen");
  EXPECT_EQ(parser.value_of("filename"), "myschema");
}

TEST_F(ArgParserTest, FilenameDefaultEmpty) {
  parser.add_option('f', "filename", "Filename", true, "");
  char *argv[] = {(char *)"program"};
  parser.parse(1, argv);
  EXPECT_FALSE(parser.is_set("filename"));
  EXPECT_EQ(parser.value_of("filename"), "");
}

TEST_F(ArgParserTest, PathWithSpaces) {
  parser.add_option('o', "output", "Output directory", true, ".");
  char *argv[] = {(char *)"program", (char *)"-o",
                  (char *)"E:/folder name/output"};
  parser.parse(3, argv);
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_EQ(parser.value_of("output"), "E:/folder name/output");
}

TEST_F(ArgParserTest, FilenameWithSpaces) {
  parser.add_option('f', "filename", "Filename", true, "");
  char *argv[] = {(char *)"program", (char *)"--filename",
                  (char *)"my file name"};
  parser.parse(3, argv);
  EXPECT_TRUE(parser.is_set("filename"));
  EXPECT_EQ(parser.value_of("filename"), "my file name");
}

TEST_F(ArgParserTest, MultiplePathsWithSpaces) {
  parser.add_option('o', "output", "Output directory", true, ".");
  parser.add_option('f', "filename", "Filename", true, "");
  char *argv[] = {(char *)"program", (char *)"-o",
                  (char *)"C:/Program Files/output", (char *)"--filename",
                  (char *)"my schema file"};
  parser.parse(5, argv);
  EXPECT_TRUE(parser.is_set("output"));
  EXPECT_TRUE(parser.is_set("filename"));
  EXPECT_EQ(parser.value_of("output"), "C:/Program Files/output");
  EXPECT_EQ(parser.value_of("filename"), "my schema file");
}

TEST_F(ArgParserTest, PositionalArgumentWithSpaces) {
  char *argv[] = {(char *)"program",
                  (char *)"E:/My Documents/schema file.skit"};
  parser.parse(2, argv);
  EXPECT_EQ(parser.positional().size(), 1);
  EXPECT_EQ(parser.positional()[0], "E:/My Documents/schema file.skit");
}
