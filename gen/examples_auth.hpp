#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace examples_auth {

enum class UserRole : int32_t {
  GUEST = 0,
  USER = 1,
  MODERATOR = 2,
  ADMIN = 3
};

enum class AuthProvider : int32_t {
  LOCAL = 0,
  GOOGLE = 1,
  GITHUB = 2,
  FACEBOOK = 3
};

class User {
public:
  User() = default;

  std::string username;
  std::string email;
  uint64_t user_id = 0;
  UserRole role;
  bool email_verified = false;
  uint64_t created_at = 0;
  std::optional<std::string> avatar_url;
  std::vector<std::string> permissions;
  std::optional<uint8_t> profile_data;

  std::vector<uint8_t> serialize() const;
  bool deserialize(const std::vector<uint8_t>& data);
};

class LoginRequest {
public:
  LoginRequest() = default;

  std::string username;
  std::string password;
  std::optional<bool> remember_me;
  AuthProvider provider;

  std::vector<uint8_t> serialize() const;
  bool deserialize(const std::vector<uint8_t>& data);
};

class LoginResponse {
public:
  LoginResponse() = default;

  bool success = false;
  std::optional<std::string> token;
  std::optional<User> user;
  std::optional<std::string> error_message;
  uint64_t expires_at = 0;
  std::optional<uint8_t> session_data;

  std::vector<uint8_t> serialize() const;
  bool deserialize(const std::vector<uint8_t>& data);
};

class Session {
public:
  Session() = default;

  uint64_t user_id = 0;
  uint64_t created_at = 0;
  int64_t expires_at = 0;
  std::string ip_address;
  std::optional<std::string> user_agent;

  std::vector<uint8_t> serialize() const;
  bool deserialize(const std::vector<uint8_t>& data);
};

} // namespace examples_auth
