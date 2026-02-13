#ifndef _VALIDATOR_HPP_
#define _VALIDATOR_HPP_

#include "ast.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace serialkit {

struct ValidationError {
  std::string message;
  SourceLocation location;

  ValidationError(std::string msg, SourceLocation loc)
      : message(std::move(msg)), location(loc) {}
};

class ValidationContext {
public:
  void add_error(const std::string &message, SourceLocation location);
  void add_error(std::string &&message, SourceLocation location);

  bool has_errors() const { return !errors_.empty(); }
  const std::vector<ValidationError> &get_errors() const { return errors_; }

  void register_enum(const std::string &name, const EnumDecl *decl);
  void register_model(const std::string &name, const ModelDecl *decl);

  const EnumDecl *find_enum(const std::string &name) const;
  const ModelDecl *find_model(const std::string &name) const;
  bool type_exists(const std::string &name) const;

private:
  std::vector<ValidationError> errors_;
  std::unordered_map<std::string, const EnumDecl *> enums_;
  std::unordered_map<std::string, const ModelDecl *> models_;
};

class AstVisitor {
public:
  virtual ~AstVisitor() = default;

  virtual void visit_schema(const Schema &schema) = 0;
  virtual void visit_enum(const EnumDecl &enum_decl) = 0;
  virtual void visit_model(const ModelDecl &model) = 0;
  virtual void visit_field(const Field &field) = 0;
  virtual void visit_enum_value(const EnumValue &value) = 0;
};

class Validator {
public:
  bool validate(const Schema &schema);
  const std::vector<ValidationError> &get_errors() const;

private:
  ValidationContext context_;
};

class SemanticValidator : public AstVisitor {
public:
  explicit SemanticValidator(ValidationContext &context);

  void visit_schema(const Schema &schema) override;
  void visit_enum(const EnumDecl &enum_decl) override;
  void visit_model(const ModelDecl &model) override;
  void visit_field(const Field &field) override;
  void visit_enum_value(const EnumValue &value) override;

private:
  ValidationContext &context_;
  const ModelDecl *current_model_;

  void validate_field_number(const Field &field);
  void validate_field_modifiers(const Field &field);
  void validate_type_exists(const Type &type, SourceLocation location);
  void check_duplicate_field_numbers(const ModelDecl &model);
  void check_duplicate_enum_values(const EnumDecl &enum_decl);
  void check_modifier_compatibility(const Field &field);
};

} // namespace serialkit

#endif