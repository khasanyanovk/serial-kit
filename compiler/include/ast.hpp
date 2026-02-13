#ifndef _AST_HPP_
#define _AST_HPP_

#include "lexer.hpp"
#include <memory>
#include <string>
#include <vector>

namespace serialkit {

class AstNode {
public:
  virtual ~AstNode() = default;

  SourceLocation location;

protected:
  explicit AstNode(SourceLocation loc) : location(loc) {}
};

enum class PrimitiveTypeKind : uint8_t {
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
  BYTE
};

class Type : public AstNode {
public:
  virtual std::string get_name() const = 0;
  virtual bool is_primitive() const = 0;

protected:
  explicit Type(SourceLocation loc) : AstNode(loc) {}
};

class PrimitiveType : public Type {
public:
  PrimitiveTypeKind kind;

  PrimitiveType(PrimitiveTypeKind k, SourceLocation loc) : Type(loc), kind(k) {}

  std::string get_name() const override;
  bool is_primitive() const override { return true; }
};

class UserType : public Type {
public:
  std::string name;

  UserType(std::string n, SourceLocation loc) : Type(loc), name(std::move(n)) {}

  std::string get_name() const override { return name; }
  bool is_primitive() const override { return false; }
};

enum FieldModifierFlags : uint8_t {
  MOD_NONE = 0,
  MOD_OPTIONAL = 1 << 0,
  MOD_REPEATED = 1 << 1,
  MOD_PACKED = 1 << 2,
  MOD_INTERNED = 1 << 3,
  MOD_BITMAP = 1 << 4
};

class Field : public AstNode {
public:
  std::unique_ptr<Type> type;
  std::string name;
  int number;
  uint8_t modifiers;

  Field(std::unique_ptr<Type> t, std::string n, int num, SourceLocation loc)
      : AstNode(loc), type(std::move(t)), name(std::move(n)), number(num),
        modifiers(MOD_NONE) {}

  inline void add_modifier(FieldModifierFlags mod) { modifiers |= mod; }
  inline bool has_modifier(FieldModifierFlags mod) const {
    return (modifiers & mod) != 0;
  }
  inline bool is_optional() const { return has_modifier(MOD_OPTIONAL); }
  inline bool is_repeated() const { return has_modifier(MOD_REPEATED); }
  inline bool is_packed() const { return has_modifier(MOD_PACKED); }
  inline bool is_interned() const { return has_modifier(MOD_INTERNED); }
  inline bool is_bitmap() const { return has_modifier(MOD_BITMAP); }
};

class EnumValue : public AstNode {
public:
  std::string name;
  int value;

  EnumValue(std::string n, int v, SourceLocation loc)
      : AstNode(loc), name(std::move(n)), value(v) {}
};

class Declaration : public AstNode {
public:
  std::string name;

  virtual ~Declaration() = default;

protected:
  Declaration(std::string n, SourceLocation loc)
      : AstNode(loc), name(std::move(n)) {}
};

class EnumDecl : public Declaration {
public:
  std::vector<std::unique_ptr<EnumValue>> values;

  EnumDecl(std::string n, SourceLocation loc)
      : Declaration(std::move(n), loc) {}
};

class ModelDecl : public Declaration {
public:
  std::vector<std::unique_ptr<Field>> fields;

  ModelDecl(std::string n, SourceLocation loc)
      : Declaration(std::move(n), loc) {}
};

class Schema : public AstNode {
public:
  std::string namespace_name;
  std::vector<std::unique_ptr<Declaration>> declarations;

  explicit Schema(SourceLocation loc = SourceLocation()) : AstNode(loc) {}

  const EnumDecl *find_enum(const std::string &name) const;
  const ModelDecl *find_model(const std::string &name) const;
};

PrimitiveTypeKind token_to_primitive_type(TokenType token);

} // namespace serialkit

#endif