#include "examples_auth.hpp"
#include <cstring>
#include <stdexcept>

namespace examples_auth {

std::vector<uint8_t> User::serialize() const {
  std::vector<uint8_t> buffer;
  buffer.reserve(64);

  {
    uint32_t field_tag = 10;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t length = username.size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), username.begin(), username.end());
  }

  {
    uint32_t field_tag = 18;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t length = email.size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), email.begin(), email.end());
  }

  {
    uint32_t field_tag = 24;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(user_id);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }

  {
    uint32_t field_tag = 34;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(role);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }

  {
    uint32_t field_tag = 40;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    buffer.push_back(email_verified ? 1 : 0);
  }

  {
    uint32_t field_tag = 48;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(created_at);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }

  if (avatar_url.has_value()) {
    uint32_t field_tag = 58;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t length = avatar_url->size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), avatar_url->begin(), avatar_url->end());
  }

  if (!permissions.empty()) {
    for (const auto& item : permissions) {
      uint32_t item_tag = 66;
      {
        uint64_t val = item_tag;
        while (val > 0x7F) {
          buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
          val >>= 7;
        }
        buffer.push_back(static_cast<uint8_t>(val));
      }
      uint64_t length = item.size();
      while (length > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
        length >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(length));
      buffer.insert(buffer.end(), item.begin(), item.end());
    }
  }

  if (profile_data.has_value()) {
    uint32_t field_tag = 74;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(*profile_data);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }


  return buffer;
}

bool User::deserialize(const std::vector<uint8_t>& data) {
  size_t pos = 0;
  while (pos < data.size()) {
    if (pos + 1 > data.size()) return false;

    uint64_t tag = 0;
    {
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        tag |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
    }

    uint32_t field_number = static_cast<uint32_t>(tag >> 3);
    uint8_t wire_type = static_cast<uint8_t>(tag & 0x7);

    switch (field_number) {
    case 1: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      username.assign(reinterpret_cast<const char*>(&data[pos]), length);
      pos += length;
      break;
    }
    case 2: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      email.assign(reinterpret_cast<const char*>(&data[pos]), length);
      pos += length;
      break;
    }
    case 3: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      user_id = static_cast<uint64_t>(value);
      break;
    }
    case 4: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      role = static_cast<UserRole>(value);
      break;
    }
    case 5: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      email_verified = static_cast<bool>(value);
      break;
    }
    case 6: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      created_at = static_cast<uint64_t>(value);
      break;
    }
    case 7: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      avatar_url = std::string(reinterpret_cast<const char*>(&data[pos]), length);
      pos += length;
      break;
    }
    case 8: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      std::string str(reinterpret_cast<const char*>(&data[pos]), length);
      permissions.push_back(std::move(str));
      pos += length;
      break;
    }
    case 9: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      profile_data = static_cast<uint8_t>(value);
      break;
    }
    default:
      // Skip unknown field
      if (wire_type == 0) {
        while (pos < data.size() && (data[pos] & 0x80)) pos++;
        if (pos < data.size()) pos++;
      } else if (wire_type == 2) {
        uint64_t length = 0;
        int shift = 0;
        while (pos < data.size()) {
          uint8_t byte = data[pos++];
          length |= static_cast<uint64_t>(byte & 0x7F) << shift;
          if ((byte & 0x80) == 0) break;
          shift += 7;
        }
        pos += length;
      } else if (wire_type == 1) {
        pos += 8;
      } else if (wire_type == 5) {
        pos += 4;
      }
      break;
    }
  }
  return true;
}

std::vector<uint8_t> LoginRequest::serialize() const {
  std::vector<uint8_t> buffer;
  buffer.reserve(64);

  {
    uint32_t field_tag = 10;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t length = username.size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), username.begin(), username.end());
  }

  {
    uint32_t field_tag = 18;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t length = password.size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), password.begin(), password.end());
  }

  if (remember_me.has_value()) {
    uint32_t field_tag = 24;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(*remember_me);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }

  {
    uint32_t field_tag = 34;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(provider);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }


  return buffer;
}

bool LoginRequest::deserialize(const std::vector<uint8_t>& data) {
  size_t pos = 0;
  while (pos < data.size()) {
    if (pos + 1 > data.size()) return false;

    uint64_t tag = 0;
    {
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        tag |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
    }

    uint32_t field_number = static_cast<uint32_t>(tag >> 3);
    uint8_t wire_type = static_cast<uint8_t>(tag & 0x7);

    switch (field_number) {
    case 1: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      username.assign(reinterpret_cast<const char*>(&data[pos]), length);
      pos += length;
      break;
    }
    case 2: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      password.assign(reinterpret_cast<const char*>(&data[pos]), length);
      pos += length;
      break;
    }
    case 3: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      remember_me = static_cast<bool>(value);
      break;
    }
    case 4: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      provider = static_cast<AuthProvider>(value);
      break;
    }
    default:
      // Skip unknown field
      if (wire_type == 0) {
        while (pos < data.size() && (data[pos] & 0x80)) pos++;
        if (pos < data.size()) pos++;
      } else if (wire_type == 2) {
        uint64_t length = 0;
        int shift = 0;
        while (pos < data.size()) {
          uint8_t byte = data[pos++];
          length |= static_cast<uint64_t>(byte & 0x7F) << shift;
          if ((byte & 0x80) == 0) break;
          shift += 7;
        }
        pos += length;
      } else if (wire_type == 1) {
        pos += 8;
      } else if (wire_type == 5) {
        pos += 4;
      }
      break;
    }
  }
  return true;
}

