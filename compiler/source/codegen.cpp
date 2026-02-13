#include "codegen.hpp"
#include <sstream>

namespace serialkit {

CodeGenerator::CodeGenerator(const Schema &schema) : schema_(schema) {}

std::string CodeGenerator::generate_header() {
  header_.str("");
  header_.clear();

  header_ << "#pragma once\n\n";
  generate_includes();
  generate_namespace_open(header_);

  for (const auto &decl : schema_.declarations) {
    if (auto *enum_decl = dynamic_cast<const EnumDecl *>(decl.get())) {
      generate_enum_declaration(*enum_decl);
    } else if (auto *model_decl = dynamic_cast<const ModelDecl *>(decl.get())) {
      generate_model_declaration(*model_decl);
    }
  }

  generate_namespace_close(header_);
  return header_.str();
}

std::string CodeGenerator::generate_source() {
  source_.str("");
  source_.clear();

  source_ << "#include \"" << schema_.namespace_name << ".hpp\"\n";
  source_ << "#include <cstring>\n";
  source_ << "#include <stdexcept>\n\n";

  generate_namespace_open(source_);

  for (const auto &decl : schema_.declarations) {
    if (auto *model_decl = dynamic_cast<const ModelDecl *>(decl.get())) {
      generate_model_implementation(*model_decl);
    }
  }

  generate_namespace_close(source_);
  return source_.str();
}

void CodeGenerator::generate_includes() {
  header_ << "#include <cstdint>\n";
  header_ << "#include <string>\n";
  header_ << "#include <vector>\n";
  header_ << "#include <optional>\n";
  header_ << "#include <memory>\n\n";
}

void CodeGenerator::generate_namespace_open(std::ostringstream &out) {
  out << "namespace " << schema_.namespace_name << " {\n\n";
}

void CodeGenerator::generate_namespace_close(std::ostringstream &out) {
  out << "} // namespace " << schema_.namespace_name << "\n";
}

void CodeGenerator::generate_enum_declaration(const EnumDecl &enum_decl) {
  header_ << "enum class " << enum_decl.name << " : int32_t {\n";

  for (size_t i = 0; i < enum_decl.values.size(); ++i) {
    const auto &value = enum_decl.values[i];
    header_ << "  " << value->name << " = " << value->value;
    if (i < enum_decl.values.size() - 1) {
      header_ << ",";
    }
    header_ << "\n";
  }

  header_ << "};\n\n";
}

void CodeGenerator::generate_model_declaration(const ModelDecl &model) {
  header_ << "class " << model.name << " {\n";
  header_ << "public:\n";
  header_ << "  " << model.name << "() = default;\n\n";

  for (const auto &field : model.fields) {
    header_ << "  " << get_field_type(*field) << " " << field->name;

    if (field->is_repeated()) {
      header_ << ";\n";
    } else if (field->is_optional()) {
      header_ << ";\n";
    } else {
      if (auto *prim = dynamic_cast<const PrimitiveType *>(field->type.get())) {
        switch (prim->kind) {
        case PrimitiveTypeKind::BOOL:
          header_ << " = false;\n";
          break;
        case PrimitiveTypeKind::STRING:
          header_ << ";\n";
          break;
        default:
          header_ << " = 0;\n";
          break;
        }
      } else {
        header_ << ";\n";
      }
    }
  }

  header_ << "\n";
  header_ << "  std::vector<uint8_t> serialize() const;\n";
  header_ << "  bool deserialize(const std::vector<uint8_t>& data);\n";
  header_ << "};\n\n";
}

void CodeGenerator::generate_model_implementation(const ModelDecl &model) {
  generate_serialize_method(model);
  generate_deserialize_method(model);
}

void CodeGenerator::generate_serialize_method(const ModelDecl &model) {
  source_ << "std::vector<uint8_t> " << model.name << "::serialize() const {\n";
  source_ << "  std::vector<uint8_t> buffer;\n";
  source_ << "  buffer.reserve(64);\n\n";

  for (const auto &field : model.fields) {
    generate_field_serializer(*field, "  ");
  }

  source_ << "\n  return buffer;\n";
  source_ << "}\n\n";
}

void CodeGenerator::generate_deserialize_method(const ModelDecl &model) {
  source_ << "bool " << model.name
          << "::deserialize(const std::vector<uint8_t>& data) {\n";
  source_ << "  size_t pos = 0;\n";
  source_ << "  while (pos < data.size()) {\n";
  source_ << "    if (pos + 1 > data.size()) return false;\n\n";

  source_ << "    uint64_t tag = 0;\n";
  source_ << "    {\n";
  source_ << "      int shift = 0;\n";
  source_ << "      while (pos < data.size()) {\n";
  source_ << "        uint8_t byte = data[pos++];\n";
  source_ << "        tag |= static_cast<uint64_t>(byte & 0x7F) << shift;\n";
  source_ << "        if ((byte & 0x80) == 0) break;\n";
  source_ << "        shift += 7;\n";
  source_ << "      }\n";
  source_ << "    }\n\n";

  source_ << "    uint32_t field_number = static_cast<uint32_t>(tag >> 3);\n";
  source_ << "    uint8_t wire_type = static_cast<uint8_t>(tag & 0x7);\n\n";

  source_ << "    switch (field_number) {\n";

  for (const auto &field : model.fields) {
    generate_field_deserializer(*field, "    ");
  }

  source_ << "    default:\n";
  source_ << "      // Skip unknown field\n";
  source_ << "      if (wire_type == 0) {\n";
  source_ << "        while (pos < data.size() && (data[pos] & 0x80)) pos++;\n";
  source_ << "        if (pos < data.size()) pos++;\n";
  source_ << "      } else if (wire_type == 2) {\n";
  source_ << "        uint64_t length = 0;\n";
  source_ << "        int shift = 0;\n";
  source_ << "        while (pos < data.size()) {\n";
  source_ << "          uint8_t byte = data[pos++];\n";
  source_
      << "          length |= static_cast<uint64_t>(byte & 0x7F) << shift;\n";
  source_ << "          if ((byte & 0x80) == 0) break;\n";
  source_ << "          shift += 7;\n";
  source_ << "        }\n";
  source_ << "        pos += length;\n";
  source_ << "      } else if (wire_type == 1) {\n";
  source_ << "        pos += 8;\n";
  source_ << "      } else if (wire_type == 5) {\n";
  source_ << "        pos += 4;\n";
  source_ << "      }\n";
  source_ << "      break;\n";
  source_ << "    }\n";
  source_ << "  }\n";
  source_ << "  return true;\n";
  source_ << "}\n\n";
}

void CodeGenerator::generate_field_serializer(const Field &field,
                                              const std::string &indent) {
  uint8_t wire_type = get_wire_type_value(*field.type, field);
  uint32_t tag = (field.number << 3) | wire_type;

  auto *prim_type = dynamic_cast<const PrimitiveType *>(field.type.get());

  if (field.is_repeated()) {
    source_ << indent << "if (!" << field.name << ".empty()) {\n";

    if (field.is_packed() && prim_type) {
      source_ << indent
              << "  uint32_t packed_tag = " << ((field.number << 3) | 2)
              << ";\n";
      source_ << indent << "  {\n";
      source_ << indent << "    uint64_t val = packed_tag;\n";
      source_ << indent << "    while (val > 0x7F) {\n";
      source_ << indent
              << "      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | "
                 "0x80));\n";
      source_ << indent << "      val >>= 7;\n";
      source_ << indent << "    }\n";
      source_ << indent << "    buffer.push_back(static_cast<uint8_t>(val));\n";
      source_ << indent << "  }\n\n";

      source_ << indent << "  std::vector<uint8_t> packed_data;\n";
      source_ << indent << "  for (const auto& item : " << field.name
              << ") {\n";

      if (prim_type->kind == PrimitiveTypeKind::INT8 ||
          prim_type->kind == PrimitiveTypeKind::INT16 ||
          prim_type->kind == PrimitiveTypeKind::INT32 ||
          prim_type->kind == PrimitiveTypeKind::INT64 ||
          prim_type->kind == PrimitiveTypeKind::UINT8 ||
          prim_type->kind == PrimitiveTypeKind::UINT16 ||
          prim_type->kind == PrimitiveTypeKind::UINT32 ||
          prim_type->kind == PrimitiveTypeKind::UINT64) {
        source_ << indent
                << "    uint64_t val = static_cast<uint64_t>(item);\n";
        source_ << indent << "    while (val > 0x7F) {\n";
        source_ << indent
                << "      packed_data.push_back(static_cast<uint8_t>((val & "
                   "0x7F) | 0x80));\n";
        source_ << indent << "      val >>= 7;\n";
        source_ << indent << "    }\n";
        source_ << indent
                << "    packed_data.push_back(static_cast<uint8_t>(val));\n";
      } else if (prim_type->kind == PrimitiveTypeKind::FLOAT) {
        source_ << indent << "    float fval = item;\n";
        source_ << indent << "    uint32_t val;\n";
        source_ << indent << "    std::memcpy(&val, &fval, sizeof(float));\n";
        source_
            << indent
            << "    packed_data.push_back(static_cast<uint8_t>(val & 0xFF));\n";
        source_ << indent
                << "    packed_data.push_back(static_cast<uint8_t>((val >> 8) "
                   "& 0xFF));\n";
        source_ << indent
                << "    packed_data.push_back(static_cast<uint8_t>((val >> 16) "
                   "& 0xFF));\n";
        source_ << indent
                << "    packed_data.push_back(static_cast<uint8_t>((val >> 24) "
                   "& 0xFF));\n";
      } else if (prim_type->kind == PrimitiveTypeKind::DOUBLE) {
        source_ << indent << "    double dval = item;\n";
        source_ << indent << "    uint64_t val;\n";
        source_ << indent << "    std::memcpy(&val, &dval, sizeof(double));\n";
        source_ << indent << "    for (int i = 0; i < 8; ++i) {\n";
        source_ << indent
                << "      packed_data.push_back(static_cast<uint8_t>((val >> "
                   "(i * 8)) & 0xFF));\n";
        source_ << indent << "    }\n";
      }

      source_ << indent << "  }\n\n";

      source_ << indent << "  uint64_t length = packed_data.size();\n";
      source_ << indent << "  while (length > 0x7F) {\n";
      source_ << indent
              << "    buffer.push_back(static_cast<uint8_t>((length & 0x7F) | "
                 "0x80));\n";
      source_ << indent << "    length >>= 7;\n";
      source_ << indent << "  }\n";
      source_ << indent
              << "  buffer.push_back(static_cast<uint8_t>(length));\n";
      source_ << indent
              << "  buffer.insert(buffer.end(), packed_data.begin(), "
                 "packed_data.end());\n";
    } else {
      source_ << indent << "  for (const auto& item : " << field.name
              << ") {\n";
      source_ << indent << "    uint32_t item_tag = " << tag << ";\n";
      source_ << indent << "    {\n";
      source_ << indent << "      uint64_t val = item_tag;\n";
      source_ << indent << "      while (val > 0x7F) {\n";
      source_ << indent
              << "        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | "
                 "0x80));\n";
      source_ << indent << "        val >>= 7;\n";
      source_ << indent << "      }\n";
      source_ << indent
              << "      buffer.push_back(static_cast<uint8_t>(val));\n";
      source_ << indent << "    }\n";

      if (prim_type) {
        if (prim_type->kind == PrimitiveTypeKind::STRING) {
          source_ << indent << "    uint64_t length = item.size();\n";
          source_ << indent << "    while (length > 0x7F) {\n";
          source_ << indent
                  << "      buffer.push_back(static_cast<uint8_t>((length & "
                     "0x7F) | 0x80));\n";
          source_ << indent << "      length >>= 7;\n";
          source_ << indent << "    }\n";
          source_ << indent
                  << "    buffer.push_back(static_cast<uint8_t>(length));\n";
          source_
              << indent
              << "    buffer.insert(buffer.end(), item.begin(), item.end());\n";
        } else {
          source_ << indent
                  << "    uint64_t val = static_cast<uint64_t>(item);\n";
          source_ << indent << "    while (val > 0x7F) {\n";
          source_ << indent
                  << "      buffer.push_back(static_cast<uint8_t>((val & 0x7F) "
                     "| 0x80));\n";
          source_ << indent << "      val >>= 7;\n";
          source_ << indent << "    }\n";
          source_ << indent
                  << "    buffer.push_back(static_cast<uint8_t>(val));\n";
        }
      } else {
        // Check if this is an enum type
        auto *user_type = dynamic_cast<const UserType *>(field.type.get());
        if (user_type && schema_.find_enum(user_type->name)) {
          // Serialize enum as integer
          source_ << indent
                  << "    uint64_t val = static_cast<uint64_t>(item);\n";
          source_ << indent << "    while (val > 0x7F) {\n";
          source_ << indent
                  << "      buffer.push_back(static_cast<uint8_t>((val & 0x7F) "
                     "| 0x80));\n";
          source_ << indent << "      val >>= 7;\n";
          source_ << indent << "    }\n";
          source_ << indent
                  << "    buffer.push_back(static_cast<uint8_t>(val));\n";
        } else {
          source_ << indent << "    auto item_data = item.serialize();\n";
          source_ << indent << "    uint64_t length = item_data.size();\n";
          source_ << indent << "    while (length > 0x7F) {\n";
          source_ << indent
                  << "      buffer.push_back(static_cast<uint8_t>((length & "
                     "0x7F) | 0x80));\n";
          source_ << indent << "      length >>= 7;\n";
          source_ << indent << "    }\n";
          source_ << indent
                  << "    buffer.push_back(static_cast<uint8_t>(length));\n";
          source_ << indent
                  << "    buffer.insert(buffer.end(), item_data.begin(), "
                     "item_data.end());\n";
        }
      }

      source_ << indent << "  }\n";
    }

    source_ << indent << "}\n\n";
  } else if (field.is_optional()) {
    source_ << indent << "if (" << field.name << ".has_value()) {\n";
    source_ << indent << "  uint32_t field_tag = " << tag << ";\n";
    source_ << indent << "  {\n";
    source_ << indent << "    uint64_t val = field_tag;\n";
    source_ << indent << "    while (val > 0x7F) {\n";
    source_ << indent
            << "      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | "
               "0x80));\n";
    source_ << indent << "      val >>= 7;\n";
    source_ << indent << "    }\n";
    source_ << indent << "    buffer.push_back(static_cast<uint8_t>(val));\n";
    source_ << indent << "  }\n";

    if (prim_type) {
      if (prim_type->kind == PrimitiveTypeKind::STRING) {
        source_ << indent << "  uint64_t length = " << field.name
                << "->size();\n";
        source_ << indent << "  while (length > 0x7F) {\n";
        source_ << indent
                << "    buffer.push_back(static_cast<uint8_t>((length & 0x7F) "
                   "| 0x80));\n";
        source_ << indent << "    length >>= 7;\n";
        source_ << indent << "  }\n";
        source_ << indent
                << "  buffer.push_back(static_cast<uint8_t>(length));\n";
        source_ << indent << "  buffer.insert(buffer.end(), " << field.name
                << "->begin(), " << field.name << "->end());\n";
      } else {
        source_ << indent << "  uint64_t val = static_cast<uint64_t>(*"
                << field.name << ");\n";
        source_ << indent << "  while (val > 0x7F) {\n";
        source_ << indent
                << "    buffer.push_back(static_cast<uint8_t>((val & 0x7F) | "
                   "0x80));\n";
        source_ << indent << "    val >>= 7;\n";
        source_ << indent << "  }\n";
        source_ << indent << "  buffer.push_back(static_cast<uint8_t>(val));\n";
      }
    } else {
      auto *user_type = dynamic_cast<const UserType *>(field.type.get());
      if (user_type && schema_.find_enum(user_type->name)) {
        source_ << indent << "  uint64_t val = static_cast<uint64_t>(*"
                << field.name << ");\n";
        source_ << indent << "  while (val > 0x7F) {\n";
        source_ << indent
                << "    buffer.push_back(static_cast<uint8_t>((val & 0x7F) | "
                   "0x80));\n";
        source_ << indent << "    val >>= 7;\n";
        source_ << indent << "  }\n";
        source_ << indent << "  buffer.push_back(static_cast<uint8_t>(val));\n";
      } else {
        source_ << indent << "  auto field_data = " << field.name
                << "->serialize();\n";
        source_ << indent << "  uint64_t length = field_data.size();\n";
        source_ << indent << "  while (length > 0x7F) {\n";
        source_
            << indent
            << "    buffer.push_back(static_cast<uint8_t>((length & 0x7F) | "
               "0x80));\n";
        source_ << indent << "    length >>= 7;\n";
        source_ << indent << "  }\n";
        source_ << indent
                << "  buffer.push_back(static_cast<uint8_t>(length));\n";
        source_ << indent
                << "  buffer.insert(buffer.end(), field_data.begin(), "
                   "field_data.end());\n";
      }
    }

    source_ << indent << "}\n\n";
  } else {
    source_ << indent << "{\n";
    source_ << indent << "  uint32_t field_tag = " << tag << ";\n";
    source_ << indent << "  {\n";
    source_ << indent << "    uint64_t val = field_tag;\n";
    source_ << indent << "    while (val > 0x7F) {\n";
    source_ << indent
            << "      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | "
               "0x80));\n";
    source_ << indent << "      val >>= 7;\n";
    source_ << indent << "    }\n";
    source_ << indent << "    buffer.push_back(static_cast<uint8_t>(val));\n";
    source_ << indent << "  }\n";

    if (prim_type) {
      if (prim_type->kind == PrimitiveTypeKind::STRING) {
        source_ << indent << "  uint64_t length = " << field.name
                << ".size();\n";
        source_ << indent << "  while (length > 0x7F) {\n";
        source_ << indent
                << "    buffer.push_back(static_cast<uint8_t>((length & 0x7F) "
                   "| 0x80));\n";
        source_ << indent << "    length >>= 7;\n";
        source_ << indent << "  }\n";
        source_ << indent
                << "  buffer.push_back(static_cast<uint8_t>(length));\n";
        source_ << indent << "  buffer.insert(buffer.end(), " << field.name
                << ".begin(), " << field.name << ".end());\n";
      } else if (prim_type->kind == PrimitiveTypeKind::BOOL) {
        source_ << indent << "  buffer.push_back(" << field.name
                << " ? 1 : 0);\n";
      } else {
        source_ << indent << "  uint64_t val = static_cast<uint64_t>("
                << field.name << ");\n";
        source_ << indent << "  while (val > 0x7F) {\n";
        source_ << indent
                << "    buffer.push_back(static_cast<uint8_t>((val & 0x7F) | "
                   "0x80));\n";
        source_ << indent << "    val >>= 7;\n";
        source_ << indent << "  }\n";
        source_ << indent << "  buffer.push_back(static_cast<uint8_t>(val));\n";
      }
    } else {
      // Check if this is an enum type
      auto *user_type = dynamic_cast<const UserType *>(field.type.get());
      if (user_type && schema_.find_enum(user_type->name)) {
        // Serialize enum as integer
        source_ << indent << "  uint64_t val = static_cast<uint64_t>("
                << field.name << ");\n";
        source_ << indent << "  while (val > 0x7F) {\n";
        source_ << indent
                << "    buffer.push_back(static_cast<uint8_t>((val & 0x7F) | "
                   "0x80));\n";
        source_ << indent << "    val >>= 7;\n";
        source_ << indent << "  }\n";
        source_ << indent << "  buffer.push_back(static_cast<uint8_t>(val));\n";
      } else {
        source_ << indent << "  auto field_data = " << field.name
                << ".serialize();\n";
        source_ << indent << "  uint64_t length = field_data.size();\n";
        source_ << indent << "  while (length > 0x7F) {\n";
        source_
            << indent
            << "    buffer.push_back(static_cast<uint8_t>((length & 0x7F) | "
               "0x80));\n";
        source_ << indent << "    length >>= 7;\n";
        source_ << indent << "  }\n";
        source_ << indent
                << "  buffer.push_back(static_cast<uint8_t>(length));\n";
        source_ << indent
                << "  buffer.insert(buffer.end(), field_data.begin(), "
                   "field_data.end());\n";
      }
    }

    source_ << indent << "}\n\n";
  }
}

void CodeGenerator::generate_field_deserializer(const Field &field,
                                                const std::string &indent) {
  source_ << indent << "case " << field.number << ": {\n";

  auto *prim_type = dynamic_cast<const PrimitiveType *>(field.type.get());

  if (prim_type) {
    if (prim_type->kind == PrimitiveTypeKind::STRING) {
      source_ << indent << "  uint64_t length = 0;\n";
      source_ << indent << "  int shift = 0;\n";
      source_ << indent << "  while (pos < data.size()) {\n";
      source_ << indent << "    uint8_t byte = data[pos++];\n";
      source_ << indent
              << "    length |= static_cast<uint64_t>(byte & 0x7F) << shift;\n";
      source_ << indent << "    if ((byte & 0x80) == 0) break;\n";
      source_ << indent << "    shift += 7;\n";
      source_ << indent << "  }\n";

      if (field.is_repeated()) {
        source_ << indent
                << "  std::string str(reinterpret_cast<const "
                   "char*>(&data[pos]), length);\n";
        source_ << indent << "  " << field.name
                << ".push_back(std::move(str));\n";
      } else if (field.is_optional()) {
        source_ << indent << "  " << field.name
                << " = std::string(reinterpret_cast<const char*>(&data[pos]), "
                   "length);\n";
      } else {
        source_
            << indent << "  " << field.name
            << ".assign(reinterpret_cast<const char*>(&data[pos]), length);\n";
      }
      source_ << indent << "  pos += length;\n";
    } else {
      source_ << indent << "  uint64_t value = 0;\n";
      source_ << indent << "  int shift = 0;\n";
      source_ << indent << "  while (pos < data.size()) {\n";
      source_ << indent << "    uint8_t byte = data[pos++];\n";
      source_ << indent
              << "    value |= static_cast<uint64_t>(byte & 0x7F) << shift;\n";
      source_ << indent << "    if ((byte & 0x80) == 0) break;\n";
      source_ << indent << "    shift += 7;\n";
      source_ << indent << "  }\n";

      std::string cpp_type = get_cpp_type(*field.type);

      if (field.is_repeated()) {
        source_ << indent << "  " << field.name << ".push_back(static_cast<"
                << cpp_type << ">(value));\n";
      } else if (field.is_optional()) {
        source_ << indent << "  " << field.name << " = static_cast<" << cpp_type
                << ">(value);\n";
      } else {
        source_ << indent << "  " << field.name << " = static_cast<" << cpp_type
                << ">(value);\n";
      }
    }
  } else {
    std::string type_name = field.type->get_name();

    // Check if this is an enum type
    auto *user_type = dynamic_cast<const UserType *>(field.type.get());
    if (user_type && schema_.find_enum(user_type->name)) {
      // Deserialize enum as integer
      source_ << indent << "  uint64_t value = 0;\n";
      source_ << indent << "  int shift = 0;\n";
      source_ << indent << "  while (pos < data.size()) {\n";
      source_ << indent << "    uint8_t byte = data[pos++];\n";
      source_ << indent
              << "    value |= static_cast<uint64_t>(byte & 0x7F) << shift;\n";
      source_ << indent << "    if ((byte & 0x80) == 0) break;\n";
      source_ << indent << "    shift += 7;\n";
      source_ << indent << "  }\n";

      if (field.is_repeated()) {
        source_ << indent << "  " << field.name << ".push_back(static_cast<"
                << type_name << ">(value));\n";
      } else if (field.is_optional()) {
        source_ << indent << "  " << field.name << " = static_cast<"
                << type_name << ">(value);\n";
      } else {
        source_ << indent << "  " << field.name << " = static_cast<"
                << type_name << ">(value);\n";
      }
    } else {
      // Deserialize as model (complex type)
      source_ << indent << "  uint64_t length = 0;\n";
      source_ << indent << "  int shift = 0;\n";
      source_ << indent << "  while (pos < data.size()) {\n";
      source_ << indent << "    uint8_t byte = data[pos++];\n";
      source_ << indent
              << "    length |= static_cast<uint64_t>(byte & 0x7F) << shift;\n";
      source_ << indent << "    if ((byte & 0x80) == 0) break;\n";
      source_ << indent << "    shift += 7;\n";
      source_ << indent << "  }\n";

      if (field.is_repeated()) {
        source_ << indent << "  " << type_name << " item;\n";
        source_ << indent
                << "  std::vector<uint8_t> item_data(data.begin() + pos, "
                   "data.begin() + pos + length);\n";
        source_ << indent
                << "  if (!item.deserialize(item_data)) return false;\n";
        source_ << indent << "  " << field.name
                << ".push_back(std::move(item));\n";
      } else if (field.is_optional()) {
        source_ << indent << "  " << type_name << " value;\n";
        source_ << indent
                << "  std::vector<uint8_t> item_data(data.begin() + pos, "
                   "data.begin() + pos + length);\n";
        source_ << indent
                << "  if (!value.deserialize(item_data)) return false;\n";
        source_ << indent << "  " << field.name << " = std::move(value);\n";
      } else {
        source_ << indent
                << "  std::vector<uint8_t> item_data(data.begin() + pos, "
                   "data.begin() + pos + length);\n";
        source_ << indent << "  if (!" << field.name
                << ".deserialize(item_data)) return false;\n";
      }

      source_ << indent << "  pos += length;\n";
    }
  }

  source_ << indent << "  break;\n";
  source_ << indent << "}\n";
}

std::string CodeGenerator::get_cpp_type(const Type &type) const {
  if (auto *prim_type = dynamic_cast<const PrimitiveType *>(&type)) {
    switch (prim_type->kind) {
    case PrimitiveTypeKind::INT8:
      return "int8_t";
    case PrimitiveTypeKind::INT16:
      return "int16_t";
    case PrimitiveTypeKind::INT32:
      return "int32_t";
    case PrimitiveTypeKind::INT64:
      return "int64_t";
    case PrimitiveTypeKind::UINT8:
      return "uint8_t";
    case PrimitiveTypeKind::UINT16:
      return "uint16_t";
    case PrimitiveTypeKind::UINT32:
      return "uint32_t";
    case PrimitiveTypeKind::UINT64:
      return "uint64_t";
    case PrimitiveTypeKind::FLOAT:
      return "float";
    case PrimitiveTypeKind::DOUBLE:
      return "double";
    case PrimitiveTypeKind::BOOL:
      return "bool";
    case PrimitiveTypeKind::STRING:
      return "std::string";
    case PrimitiveTypeKind::BYTE:
      return "uint8_t";
    default:
      return "int32_t";
    }
  } else if (auto *user_type = dynamic_cast<const UserType *>(&type)) {
    return user_type->name;
  }
  return "int32_t";
}

std::string CodeGenerator::get_wire_type(const Type &type,
                                         const Field &field) const {
  uint8_t wire_value = get_wire_type_value(type, field);
  return std::to_string(wire_value);
}

uint8_t CodeGenerator::get_wire_type_value(const Type &type,
                                           const Field &field) const {
  if (field.is_packed()) {
    return 3; // PACKED_ARRAY
  }

  if (field.is_bitmap()) {
    return 7; // BITMAP
  }

  if (auto *prim_type = dynamic_cast<const PrimitiveType *>(&type)) {
    if (field.is_interned() && prim_type->kind == PrimitiveTypeKind::STRING) {
      return 6; // STRING_TABLE
    }

    switch (prim_type->kind) {
    case PrimitiveTypeKind::INT8:
    case PrimitiveTypeKind::INT16:
    case PrimitiveTypeKind::INT32:
    case PrimitiveTypeKind::INT64:
    case PrimitiveTypeKind::UINT8:
    case PrimitiveTypeKind::UINT16:
    case PrimitiveTypeKind::UINT32:
    case PrimitiveTypeKind::UINT64:
    case PrimitiveTypeKind::BOOL:
      return 0; // VARINT
    case PrimitiveTypeKind::DOUBLE:
      return 1; // FIXED64
    case PrimitiveTypeKind::FLOAT:
      return 5; // FIXED32
    case PrimitiveTypeKind::STRING:
    case PrimitiveTypeKind::BYTE:
      return 2; // LENGTH_DELIMITED
    default:
      return 0;
    }
  }

  return 2; // LENGTH_DELIMITED for user types
}

std::string CodeGenerator::get_field_type(const Field &field) const {
  std::string base_type = get_cpp_type(*field.type);

  if (field.is_repeated()) {
    return "std::vector<" + base_type + ">";
  } else if (field.is_optional()) {
    return "std::optional<" + base_type + ">";
  }

  return base_type;
}

} // namespace serialkit
