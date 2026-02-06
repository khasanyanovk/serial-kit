#include "lexer.hpp"
#include <cctype>
#include <sstream>
#include <string>
#include <unordered_map>


namespace serialkit {

static const std::unordered_map<std::string_view, TokenType> keywords = {
    {"namespace", TokenType::NAMESPACE}, {"enum", TokenType::ENUM},
    {"model", TokenType::MODEL},         {"optional", TokenType::OPTIONAL},
    {"repeated", TokenType::REPEATED},   {"packed", TokenType::PACKED},
    {"interned", TokenType::INTERNED},   {"bitmap", TokenType::BITMAP},

    {"int8", TokenType::INT8},           {"int16", TokenType::INT16},
    {"int32", TokenType::INT32},         {"int64", TokenType::INT64},
    {"uint8", TokenType::UINT8},         {"uint16", TokenType::UINT16},
    {"uint32", TokenType::UINT32},       {"uint64", TokenType::UINT64},
    {"float", TokenType::FLOAT},         {"double", TokenType::DOUBLE},
    {"bool", TokenType::BOOL},           {"string", TokenType::STRING},
    {"byte", TokenType::BYTE},
};

Lexer::Lexer(std::string_view source)
    : source_(source), position_(0), line_(1), column_(1),
      peeked_token_(std::nullopt) {}

Token Lexer::next_token() {
  if (peeked_token_.has_value()) {
    Token token = std::move(*peeked_token_);
    peeked_token_.reset();
    return token;
  }

  skip_whitespace_and_comments();

  if (is_at_end()) {
    return make_token(TokenType::END_OF_FILE);
  }

  char c = current_char();

  switch (c) {
  case ';':
    advance();
    return make_token(TokenType::SEMICOLON, ";");
  case '=':
    advance();
    return make_token(TokenType::EQUALS, "=");
  case '{':
    advance();
    return make_token(TokenType::LBRACE, "{");
  case '}':
    advance();
    return make_token(TokenType::RBRACE, "}");
  case '.':
    advance();
    return make_token(TokenType::DOT, ".");
  case '-':
    if (peek_char() && is_digit(peek_char())) {
      return read_number();
    }
    advance();
    return make_token(TokenType::INVALID, std::string(1, c));
  }

  if (is_identifier_start(c)) {
    return read_identifier_or_keyword();
  }

  if (is_digit(c)) {
    return read_number();
  }

  advance();
  return make_token(TokenType::INVALID, std::string(1, c));
}

Token Lexer::peek_token() {
  if (!peeked_token_.has_value()) {
    peeked_token_ = next_token();
  }
  return *peeked_token_;
}

bool Lexer::has_more_tokens() const { return !is_at_end(); }

std::string Lexer::format_error(const std::string &message,
                                const SourceLocation &loc) const {
  std::ostringstream oss;
  oss << "Error at line " << loc.line << ", column " << loc.column << ": "
      << message;
  return oss.str();
}

SourceLocation Lexer::current_location() const {
  return SourceLocation(line_, column_, position_);
}

char Lexer::current_char() const {
  if (is_at_end()) {
    return '\0';
  }
  return source_[position_];
}

char Lexer::peek_char(size_t offset) const {
  size_t pos = position_ + offset;
  if (pos >= source_.size()) {
    return '\0';
  }
  return source_[pos];
}

void Lexer::advance() {
  if (is_at_end()) {
    return;
  }

  char c = current_char();
  position_++;

  if (c == '\n') {
    line_++;
    column_ = 1;
  } else {
    column_++;
  }
}

bool Lexer::is_at_end() const { return position_ >= source_.size(); }

void Lexer::skip_whitespace_and_comments() {
  while (!is_at_end()) {
    char c = current_char();

    if (std::isspace(c)) {
      advance();
      continue;
    }

    if (c == '/' && peek_char() == '/') {
      skip_line_comment();
      continue;
    }

    if (c == '/' && peek_char() == '*') {
      skip_block_comment();
      continue;
    }

    break;
  }
}

void Lexer::skip_line_comment() {
  advance();
  advance();

  while (!is_at_end() && current_char() != '\n') {
    advance();
  }
}

void Lexer::skip_block_comment() {
  advance();
  advance();

  while (!is_at_end()) {
    if (current_char() == '*' && peek_char() == '/') {
      advance();
      advance();
      break;
    }
    advance();
  }
}

Token Lexer::read_identifier_or_keyword() {
  SourceLocation start_loc = current_location();
  size_t start = position_;

  while (!is_at_end() && is_identifier_continue(current_char())) {
    advance();
  }

  std::string_view identifier = source_.substr(start, position_ - start);
  TokenType type = keyword_or_identifier(identifier);
  return Token(type, identifier, start_loc);
}

Token Lexer::read_number() {
  SourceLocation start_loc = current_location();
  size_t start = position_;

  if (current_char() == '-') {
    advance();
  }

  while (!is_at_end() && is_digit(current_char())) {
    advance();
  }

  std::string_view number = source_.substr(start, position_ - start);
  return Token(TokenType::NUMBER, number, start_loc);
}

Token Lexer::make_token(TokenType type, std::string_view value) {
  return Token(type, value, current_location());
}

Token Lexer::make_token(TokenType type) {
  return Token(type, current_location());
}

inline bool Lexer::is_identifier_start(char c) const {
  return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

inline bool Lexer::is_identifier_continue(char c) const {
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

inline bool Lexer::is_digit(char c) const {
  return std::isdigit(static_cast<unsigned char>(c));
}

TokenType Lexer::keyword_or_identifier(std::string_view word) const {
  auto it = keywords.find(word);
  if (it != keywords.end()) {
    return it->second;
  }
  return TokenType::IDENTIFIER;
}

const char *token_type_to_string(TokenType type) {
  switch (type) {
  case TokenType::NAMESPACE:
    return "NAMESPACE";
  case TokenType::ENUM:
    return "ENUM";
  case TokenType::MODEL:
    return "MODEL";
  case TokenType::OPTIONAL:
    return "OPTIONAL";
  case TokenType::REPEATED:
    return "REPEATED";
  case TokenType::PACKED:
    return "PACKED";
  case TokenType::INTERNED:
    return "INTERNED";
  case TokenType::BITMAP:
    return "BITMAP";

  case TokenType::INT8:
    return "INT8";
  case TokenType::INT16:
    return "INT16";
  case TokenType::INT32:
    return "INT32";
  case TokenType::INT64:
    return "INT64";
  case TokenType::UINT8:
    return "UINT8";
  case TokenType::UINT16:
    return "UINT16";
  case TokenType::UINT32:
    return "UINT32";
  case TokenType::UINT64:
    return "UINT64";
  case TokenType::FLOAT:
    return "FLOAT";
  case TokenType::DOUBLE:
    return "DOUBLE";
  case TokenType::BOOL:
    return "BOOL";
  case TokenType::STRING:
    return "STRING";
  case TokenType::BYTE:
    return "BYTE";

  case TokenType::SEMICOLON:
    return "SEMICOLON";
  case TokenType::EQUALS:
    return "EQUALS";
  case TokenType::LBRACE:
    return "LBRACE";
  case TokenType::RBRACE:
    return "RBRACE";
  case TokenType::DOT:
    return "DOT";

  case TokenType::IDENTIFIER:
    return "IDENTIFIER";
  case TokenType::NUMBER:
    return "NUMBER";

  case TokenType::END_OF_FILE:
    return "END_OF_FILE";
  case TokenType::INVALID:
    return "INVALID";

  default:
    return "UNKNOWN";
  }
}

} // namespace serialkit
