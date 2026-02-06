#include "codegen.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <gtest/gtest.h>

using namespace serialkit;

class CodeGenTest : public ::testing::Test {
protected:
  std::unique_ptr<Schema> parse_schema(const std::string &source) {
    Lexer lexer(source);
    Parser parser(lexer);
    return parser.parse_schema();
  }
};

TEST_F(CodeGenTest, GenerateSimpleEnum) {
  std::string source = R"(
    namespace test;
    
    enum Status {
      OK = 0;
      ERROR = 1;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("namespace test"), std::string::npos);
  EXPECT_NE(header.find("enum class Status"), std::string::npos);
  EXPECT_NE(header.find("OK = 0"), std::string::npos);
  EXPECT_NE(header.find("ERROR = 1"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateSimpleModel) {
  std::string source = R"(
    namespace test;
    
    model User {
      string name = 1;
      int32 age = 2;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("class User"), std::string::npos);
  EXPECT_NE(header.find("std::string name"), std::string::npos);
  EXPECT_NE(header.find("int32_t age"), std::string::npos);
  EXPECT_NE(header.find("std::vector<uint8_t> serialize()"), std::string::npos);
  EXPECT_NE(header.find("bool deserialize"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateOptionalFields) {
  std::string source = R"(
    namespace test;
    
    model Profile {
      optional string bio = 1;
      optional int32 rating = 2;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("std::optional<std::string> bio"), std::string::npos);
  EXPECT_NE(header.find("std::optional<int32_t> rating"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateRepeatedFields) {
  std::string source = R"(
    namespace test;
    
    model List {
      repeated string tags = 1;
      repeated int32 numbers = 2;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("std::vector<std::string> tags"), std::string::npos);
  EXPECT_NE(header.find("std::vector<int32_t> numbers"), std::string::npos);
}

TEST_F(CodeGenTest, GeneratePackedFields) {
  std::string source = R"(
    namespace test;
    
    model Data {
      repeated packed int32 values = 1;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string source_code = codegen.generate_source();

  EXPECT_NE(source_code.find("packed"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateNestedTypes) {
  std::string source = R"(
    namespace test;
    
    model Address {
      string street = 1;
      string city = 2;
    }
    
    model Person {
      string name = 1;
      Address address = 2;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("class Address"), std::string::npos);
  EXPECT_NE(header.find("class Person"), std::string::npos);
  EXPECT_NE(header.find("Address address"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateEnumAndModel) {
  std::string source = R"(
    namespace test;
    
    enum Role {
      ADMIN = 0;
      USER = 1;
    }
    
    model Account {
      string username = 1;
      Role role = 2;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("enum class Role"), std::string::npos);
  EXPECT_NE(header.find("class Account"), std::string::npos);
  EXPECT_NE(header.find("Role role"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateSerializeMethod) {
  std::string source = R"(
    namespace test;
    
    model Simple {
      int32 id = 1;
      string name = 2;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string source_code = codegen.generate_source();

  EXPECT_NE(source_code.find("std::vector<uint8_t> Simple::serialize()"),
            std::string::npos);
  EXPECT_NE(source_code.find("buffer.reserve"), std::string::npos);
  EXPECT_NE(source_code.find("return buffer"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateDeserializeMethod) {
  std::string source = R"(
    namespace test;
    
    model Simple {
      int32 id = 1;
      string name = 2;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string source_code = codegen.generate_source();

  EXPECT_NE(source_code.find("bool Simple::deserialize"), std::string::npos);
  EXPECT_NE(source_code.find("uint32_t field_number"), std::string::npos);
  EXPECT_NE(source_code.find("uint8_t wire_type"), std::string::npos);
  EXPECT_NE(source_code.find("switch (field_number)"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateAllPrimitiveTypes) {
  std::string source = R"(
    namespace test;
    
    model AllTypes {
      int32 field_int32 = 1;
      int64 field_int64 = 2;
      uint32 field_uint32 = 3;
      uint64 field_uint64 = 4;
      float field_float = 5;
      double field_double = 6;
      bool field_bool = 7;
      string field_string = 8;
      byte field_byte = 9;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("int32_t field_int32"), std::string::npos);
  EXPECT_NE(header.find("int64_t field_int64"), std::string::npos);
  EXPECT_NE(header.find("uint32_t field_uint32"), std::string::npos);
  EXPECT_NE(header.find("uint64_t field_uint64"), std::string::npos);
  EXPECT_NE(header.find("float field_float"), std::string::npos);
  EXPECT_NE(header.find("double field_double"), std::string::npos);
  EXPECT_NE(header.find("bool field_bool"), std::string::npos);
  EXPECT_NE(header.find("std::string field_string"), std::string::npos);
  EXPECT_NE(header.find("uint8_t field_byte"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateDefaultValues) {
  std::string source = R"(
    namespace test;
    
    model Defaults {
      int32 number = 1;
      bool flag = 2;
      string text = 3;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("number = 0"), std::string::npos);
  EXPECT_NE(header.find("flag = false"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateIncludeGuards) {
  std::string source = R"(
    namespace test;
    
    model Example {
      int32 value = 1;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("#pragma once"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateRequiredIncludes) {
  std::string source = R"(
    namespace test;
    
    model Example {
      optional string name = 1;
      repeated int32 values = 2;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("#include <cstdint>"), std::string::npos);
  EXPECT_NE(header.find("#include <string>"), std::string::npos);
  EXPECT_NE(header.find("#include <vector>"), std::string::npos);
  EXPECT_NE(header.find("#include <optional>"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateMultipleModels) {
  std::string source = R"(
    namespace test;
    
    model First {
      int32 id = 1;
    }
    
    model Second {
      string name = 1;
    }
    
    model Third {
      bool active = 1;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("class First"), std::string::npos);
  EXPECT_NE(header.find("class Second"), std::string::npos);
  EXPECT_NE(header.find("class Third"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateSourceImplementation) {
  std::string source = R"(
    namespace test;
    
    model Message {
      int32 id = 1;
      string text = 2;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string source_code = codegen.generate_source();

  EXPECT_NE(source_code.find("#include \"test.hpp\""), std::string::npos);
  EXPECT_NE(source_code.find("namespace test"), std::string::npos);
  EXPECT_NE(source_code.find("Message::serialize()"), std::string::npos);
  EXPECT_NE(source_code.find("Message::deserialize"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateRepeatedNestedTypes) {
  std::string source = R"(
    namespace test;
    
    model Item {
      string name = 1;
    }
    
    model Container {
      repeated Item items = 1;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("std::vector<Item> items"), std::string::npos);
}

TEST_F(CodeGenTest, GenerateOptionalNestedTypes) {
  std::string source = R"(
    namespace test;
    
    model Config {
      string value = 1;
    }
    
    model Settings {
      optional Config config = 1;
    }
  )";

  auto schema = parse_schema(source);
  ASSERT_NE(schema, nullptr);

  CodeGenerator codegen(*schema);
  std::string header = codegen.generate_header();

  EXPECT_NE(header.find("std::optional<Config> config"), std::string::npos);
}
