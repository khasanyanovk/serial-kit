#pragma once

#include "ast.hpp"
#include "lexer.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace serialkit {

class ParseError : public std::runtime_error {
public:
  SourceLocation location;

  ParseError(const std::string &message, SourceLocation loc)
      : std::runtime_error(message), location(loc) {}
};

class Parser {
public:
  explicit Parser(Lexer &lexer);

  std::unique_ptr<Schema> parse_schema();

private:
  Lexer &lexer_;
  Token current_token_;

  void advance();
  bool match(TokenType type);
  bool check(TokenType type) const;
  Token consume(TokenType type, const std::string &error_message);

  void parse_namespace(Schema &schema);
  std::unique_ptr<Declaration> parse_declaration();
  std::unique_ptr<EnumDecl> parse_enum();
  std::unique_ptr<ModelDecl> parse_model();
  std::unique_ptr<Field> parse_field();
  std::unique_ptr<Type> parse_type();
  uint8_t parse_modifiers();

  bool is_primitive_type(TokenType type) const;
  bool is_field_modifier(TokenType type) const;

  [[noreturn]] void error(const std::string &message);
};

} // namespace serialkit
