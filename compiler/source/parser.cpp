#include "parser.hpp"
#include <sstream>

namespace serialkit {

Parser::Parser(Lexer &lexer)
    : lexer_(lexer), current_token_(lexer_.next_token()) {}

std::unique_ptr<Schema> Parser::parse_schema() {
  auto schema = std::make_unique<Schema>(current_token_.location);

  parse_namespace(*schema);

  while (!check(TokenType::END_OF_FILE)) {
    schema->declarations.push_back(parse_declaration());
  }

  return schema;
}

void Parser::advance() { current_token_ = lexer_.next_token(); }

bool Parser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::check(TokenType type) const { return current_token_.type == type; }

Token Parser::consume(TokenType type, const std::string &error_message) {
  if (!check(type)) {
    error(error_message);
  }
  Token token = current_token_;
  advance();
  return token;
}

void Parser::parse_namespace(Schema &schema) {
  consume(TokenType::NAMESPACE,
          "Expected 'namespace' at the beginning of file");

  std::string ns_name;
  ns_name.reserve(64);
  ns_name = std::string(
      consume(TokenType::IDENTIFIER, "Expected namespace name").value);

  while (match(TokenType::DOT)) {
    ns_name += ".";
    ns_name += std::string(
        consume(TokenType::IDENTIFIER, "Expected identifier after '.'").value);
  }

  consume(TokenType::SEMICOLON, "Expected ';' after namespace declaration");

  schema.namespace_name = std::move(ns_name);
}

std::unique_ptr<Declaration> Parser::parse_declaration() {
  if (check(TokenType::ENUM)) {
    return parse_enum();
  } else if (check(TokenType::MODEL)) {
    return parse_model();
  } else {
    error("Expected 'enum' or 'model' declaration");
  }
}

std::unique_ptr<EnumDecl> Parser::parse_enum() {
  SourceLocation loc = current_token_.location;
  consume(TokenType::ENUM, "Expected 'enum'");

  std::string name =
      std::string(consume(TokenType::IDENTIFIER, "Expected enum name").value);
  auto enum_decl = std::make_unique<EnumDecl>(std::move(name), loc);
  enum_decl->values.reserve(8);

  consume(TokenType::LBRACE, "Expected '{' after enum name");

  while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
    SourceLocation value_loc = current_token_.location;
    std::string value_name = std::string(
        consume(TokenType::IDENTIFIER, "Expected enum value name").value);

    consume(TokenType::EQUALS, "Expected '=' after enum value name");

    Token number_token =
        consume(TokenType::NUMBER, "Expected number after '='");
    int value = std::stoi(std::string(number_token.value));

    consume(TokenType::SEMICOLON, "Expected ';' after enum value");

    enum_decl->values.push_back(
        std::make_unique<EnumValue>(std::move(value_name), value, value_loc));
  }

  consume(TokenType::RBRACE, "Expected '}' after enum body");

  return enum_decl;
}

std::unique_ptr<ModelDecl> Parser::parse_model() {
  SourceLocation loc = current_token_.location;
  consume(TokenType::MODEL, "Expected 'model'");

  std::string name =
      std::string(consume(TokenType::IDENTIFIER, "Expected model name").value);
  auto model_decl = std::make_unique<ModelDecl>(std::move(name), loc);
  model_decl->fields.reserve(16);

  consume(TokenType::LBRACE, "Expected '{' after model name");

  while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
    model_decl->fields.push_back(parse_field());
  }

  consume(TokenType::RBRACE, "Expected '}' after model body");

  return model_decl;
}

std::unique_ptr<Field> Parser::parse_field() {
  SourceLocation loc = current_token_.location;

  uint8_t modifiers = parse_modifiers();
  auto type = parse_type();

  std::string field_name =
      std::string(consume(TokenType::IDENTIFIER, "Expected field name").value);

  consume(TokenType::EQUALS, "Expected '=' after field name");

  Token number_token = consume(TokenType::NUMBER, "Expected field number");
  int field_number = std::stoi(std::string(number_token.value));

  consume(TokenType::SEMICOLON, "Expected ';' after field declaration");

  auto field = std::make_unique<Field>(std::move(type), std::move(field_name),
                                       field_number, loc);
  field->modifiers = modifiers;

  return field;
}

std::unique_ptr<Type> Parser::parse_type() {
  SourceLocation loc = current_token_.location;

  if (is_primitive_type(current_token_.type)) {
    PrimitiveTypeKind kind = token_to_primitive_type(current_token_.type);
    advance();
    return std::make_unique<PrimitiveType>(kind, loc);
  } else if (check(TokenType::IDENTIFIER)) {
    std::string name = std::string(current_token_.value);
    advance();
    return std::make_unique<UserType>(std::move(name), loc);
  } else {
    error("Expected type name");
  }
}

uint8_t Parser::parse_modifiers() {
  uint8_t modifiers = MOD_NONE;

  while (is_field_modifier(current_token_.type)) {
    switch (current_token_.type) {
    case TokenType::OPTIONAL:
      modifiers |= MOD_OPTIONAL;
      break;
    case TokenType::REPEATED:
      modifiers |= MOD_REPEATED;
      break;
    case TokenType::PACKED:
      modifiers |= MOD_PACKED;
      break;
    case TokenType::INTERNED:
      modifiers |= MOD_INTERNED;
      break;
    case TokenType::BITMAP:
      modifiers |= MOD_BITMAP;
      break;
    default:
      break;
    }
    advance();
  }

  return modifiers;
}

bool Parser::is_primitive_type(TokenType type) const {
  switch (type) {
  case TokenType::INT8:
  case TokenType::INT16:
  case TokenType::INT32:
  case TokenType::INT64:
  case TokenType::UINT8:
  case TokenType::UINT16:
  case TokenType::UINT32:
  case TokenType::UINT64:
  case TokenType::FLOAT:
  case TokenType::DOUBLE:
  case TokenType::BOOL:
  case TokenType::STRING:
  case TokenType::BYTE:
    return true;
  default:
    return false;
  }
}

bool Parser::is_field_modifier(TokenType type) const {
  switch (type) {
  case TokenType::OPTIONAL:
  case TokenType::REPEATED:
  case TokenType::PACKED:
  case TokenType::INTERNED:
  case TokenType::BITMAP:
    return true;
  default:
    return false;
  }
}

void Parser::error(const std::string &message) {
  std::ostringstream oss;
  oss << "Parse error at line " << current_token_.location.line << ", column "
      << current_token_.location.column << ": " << message;
  throw ParseError(oss.str(), current_token_.location);
}

} // namespace serialkit
