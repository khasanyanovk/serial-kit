#include "lexer.hpp"
#include <gtest/gtest.h>

using namespace serialkit;

TEST(LexerTest, EmptySource) {
  Lexer lexer("");
  Token token = lexer.next_token();
  EXPECT_EQ(token.type, TokenType::END_OF_FILE);
}

TEST(LexerTest, Keywords) {
  Lexer lexer("namespace enum model optional repeated packed interned bitmap");

  EXPECT_EQ(lexer.next_token().type, TokenType::NAMESPACE);
  EXPECT_EQ(lexer.next_token().type, TokenType::ENUM);
  EXPECT_EQ(lexer.next_token().type, TokenType::MODEL);
  EXPECT_EQ(lexer.next_token().type, TokenType::OPTIONAL);
  EXPECT_EQ(lexer.next_token().type, TokenType::REPEATED);
  EXPECT_EQ(lexer.next_token().type, TokenType::PACKED);
  EXPECT_EQ(lexer.next_token().type, TokenType::INTERNED);
  EXPECT_EQ(lexer.next_token().type, TokenType::BITMAP);
  EXPECT_EQ(lexer.next_token().type, TokenType::END_OF_FILE);
}

TEST(LexerTest, PrimitiveTypes) {
  Lexer lexer("int32 uint64 float double bool string byte");

  EXPECT_EQ(lexer.next_token().type, TokenType::INT32);
  EXPECT_EQ(lexer.next_token().type, TokenType::UINT64);
  EXPECT_EQ(lexer.next_token().type, TokenType::FLOAT);
  EXPECT_EQ(lexer.next_token().type, TokenType::DOUBLE);
  EXPECT_EQ(lexer.next_token().type, TokenType::BOOL);
  EXPECT_EQ(lexer.next_token().type, TokenType::STRING);
  EXPECT_EQ(lexer.next_token().type, TokenType::BYTE);
}

TEST(LexerTest, Symbols) {
  Lexer lexer("; = { } .");

  EXPECT_EQ(lexer.next_token().type, TokenType::SEMICOLON);
  EXPECT_EQ(lexer.next_token().type, TokenType::EQUALS);
  EXPECT_EQ(lexer.next_token().type, TokenType::LBRACE);
  EXPECT_EQ(lexer.next_token().type, TokenType::RBRACE);
  EXPECT_EQ(lexer.next_token().type, TokenType::DOT);
}

TEST(LexerTest, Identifiers) {
  Lexer lexer("MyModel user_id _private field123");

  Token t1 = lexer.next_token();
  EXPECT_EQ(t1.type, TokenType::IDENTIFIER);
  EXPECT_EQ(t1.value, "MyModel");

  Token t2 = lexer.next_token();
  EXPECT_EQ(t2.type, TokenType::IDENTIFIER);
  EXPECT_EQ(t2.value, "user_id");

  Token t3 = lexer.next_token();
  EXPECT_EQ(t3.type, TokenType::IDENTIFIER);
  EXPECT_EQ(t3.value, "_private");

  Token t4 = lexer.next_token();
  EXPECT_EQ(t4.type, TokenType::IDENTIFIER);
  EXPECT_EQ(t4.value, "field123");
}

TEST(LexerTest, Numbers) {
  Lexer lexer("0 1 42 123 999");

  Token t1 = lexer.next_token();
  EXPECT_EQ(t1.type, TokenType::NUMBER);
  EXPECT_EQ(t1.value, "0");

  Token t2 = lexer.next_token();
  EXPECT_EQ(t2.type, TokenType::NUMBER);
  EXPECT_EQ(t2.value, "1");

  Token t3 = lexer.next_token();
  EXPECT_EQ(t3.type, TokenType::NUMBER);
  EXPECT_EQ(t3.value, "42");
}

TEST(LexerTest, LineComments) {
  Lexer lexer("namespace // this is a comment\nmodel");

  EXPECT_EQ(lexer.next_token().type, TokenType::NAMESPACE);
  EXPECT_EQ(lexer.next_token().type, TokenType::MODEL);
}

TEST(LexerTest, BlockComments) {
  Lexer lexer("namespace /* block comment */ model");

  EXPECT_EQ(lexer.next_token().type, TokenType::NAMESPACE);
  EXPECT_EQ(lexer.next_token().type, TokenType::MODEL);
}