std::vector<uint8_t> LoginResponse::serialize() const {
  std::vector<uint8_t> buffer;
  buffer.reserve(64);

  {
    uint32_t field_tag = 8;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    buffer.push_back(success ? 1 : 0);
  }

  if (token.has_value()) {
    uint32_t field_tag = 18;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t length = token->size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), token->begin(), token->end());
  }

  if (user.has_value()) {
    uint32_t field_tag = 26;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    auto field_data = user->serialize();
    uint64_t length = field_data.size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), field_data.begin(), field_data.end());
  }

  if (error_message.has_value()) {
    uint32_t field_tag = 34;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t length = error_message->size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), error_message->begin(), error_message->end());
  }

  {
    uint32_t field_tag = 40;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(expires_at);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }

  if (session_data.has_value()) {
    uint32_t field_tag = 50;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(*session_data);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }


  return buffer;
}

bool LoginResponse::deserialize(const std::vector<uint8_t>& data) {
  size_t pos = 0;
  while (pos < data.size()) {
    if (pos + 1 > data.size()) return false;

    uint64_t tag = 0;
    {
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        tag |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
    }

    uint32_t field_number = static_cast<uint32_t>(tag >> 3);
    uint8_t wire_type = static_cast<uint8_t>(tag & 0x7);

    switch (field_number) {
    case 1: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      success = static_cast<bool>(value);
      break;
    }
    case 2: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      token = std::string(reinterpret_cast<const char*>(&data[pos]), length);
      pos += length;
      break;
    }
    case 3: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      User value;
      std::vector<uint8_t> item_data(data.begin() + pos, data.begin() + pos + length);
      if (!value.deserialize(item_data)) return false;
      user = std::move(value);
      pos += length;
      break;
    }
    case 4: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      error_message = std::string(reinterpret_cast<const char*>(&data[pos]), length);
      pos += length;
      break;
    }
    case 5: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      expires_at = static_cast<uint64_t>(value);
      break;
    }
    case 6: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      session_data = static_cast<uint8_t>(value);
      break;
    }
    default:
      // Skip unknown field
      if (wire_type == 0) {
        while (pos < data.size() && (data[pos] & 0x80)) pos++;
        if (pos < data.size()) pos++;
      } else if (wire_type == 2) {
        uint64_t length = 0;
        int shift = 0;
        while (pos < data.size()) {
          uint8_t byte = data[pos++];
          length |= static_cast<uint64_t>(byte & 0x7F) << shift;
          if ((byte & 0x80) == 0) break;
          shift += 7;
        }
        pos += length;
      } else if (wire_type == 1) {
        pos += 8;
      } else if (wire_type == 5) {
        pos += 4;
      }
      break;
    }
  }
  return true;
}

std::vector<uint8_t> Session::serialize() const {
  std::vector<uint8_t> buffer;
  buffer.reserve(64);

  {
    uint32_t field_tag = 16;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(user_id);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }

  {
    uint32_t field_tag = 24;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(created_at);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }

  {
    uint32_t field_tag = 32;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t val = static_cast<uint64_t>(expires_at);
    while (val > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
      val >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(val));
  }

  {
    uint32_t field_tag = 42;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t length = ip_address.size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), ip_address.begin(), ip_address.end());
  }

  if (user_agent.has_value()) {
    uint32_t field_tag = 50;
    {
      uint64_t val = field_tag;
      while (val > 0x7F) {
        buffer.push_back(static_cast<uint8_t>((val & 0x7F) | 0x80));
        val >>= 7;
      }
      buffer.push_back(static_cast<uint8_t>(val));
    }
    uint64_t length = user_agent->size();
    while (length > 0x7F) {
      buffer.push_back(static_cast<uint8_t>((length & 0x7F) | 0x80));
      length >>= 7;
    }
    buffer.push_back(static_cast<uint8_t>(length));
    buffer.insert(buffer.end(), user_agent->begin(), user_agent->end());
  }


  return buffer;
}

bool Session::deserialize(const std::vector<uint8_t>& data) {
  size_t pos = 0;
  while (pos < data.size()) {
    if (pos + 1 > data.size()) return false;

    uint64_t tag = 0;
    {
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        tag |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
    }

    uint32_t field_number = static_cast<uint32_t>(tag >> 3);
    uint8_t wire_type = static_cast<uint8_t>(tag & 0x7);

    switch (field_number) {
    case 2: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      user_id = static_cast<uint64_t>(value);
      break;
    }
    case 3: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      created_at = static_cast<uint64_t>(value);
      break;
    }
    case 4: {
      uint64_t value = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      expires_at = static_cast<int64_t>(value);
      break;
    }
    case 5: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      ip_address.assign(reinterpret_cast<const char*>(&data[pos]), length);
      pos += length;
      break;
    }
    case 6: {
      uint64_t length = 0;
      int shift = 0;
      while (pos < data.size()) {
        uint8_t byte = data[pos++];
        length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) break;
        shift += 7;
      }
      user_agent = std::string(reinterpret_cast<const char*>(&data[pos]), length);
      pos += length;
      break;
    }
    default:
      // Skip unknown field
      if (wire_type == 0) {
        while (pos < data.size() && (data[pos] & 0x80)) pos++;
        if (pos < data.size()) pos++;
      } else if (wire_type == 2) {
        uint64_t length = 0;
        int shift = 0;
        while (pos < data.size()) {
          uint8_t byte = data[pos++];
          length |= static_cast<uint64_t>(byte & 0x7F) << shift;
          if ((byte & 0x80) == 0) break;
          shift += 7;
        }
        pos += length;
      } else if (wire_type == 1) {
        pos += 8;
      } else if (wire_type == 5) {
        pos += 4;
      }
      break;
    }
  }
  return true;
}

} // namespace examples_auth
