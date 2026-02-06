#include "lexer.hpp"
#include "parser.hpp"
#include "validator.hpp"
#include <gtest/gtest.h>

using namespace serialkit;

class ValidatorTest : public ::testing::Test {
protected:
  std::unique_ptr<Schema> parse_and_validate(const char *source,
                                             Validator &validator) {
    Lexer lexer(source);
    Parser parser(lexer);
    auto schema = parser.parse_schema();
    validator.validate(*schema);
    return schema;
  }
};

TEST_F(ValidatorTest, ValidSchema) {
  const char *source = R"(
    namespace test;
    
    enum Status {
      ACTIVE = 0;
      INACTIVE = 1;
    }
    
    model User {
      string name = 1;
      uint32 id = 2;
      Status status = 3;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  EXPECT_FALSE(validator.get_errors().empty() == false);
}

TEST_F(ValidatorTest, DuplicateDeclarationNames) {
  const char *source = R"(
    namespace test;
    
    enum Status { ACTIVE = 0; }
    model Status { string name = 1; }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("Duplicate"),
            std::string::npos);
}

TEST_F(ValidatorTest, DuplicateFieldNumbers) {
  const char *source = R"(
    namespace test;
    
    model User {
      string name = 1;
      uint32 id = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("Duplicate field number"),
            std::string::npos);
}

TEST_F(ValidatorTest, UnknownType) {
  const char *source = R"(
    namespace test;
    
    model User {
      Priority priority = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("Unknown type"),
            std::string::npos);
}

TEST_F(ValidatorTest, FieldNumberOutOfRange) {
  const char *source = R"(
    namespace test;
    
    model User {
      string name = 0;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("out of valid range"),
            std::string::npos);
}

TEST_F(ValidatorTest, FieldNumberInReservedRange) {
  const char *source = R"(
    namespace test;
    
    model User {
      string name = 19500;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("reserved range"),
            std::string::npos);
}

TEST_F(ValidatorTest, PackedWithoutRepeated) {
  const char *source = R"(
    namespace test;
    
    model Data {
      packed uint32 value = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("packed"),
            std::string::npos);
}

TEST_F(ValidatorTest, PackedWithNonPrimitive) {
  const char *source = R"(
    namespace test;
    
    model Inner { string data = 1; }
    model Outer {
      packed repeated Inner items = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("primitive"),
            std::string::npos);
}

TEST_F(ValidatorTest, InternedWithNonString) {
  const char *source = R"(
    namespace test;
    
    model Data {
      interned uint32 value = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("string"),
            std::string::npos);
}

TEST_F(ValidatorTest, BitmapWithNonBool) {
  const char *source = R"(
    namespace test;
    
    model Data {
      bitmap repeated uint32 flags = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("bool"), std::string::npos);
}

TEST_F(ValidatorTest, BitmapWithoutRepeated) {
  const char *source = R"(
    namespace test;
    
    model Data {
      bitmap bool flag = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("bitmap"),
            std::string::npos);
}

TEST_F(ValidatorTest, OptionalAndRepeated) {
  const char *source = R"(
    namespace test;
    
    model Data {
      optional repeated string values = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("optional"),
            std::string::npos);
}

TEST_F(ValidatorTest, PackedAndBitmap) {
  const char *source = R"(
    namespace test;
    
    model Data {
      packed bitmap repeated bool flags = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("packed"),
            std::string::npos);
}

TEST_F(ValidatorTest, EmptyEnum) {
  const char *source = R"(
    namespace test;
    
    enum Empty { }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("at least one value"),
            std::string::npos);
}

TEST_F(ValidatorTest, EmptyModel) {
  const char *source = R"(
    namespace test;
    
    model Empty { }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("at least one field"),
            std::string::npos);
}

TEST_F(ValidatorTest, DuplicateEnumValueNames) {
  const char *source = R"(
    namespace test;
    
    enum Status {
      ACTIVE = 0;
      ACTIVE = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("Duplicate enum value name"),
            std::string::npos);
}

TEST_F(ValidatorTest, DuplicateEnumValueNumbers) {
  const char *source = R"(
    namespace test;
    
    enum Status {
      ACTIVE = 0;
      RUNNING = 0;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("Duplicate enum value"),
            std::string::npos);
}

TEST_F(ValidatorTest, NegativeEnumValue) {
  const char *source = R"(
    namespace test;
    
    enum Status {
      INVALID = -1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  ASSERT_FALSE(validator.get_errors().empty());
  EXPECT_NE(validator.get_errors()[0].message.find("negative"),
            std::string::npos);
}

TEST_F(ValidatorTest, ValidInternedString) {
  const char *source = R"(
    namespace test;
    
    model Log {
      interned string level = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  EXPECT_TRUE(validator.get_errors().empty());
}

TEST_F(ValidatorTest, ValidPackedRepeated) {
  const char *source = R"(
    namespace test;
    
    model Data {
      packed repeated uint32 values = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  EXPECT_TRUE(validator.get_errors().empty());
}

TEST_F(ValidatorTest, ValidBitmapRepeated) {
  const char *source = R"(
    namespace test;
    
    model Flags {
      bitmap repeated bool features = 1;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  EXPECT_TRUE(validator.get_errors().empty());
}

TEST_F(ValidatorTest, ComplexValidSchema) {
  const char *source = R"(
    namespace examples.test;
    
    enum Priority {
      LOW = 0;
      MEDIUM = 1;
      HIGH = 2;
    }
    
    model Task {
      string title = 1;
      uint32 id = 2;
      bool completed = 3;
      Priority priority = 4;
      optional uint64 completed_at = 5;
      repeated string tags = 6;
      packed repeated uint32 metrics = 7;
      interned string category = 8;
    }
    
    model TaskList {
      string name = 1;
      repeated Task tasks = 2;
    }
  )";

  Validator validator;
  parse_and_validate(source, validator);

  EXPECT_TRUE(validator.get_errors().empty());
}
