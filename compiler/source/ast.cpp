#include "ast.hpp"
#include <stdexcept>

namespace serialkit {

std::string PrimitiveType::get_name() const {
  switch (kind) {
  case PrimitiveTypeKind::INT8:
    return "int8";
  case PrimitiveTypeKind::INT16:
    return "int16";
  case PrimitiveTypeKind::INT32:
    return "int32";
  case PrimitiveTypeKind::INT64:
    return "int64";
  case PrimitiveTypeKind::UINT8:
    return "uint8";
  case PrimitiveTypeKind::UINT16:
    return "uint16";
  case PrimitiveTypeKind::UINT32:
    return "uint32";
  case PrimitiveTypeKind::UINT64:
    return "uint64";
  case PrimitiveTypeKind::FLOAT:
    return "float";
  case PrimitiveTypeKind::DOUBLE:
    return "double";
  case PrimitiveTypeKind::BOOL:
    return "bool";
  case PrimitiveTypeKind::STRING:
    return "string";
  case PrimitiveTypeKind::BYTE:
    return "byte";
  default:
    return "unknown";
  }
}

const EnumDecl *Schema::find_enum(const std::string &name) const {
  for (const auto &decl : declarations) {
    if (auto *enum_decl = dynamic_cast<EnumDecl *>(decl.get())) {
      if (enum_decl->name == name) {
        return enum_decl;
      }
    }
  }
  return nullptr;
}

const ModelDecl *Schema::find_model(const std::string &name) const {
  for (const auto &decl : declarations) {
    if (auto *model_decl = dynamic_cast<ModelDecl *>(decl.get())) {
      if (model_decl->name == name) {
        return model_decl;
      }
    }
  }
  return nullptr;
}

PrimitiveTypeKind token_to_primitive_type(TokenType token) {
  switch (token) {
  case TokenType::INT8:
    return PrimitiveTypeKind::INT8;
  case TokenType::INT16:
    return PrimitiveTypeKind::INT16;
  case TokenType::INT32:
    return PrimitiveTypeKind::INT32;
  case TokenType::INT64:
    return PrimitiveTypeKind::INT64;
  case TokenType::UINT8:
    return PrimitiveTypeKind::UINT8;
  case TokenType::UINT16:
    return PrimitiveTypeKind::UINT16;
  case TokenType::UINT32:
    return PrimitiveTypeKind::UINT32;
  case TokenType::UINT64:
    return PrimitiveTypeKind::UINT64;
  case TokenType::FLOAT:
    return PrimitiveTypeKind::FLOAT;
  case TokenType::DOUBLE:
    return PrimitiveTypeKind::DOUBLE;
  case TokenType::BOOL:
    return PrimitiveTypeKind::BOOL;
  case TokenType::STRING:
    return PrimitiveTypeKind::STRING;
  case TokenType::BYTE:
    return PrimitiveTypeKind::BYTE;
  default:
    throw std::runtime_error("Invalid primitive type token");
  }
}

} // namespace serialkit