TEST(LexerTest, MultilineBlockComments) {
  Lexer lexer("namespace /* multi\nline\ncomment */ model");

  EXPECT_EQ(lexer.next_token().type, TokenType::NAMESPACE);
  EXPECT_EQ(lexer.next_token().type, TokenType::MODEL);
}

TEST(LexerTest, SourceLocations) {
  Lexer lexer("namespace\nmodel");

  Token t1 = lexer.next_token();
  EXPECT_EQ(t1.location.line, 1);
  EXPECT_EQ(t1.location.column, 1);

  Token t2 = lexer.next_token();
  EXPECT_EQ(t2.location.line, 2);
  EXPECT_EQ(t2.location.column, 1);
}

TEST(LexerTest, RealWorldExample) {
  const char *source = R"(
        namespace examples.basic;
        
        enum Priority {
            LOW = 0;
            HIGH = 1;
        }
        
        model Task {
            string title = 1;
            uint32 id = 2;
            optional bool completed = 3;
        }
    )";

  Lexer lexer(source);

  EXPECT_EQ(lexer.next_token().type, TokenType::NAMESPACE);
  EXPECT_EQ(lexer.next_token().type, TokenType::IDENTIFIER); // examples
  EXPECT_EQ(lexer.next_token().type, TokenType::DOT);
  EXPECT_EQ(lexer.next_token().type, TokenType::IDENTIFIER); // basic
  EXPECT_EQ(lexer.next_token().type, TokenType::SEMICOLON);

  EXPECT_EQ(lexer.next_token().type, TokenType::ENUM);
  EXPECT_EQ(lexer.next_token().type, TokenType::IDENTIFIER); // Priority
  EXPECT_EQ(lexer.next_token().type, TokenType::LBRACE);

  EXPECT_EQ(lexer.next_token().type, TokenType::IDENTIFIER); // LOW
  EXPECT_EQ(lexer.next_token().type, TokenType::EQUALS);
  EXPECT_EQ(lexer.next_token().type, TokenType::NUMBER); // 0
  EXPECT_EQ(lexer.next_token().type, TokenType::SEMICOLON);

  EXPECT_EQ(lexer.next_token().type, TokenType::IDENTIFIER); // HIGH
  EXPECT_EQ(lexer.next_token().type, TokenType::EQUALS);
  EXPECT_EQ(lexer.next_token().type, TokenType::NUMBER); // 1
  EXPECT_EQ(lexer.next_token().type, TokenType::SEMICOLON);

  EXPECT_EQ(lexer.next_token().type, TokenType::RBRACE);

  EXPECT_EQ(lexer.next_token().type, TokenType::MODEL);
  EXPECT_EQ(lexer.next_token().type, TokenType::IDENTIFIER); // Task
  EXPECT_EQ(lexer.next_token().type, TokenType::LBRACE);

  EXPECT_EQ(lexer.next_token().type, TokenType::STRING);
  EXPECT_EQ(lexer.next_token().type, TokenType::IDENTIFIER); // title
  EXPECT_EQ(lexer.next_token().type, TokenType::EQUALS);
  EXPECT_EQ(lexer.next_token().type, TokenType::NUMBER); // 1
  EXPECT_EQ(lexer.next_token().type, TokenType::SEMICOLON);
}

TEST(LexerTest, PeekToken) {
  Lexer lexer("namespace model");

  Token peeked = lexer.peek_token();
  EXPECT_EQ(peeked.type, TokenType::NAMESPACE);

  Token actual = lexer.next_token();
  EXPECT_EQ(actual.type, TokenType::NAMESPACE);

  EXPECT_EQ(lexer.next_token().type, TokenType::MODEL);
}

TEST(LexerTest, ErrorFormatting) {
  Lexer lexer("some code");
  SourceLocation loc(5, 12, 50);

  std::string error = lexer.format_error("Unexpected token", loc);
  EXPECT_NE(error.find("line 5"), std::string::npos);
  EXPECT_NE(error.find("column 12"), std::string::npos);
  EXPECT_NE(error.find("Unexpected token"), std::string::npos);
}
