#ifndef _CODEGEN_HPP_
#define _CODEGEN_HPP_

#include "ast.hpp"
#include <sstream>
#include <string>

namespace serialkit {

class CodeGenerator {
public:
  explicit CodeGenerator(const Schema &schema);

  std::string generate_header();
  std::string generate_source();

private:
  void generate_includes();
  void generate_namespace_open(std::ostringstream &);
  void generate_namespace_close(std::ostringstream &);
  void generate_enum_declaration(const EnumDecl &enum_decl);
  void generate_model_declaration(const ModelDecl &model);
  void generate_model_implementation(const ModelDecl &model);
  void generate_serialize_method(const ModelDecl &model);
  void generate_deserialize_method(const ModelDecl &model);
  void generate_field_serializer(const Field &field, const std::string &indent);
  void generate_field_deserializer(const Field &field,
                                   const std::string &indent);

  std::string get_cpp_type(const Type &type) const;
  std::string get_wire_type(const Type &type, const Field &field) const;
  std::string get_field_type(const Field &field) const;

  uint8_t get_wire_type_value(const Type &type, const Field &field) const;

  const Schema &schema_;
  std::ostringstream header_;
  std::ostringstream source_;
};

} // namespace serialkit

#endif // SERIALKIT_CODEGEN_HPP
