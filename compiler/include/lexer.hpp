#ifndef _LEXER_HPP_
#define _LEXER_HPP_

#include <optional>
#include <string>
#include <string_view>

namespace serialkit {

enum class TokenType {
  // Keywords
  NAMESPACE,
  ENUM,
  MODEL,
  OPTIONAL,
  REPEATED,
  PACKED,
  INTERNED,
  BITMAP,

  // Primitives
  INT8,
  INT16,
  INT32,
  INT64,
  UINT8,
  UINT16,
  UINT32,
  UINT64,
  FLOAT,
  DOUBLE,
  BOOL,
  STRING,
  BYTE,

  // Symbols
  SEMICOLON, // ;
  EQUALS,    // =
  LBRACE,    // {
  RBRACE,    // }
  DOT,       // .

  // Literals
  IDENTIFIER,
  NUMBER,

  // Special
  END_OF_FILE,
  INVALID
};

struct SourceLocation {
  size_t line;
  size_t column;
  size_t offset;

  SourceLocation(size_t l = 1, size_t c = 1, size_t o = 0)
      : line(l), column(c), offset(o) {}
};

struct Token {
  TokenType type;
  std::string value;
  SourceLocation location;

  Token(TokenType t, std::string v, SourceLocation loc)
      : type(t), value(std::move(v)), location(loc) {}

  Token(TokenType t, SourceLocation loc) : type(t), value(), location(loc) {}
};

class Lexer {
public:
  explicit Lexer(std::string_view source);

  Token next_token();
  Token peek_token();
  bool has_more_tokens() const;

  std::string format_error(const std::string &message,
                           const SourceLocation &loc) const;

  SourceLocation current_location() const;

private:
  std::string_view source_;
  size_t position_;
  size_t line_;
  size_t column_;
  std::optional<Token> peeked_token_;

  char current_char() const;
  char peek_char(size_t offset = 1) const;
  void advance();
  bool is_at_end() const;

  void skip_whitespace_and_comments();
  void skip_line_comment();
  void skip_block_comment();

  Token read_identifier_or_keyword();
  Token read_number();
  Token make_token(TokenType type, const std::string &value);
  Token make_token(TokenType type);

  bool is_identifier_start(char c) const;
  bool is_identifier_continue(char c) const;
  bool is_digit(char c) const;
  TokenType keyword_or_identifier(const std::string &word) const;
};

const char *token_type_to_string(TokenType type);

} // namespace serialkit

#endif