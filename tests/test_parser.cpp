#include "lexer.hpp"
#include "parser.hpp"
#include <gtest/gtest.h>

using namespace serialkit;

TEST(ParserTest, ParseNamespace) {
  const char *source = "namespace examples.basic;";
  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();

  ASSERT_NE(schema, nullptr);
  EXPECT_EQ(schema->namespace_name, "examples.basic");
}

TEST(ParserTest, ParseSimpleEnum) {
  const char *source = R"(
        namespace test;
        
        enum Status {
            INACTIVE = 0;
            ACTIVE = 1;
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();

  ASSERT_EQ(schema->declarations.size(), 1);

  auto *enum_decl = dynamic_cast<EnumDecl *>(schema->declarations[0].get());
  ASSERT_NE(enum_decl, nullptr);
  EXPECT_EQ(enum_decl->name, "Status");
  ASSERT_EQ(enum_decl->values.size(), 2);
  EXPECT_EQ(enum_decl->values[0]->name, "INACTIVE");
  EXPECT_EQ(enum_decl->values[0]->value, 0);
  EXPECT_EQ(enum_decl->values[1]->name, "ACTIVE");
  EXPECT_EQ(enum_decl->values[1]->value, 1);
}

TEST(ParserTest, ParseSimpleModel) {
  const char *source = R"(
        namespace test;
        
        model User {
            string username = 1;
            uint32 id = 2;
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();

  ASSERT_EQ(schema->declarations.size(), 1);

  auto *model_decl = dynamic_cast<ModelDecl *>(schema->declarations[0].get());
  ASSERT_NE(model_decl, nullptr);
  EXPECT_EQ(model_decl->name, "User");
  ASSERT_EQ(model_decl->fields.size(), 2);

  EXPECT_EQ(model_decl->fields[0]->name, "username");
  EXPECT_EQ(model_decl->fields[0]->number, 1);
  EXPECT_TRUE(model_decl->fields[0]->type->is_primitive());

  EXPECT_EQ(model_decl->fields[1]->name, "id");
  EXPECT_EQ(model_decl->fields[1]->number, 2);
}

TEST(ParserTest, ParseOptionalField) {
  const char *source = R"(
        namespace test;
        
        model User {
            optional string email = 1;
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();
  auto *model = dynamic_cast<ModelDecl *>(schema->declarations[0].get());

  ASSERT_NE(model, nullptr);
  ASSERT_EQ(model->fields.size(), 1);
  EXPECT_TRUE(model->fields[0]->is_optional());
}

TEST(ParserTest, ParseRepeatedField) {
  const char *source = R"(
        namespace test;
        
        model User {
            repeated string tags = 1;
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();
  auto *model = dynamic_cast<ModelDecl *>(schema->declarations[0].get());

  ASSERT_NE(model, nullptr);
  ASSERT_EQ(model->fields.size(), 1);
  EXPECT_TRUE(model->fields[0]->is_repeated());
}

TEST(ParserTest, ParsePackedRepeatedField) {
  const char *source = R"(
        namespace test;
        
        model Data {
            packed repeated uint32 values = 1;
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();
  auto *model = dynamic_cast<ModelDecl *>(schema->declarations[0].get());

  ASSERT_NE(model, nullptr);
  ASSERT_EQ(model->fields.size(), 1);
  EXPECT_TRUE(model->fields[0]->is_packed());
  EXPECT_TRUE(model->fields[0]->is_repeated());
}

TEST(ParserTest, ParseInternedString) {
  const char *source = R"(
        namespace test;
        
        model Log {
            interned string level = 1;
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();
  auto *model = dynamic_cast<ModelDecl *>(schema->declarations[0].get());

  ASSERT_NE(model, nullptr);
  ASSERT_EQ(model->fields.size(), 1);
  EXPECT_TRUE(model->fields[0]->is_interned());
}

TEST(ParserTest, ParseBitmapBool) {
  const char *source = R"(
        namespace test;
        
        model Flags {
            bitmap repeated bool features = 1;
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();
  auto *model = dynamic_cast<ModelDecl *>(schema->declarations[0].get());

  ASSERT_NE(model, nullptr);
  ASSERT_EQ(model->fields.size(), 1);
  EXPECT_TRUE(model->fields[0]->is_bitmap());
  EXPECT_TRUE(model->fields[0]->is_repeated());
}

TEST(ParserTest, ParseUserDefinedType) {
  const char *source = R"(
        namespace test;
        
        model Task {
            Priority priority = 1;
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();
  auto *model = dynamic_cast<ModelDecl *>(schema->declarations[0].get());

  ASSERT_NE(model, nullptr);
  ASSERT_EQ(model->fields.size(), 1);

  auto *user_type = dynamic_cast<UserType *>(model->fields[0]->type.get());
  ASSERT_NE(user_type, nullptr);
  EXPECT_EQ(user_type->name, "Priority");
  EXPECT_FALSE(user_type->is_primitive());
}

TEST(ParserTest, ParseMultipleDeclarations) {
  const char *source = R"(
        namespace test;
        
        enum Status {
            ACTIVE = 0;
            INACTIVE = 1;
        }
        
        model User {
            string name = 1;
            Status status = 2;
        }
        
        model Task {
            string title = 1;
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();

  EXPECT_EQ(schema->namespace_name, "test");
  ASSERT_EQ(schema->declarations.size(), 3);

  EXPECT_NE(dynamic_cast<EnumDecl *>(schema->declarations[0].get()), nullptr);
  EXPECT_NE(dynamic_cast<ModelDecl *>(schema->declarations[1].get()), nullptr);
  EXPECT_NE(dynamic_cast<ModelDecl *>(schema->declarations[2].get()), nullptr);
}

TEST(ParserTest, ParseRealWorldExample) {
  const char *source = R"(
        namespace examples.basic;
        
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
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  auto schema = parser.parse_schema();

  EXPECT_EQ(schema->namespace_name, "examples.basic");
  ASSERT_EQ(schema->declarations.size(), 2);

  auto *enum_decl = dynamic_cast<EnumDecl *>(schema->declarations[0].get());
  ASSERT_NE(enum_decl, nullptr);
  EXPECT_EQ(enum_decl->name, "Priority");
  EXPECT_EQ(enum_decl->values.size(), 3);

  auto *model_decl = dynamic_cast<ModelDecl *>(schema->declarations[1].get());
  ASSERT_NE(model_decl, nullptr);
  EXPECT_EQ(model_decl->name, "Task");
  EXPECT_EQ(model_decl->fields.size(), 5);
}

TEST(ParserTest, ErrorOnMissingNamespace) {
  const char *source = "model User { }";

  Lexer lexer(source);
  Parser parser(lexer);

  EXPECT_THROW({ parser.parse_schema(); }, ParseError);
}

TEST(ParserTest, ErrorOnMissingSemicolon) {
  const char *source = R"(
        namespace test;
        
        model User {
            string name = 1
        }
    )";

  Lexer lexer(source);
  Parser parser(lexer);

  EXPECT_THROW({ parser.parse_schema(); }, ParseError);
}

TEST(ParserTest, FindEnum) {
  const char *source = R"(
        namespace test;
        
        enum Status { ACTIVE = 0; }
        model User { string name = 1; }
    )";

  Lexer lexer(source);
  Parser parser(lexer);
  auto schema = parser.parse_schema();

  auto *found_enum = schema->find_enum("Status");
  ASSERT_NE(found_enum, nullptr);
  EXPECT_EQ(found_enum->name, "Status");

  EXPECT_EQ(schema->find_enum("NotFound"), nullptr);
}

TEST(ParserTest, FindModel) {
  const char *source = R"(
        namespace test;
        
        enum Status { ACTIVE = 0; }
        model User { string name = 1; }
    )";

  Lexer lexer(source);
  Parser parser(lexer);
  auto schema = parser.parse_schema();

  auto *found_model = schema->find_model("User");
  ASSERT_NE(found_model, nullptr);
  EXPECT_EQ(found_model->name, "User");

  EXPECT_EQ(schema->find_model("NotFound"), nullptr);
}
