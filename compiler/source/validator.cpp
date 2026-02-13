#include "validator.hpp"
#include <sstream>
#include <unordered_set>

namespace serialkit {

void ValidationContext::add_error(const std::string &message,
                                  SourceLocation location) {
  errors_.emplace_back(message, location);
}

void ValidationContext::add_error(std::string &&message,
                                  SourceLocation location) {
  errors_.emplace_back(std::move(message), location);
}

void ValidationContext::register_enum(const std::string &name,
                                      const EnumDecl *decl) {
  enums_[name] = decl;
}

void ValidationContext::register_model(const std::string &name,
                                       const ModelDecl *decl) {
  models_[name] = decl;
}

const EnumDecl *ValidationContext::find_enum(const std::string &name) const {
  auto it = enums_.find(name);
  return it != enums_.end() ? it->second : nullptr;
}

const ModelDecl *ValidationContext::find_model(const std::string &name) const {
  auto it = models_.find(name);
  return it != models_.end() ? it->second : nullptr;
}

bool ValidationContext::type_exists(const std::string &name) const {
  return enums_.find(name) != enums_.end() ||
         models_.find(name) != models_.end();
}

bool Validator::validate(const Schema &schema) {
  SemanticValidator validator(context_);
  validator.visit_schema(schema);
  return !context_.has_errors();
}

const std::vector<ValidationError> &Validator::get_errors() const {
  return context_.get_errors();
}

SemanticValidator::SemanticValidator(ValidationContext &context)
    : context_(context), current_model_(nullptr) {}

void SemanticValidator::visit_schema(const Schema &schema) {
  if (schema.namespace_name.empty()) {
    context_.add_error("Namespace cannot be empty", schema.location);
  }

  std::unordered_set<std::string> declaration_names;
  declaration_names.reserve(schema.declarations.size());

  for (const auto &decl : schema.declarations) {
    if (!declaration_names.insert(decl->name).second) {
      std::ostringstream oss;
      oss << "Duplicate declaration name '" << decl->name << "'";
      context_.add_error(oss.str(), decl->location);
    }
  }

  for (const auto &decl : schema.declarations) {
    if (auto *enum_decl = dynamic_cast<const EnumDecl *>(decl.get())) {
      context_.register_enum(enum_decl->name, enum_decl);
    } else if (auto *model_decl = dynamic_cast<const ModelDecl *>(decl.get())) {
      context_.register_model(model_decl->name, model_decl);
    }
  }

  for (const auto &decl : schema.declarations) {
    if (auto *enum_decl = dynamic_cast<const EnumDecl *>(decl.get())) {
      visit_enum(*enum_decl);
    } else if (auto *model_decl = dynamic_cast<const ModelDecl *>(decl.get())) {
      visit_model(*model_decl);
    }
  }
}

void SemanticValidator::visit_enum(const EnumDecl &enum_decl) {
  if (enum_decl.values.empty()) {
    std::ostringstream oss;
    oss << "Enum '" << enum_decl.name << "' must have at least one value";
    context_.add_error(oss.str(), enum_decl.location);
    return;
  }

  check_duplicate_enum_values(enum_decl);

  for (const auto &value : enum_decl.values) {
    visit_enum_value(*value);
  }
}

void SemanticValidator::visit_model(const ModelDecl &model) {
  if (model.fields.empty()) {
    std::ostringstream oss;
    oss << "Model '" << model.name << "' must have at least one field";
    context_.add_error(oss.str(), model.location);
    return;
  }

  current_model_ = &model;
  check_duplicate_field_numbers(model);

  for (const auto &field : model.fields) {
    visit_field(*field);
  }

  current_model_ = nullptr;
}

void SemanticValidator::visit_field(const Field &field) {
  validate_field_number(field);
  validate_field_modifiers(field);
  validate_type_exists(*field.type, field.location);
  check_modifier_compatibility(field);
}

void SemanticValidator::visit_enum_value(const EnumValue &value) {
  if (value.value < 0) {
    std::ostringstream oss;
    oss << "Enum value '" << value.name << "' cannot be negative";
    context_.add_error(oss.str(), value.location);
  }
}

void SemanticValidator::validate_field_number(const Field &field) {
  if (field.number < 1 || field.number > 536870911) {
    std::ostringstream oss;
    oss << "Field number " << field.number
        << " is out of valid range (1-536870911)";
    context_.add_error(oss.str(), field.location);
  }

  if (field.number >= 19000 && field.number <= 19999) {
    std::ostringstream oss;
    oss << "Field number " << field.number
        << " is in reserved range (19000-19999)";
    context_.add_error(oss.str(), field.location);
  }
}

void SemanticValidator::validate_field_modifiers(const Field &field) {
  bool has_repeated = field.is_repeated();
  bool has_packed = field.is_packed();
  bool has_interned = field.is_interned();
  bool has_bitmap = field.is_bitmap();
  bool has_optional = field.is_optional();

  if (has_optional && has_repeated) {
    context_.add_error("Field cannot be both 'optional' and 'repeated'",
                       field.location);
  }

  if (has_packed && !has_repeated) {
    context_.add_error("'packed' modifier requires 'repeated'", field.location);
  }

  if (has_bitmap && !has_repeated) {
    context_.add_error("'bitmap' modifier requires 'repeated'", field.location);
  }

  if (has_packed && has_bitmap) {
    context_.add_error("Field cannot have both 'packed' and 'bitmap' modifiers",
                       field.location);
  }

  if (has_interned && field.type->get_name() != "string") {
    context_.add_error("Not string field cannot be marked as 'interned'",
                       field.location);
  }
}

void SemanticValidator::validate_type_exists(const Type &type,
                                             SourceLocation location) {
  if (!type.is_primitive()) {
    const auto *user_type = dynamic_cast<const UserType *>(&type);
    if (user_type && !context_.type_exists(user_type->name)) {
      std::ostringstream oss;
      oss << "Unknown type '" << user_type->name << "'";
      context_.add_error(oss.str(), location);
    }
  }
}

void SemanticValidator::check_duplicate_field_numbers(const ModelDecl &model) {
  std::unordered_set<int> field_numbers;
  field_numbers.reserve(model.fields.size());

  for (const auto &field : model.fields) {
    if (!field_numbers.insert(field->number).second) {
      std::ostringstream oss;
      oss << "Duplicate field number " << field->number << " in model '"
          << model.name << "'";
      context_.add_error(oss.str(), field->location);
    }
  }
}

void SemanticValidator::check_duplicate_enum_values(const EnumDecl &enum_decl) {
  std::unordered_set<std::string> value_names;
  std::unordered_set<int> value_numbers;
  value_names.reserve(enum_decl.values.size());
  value_numbers.reserve(enum_decl.values.size());

  for (const auto &value : enum_decl.values) {
    if (!value_names.insert(value->name).second) {
      std::ostringstream oss;
      oss << "Duplicate enum value name '" << value->name << "' in enum '"
          << enum_decl.name << "'";
      context_.add_error(oss.str(), value->location);
    }

    if (!value_numbers.insert(value->value).second) {
      std::ostringstream oss;
      oss << "Duplicate enum value " << value->value << " in enum '"
          << enum_decl.name << "'";
      context_.add_error(oss.str(), value->location);
    }
  }
}

void SemanticValidator::check_modifier_compatibility(const Field &field) {
  if (field.is_packed()) {
    if (!field.type->is_primitive()) {
      context_.add_error(
          "'packed' modifier can only be used with primitive types",
          field.location);
    }
  }

  if (field.is_interned()) {
    auto *prim_type = dynamic_cast<const PrimitiveType *>(field.type.get());
    if (!prim_type || prim_type->kind != PrimitiveTypeKind::STRING) {
      context_.add_error(
          "'interned' modifier can only be used with 'string' type",
          field.location);
    }
  }

  if (field.is_bitmap()) {
    auto *prim_type = dynamic_cast<const PrimitiveType *>(field.type.get());
    if (!prim_type || prim_type->kind != PrimitiveTypeKind::BOOL) {
      context_.add_error("'bitmap' modifier can only be used with 'bool' type",
                         field.location);
    }
  }
}

} // namespace serialkit
