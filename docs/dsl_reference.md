# SerialKit DSL Reference

Complete reference for the SerialKit schema definition language (.skit files).

## Table of Contents

1. [File Structure](#file-structure)
2. [Namespaces](#namespaces)
3. [Comments](#comments)
4. [Enums](#enums)
5. [Models](#models)
6. [Data Types](#data-types)
7. [Field Modifiers](#field-modifiers)
8. [Field Numbers](#field-numbers)
9. [Naming Conventions](#naming-conventions)
10. [Examples](#examples)

## File Structure

A `.skit` file has the following structure:

```cpp
namespace <namespace_name>;

// Declarations (enums and models)
enum EnumName { ... }
model ModelName { ... }
```

### Rules

- File must start with a `namespace` declaration
- Namespace must end with semicolon
- Declarations can appear in any order
- Multiple enums and models allowed per file

## Namespaces

Defines the C++ namespace for generated code.

### Syntax

```cpp
namespace <identifier>;
```

### Examples

```cpp
namespace myapp;
namespace user_management;
namespace telemetry_v2;
```

### Rules

- Must be the first non-comment line
- Use alphanumeric characters and underscores
- No dots (unlike some other schema languages)
- Generated C++ code uses this as namespace

## Comments

### Line Comments

```cpp
// This is a line comment
namespace myapp; // Comment after code
```

### Block Comments

```cpp
/*
 * Multi-line block comment
 * Can span multiple lines
 */
namespace myapp;

/* Single line block comment */
model User { ... }
```

### Documentation Comments

While the compiler doesn't generate documentation, you can use comments to document your schema:

```cpp
/**
 * User account information
 * Contains authentication and profile data
 */
model User {
    string username = 1;  // Unique username
    string email = 2;     // User's email address
}
```

## Enums

Enumeration types with integer values.

### Syntax

```cpp
enum EnumName {
    VALUE_NAME = number;
    VALUE_NAME_2 = number;
    ...
}
```

### Example

```cpp
enum Status {
    INACTIVE = 0;
    ACTIVE = 1;
    SUSPENDED = 2;
    DELETED = 3;
}
```

### Rules

- Enum names must start with uppercase letter
- Value names typically use UPPER_SNAKE_CASE
- Values must be non-negative integers
- Values must be unique within the enum
- Value names must be unique within the enum
- Each value ends with semicolon
- Generated as `enum class EnumName : int32_t`

### Generated Code

```cpp
// Input
enum Role {
    GUEST = 0;
    USER = 1;
    ADMIN = 2;
}

// Generated
enum class Role : int32_t {
    GUEST = 0,
    USER = 1,
    ADMIN = 2
};
```

## Models

Structured types containing fields.

### Syntax

```cpp
model ModelName {
    [modifiers] type field_name = field_number;
    ...
}
```

### Example

```cpp
model User {
    string username = 1;
    uint64 user_id = 2;
    optional string email = 3;
    repeated string tags = 4;
}
```

### Rules

- Model names must start with uppercase letter
- Field names use snake_case
- Each field ends with semicolon
- Field numbers must be unique within model
- Empty models are allowed (but not recommended)

### Generated Code

```cpp
class User {
public:
    User() = default;

    std::string username;
    uint64_t user_id = 0;
    std::optional<std::string> email;
    std::vector<std::string> tags;

    std::vector<uint8_t> serialize() const;
    bool deserialize(const std::vector<uint8_t>& data);
};
```

## Data Types

### Primitive Types

| Type | C++ Type | Description | Size |
|------|----------|-------------|------|
| `int8` | `int8_t` | Signed 8-bit integer | Varint |
| `int16` | `int16_t` | Signed 16-bit integer | Varint |
| `int32` | `int32_t` | Signed 32-bit integer | Varint |
| `int64` | `int64_t` | Signed 64-bit integer | Varint |
| `uint8` | `uint8_t` | Unsigned 8-bit integer | Varint |
| `uint16` | `uint16_t` | Unsigned 16-bit integer | Varint |
| `uint32` | `uint32_t` | Unsigned 32-bit integer | Varint |
| `uint64` | `uint64_t` | Unsigned 64-bit integer | Varint |
| `float` | `float` | 32-bit floating point | 4 bytes |
| `double` | `double` | 64-bit floating point | 8 bytes |
| `bool` | `bool` | Boolean value | 1 byte |
| `string` | `std::string` | UTF-8 string | Length-delimited |
| `byte` | `uint8_t` | Single byte | Varint |

### User-Defined Types

```cpp
// Enum types
enum Status { ... }
model User {
    Status status = 1;  // Use enum type
}

// Model types (nested)
model Address { ... }
model Person {
    Address address = 1;  // Use model type
}
```

### Type Examples

```cpp
model AllTypes {
    // Integers
    int32 count = 1;
    uint64 id = 2;
    
    // Floating point
    float temperature = 3;
    double precise_value = 4;
    
    // Boolean
    bool is_active = 5;
    
    // String
    string message = 6;
    
    // User-defined
    Status status = 7;
    Address address = 8;
}
```

## Field Modifiers

Modifiers change field behavior and encoding.

### Optional

Makes a field optional (may not be present).

```cpp
model User {
    string username = 1;           // Required
    optional string email = 2;     // Optional
}
```

**Generated**: `std::optional<T>`

**Usage**:
```cpp
User user;
user.email = "test@example.com";  // Set optional
if (user.email.has_value()) {
    std::cout << *user.email;
}
```

### Repeated

Creates an array/list of values.

```cpp
model User {
    repeated string tags = 1;
}
```

**Generated**: `std::vector<T>`

**Usage**:
```cpp
User user;
user.tags.push_back("admin");
user.tags.push_back("verified");
```

### Packed

Optimizes repeated numeric/boolean fields (combines with `repeated`).

```cpp
model Metrics {
    packed repeated uint32 values = 1;
    packed repeated float readings = 2;
}
```

**Effect**: 30-70% size reduction for numeric arrays

**Wire format**: All values packed together, one tag for entire array

**Restrictions**:
- Only with `repeated`
- Only for numeric types and bool
- Not for strings or models

### Interned

Deduplicates repeated strings via string table.

```cpp
model LogEntry {
    interned string category = 1;  // "auth", "db", "api"
    string message = 2;            // Unique - don't intern
}
```

**Effect**: 40-60% size reduction when strings repeat

**Wire format**: Strings stored in table, fields reference by index

**Restrictions**:
- Only for `string` type
- Most effective when values repeat frequently

### Bitmap

Packs boolean arrays into bits.

```cpp
model Features {
    bitmap repeated bool flags = 1;
}
```

**Effect**: 87% size reduction for bool arrays

**Wire format**: 8 bools packed into 1 byte

**Restrictions**:
- Only with `repeated bool`
- Incompatible with `packed`

### Modifier Combinations

```cpp
// ✅ Valid combinations
repeated string tags = 1;
optional string bio = 2;
packed repeated uint32 ids = 3;
bitmap repeated bool flags = 4;
interned string category = 5;

// ❌ Invalid combinations
packed bitmap repeated bool flags = 1;  // Can't combine packed + bitmap
optional repeated string tags = 2;       // Can't combine optional + repeated
packed string text = 3;                  // Packed requires repeated
interned uint32 value = 4;               // Interned only for strings
```

## Field Numbers

Each field must have a unique number within its model.

### Syntax

```cpp
type field_name = NUMBER;
```

### Rules

- **Range**: 1 to 536,870,911
- **Reserved**: 19,000 to 19,999 (cannot be used)
- **Unique**: Within the same model
- **Sequential**: Recommended (1, 2, 3, ...) for efficiency
- **Immutable**: Once assigned, don't change (breaks compatibility)

### Examples

```cpp
// ✅ GOOD: Sequential numbering
model User {
    string name = 1;
    uint32 id = 2;
    string email = 3;
}

// ✅ ALLOWED: Non-sequential (but wastes tag space)
model User {
    string name = 1;
    uint32 id = 10;
    string email = 100;
}

// ❌ INVALID: Duplicate numbers
model User {
    string name = 1;
    uint32 id = 1;  // ERROR: Duplicate field number
}

// ❌ INVALID: Reserved range
model User {
    string name = 19000;  // ERROR: Reserved range
}

// ❌ INVALID: Out of range
model User {
    string name = 536870912;  // ERROR: Too large
}
```

### Field Number Best Practices

1. **Start at 1**: Always begin numbering from 1
2. **Sequential**: Use consecutive numbers (1, 2, 3, ...)
3. **Don't reuse**: Never reuse deleted field numbers
4. **Reserve ranges**: Document ranges for future use
5. **Low numbers first**: Frequently used fields get small numbers (more efficient)

## Naming Conventions

### Recommended Style

```cpp
namespace my_application;

enum Status {           // PascalCase for enum name
    ACTIVE = 0;        // UPPER_SNAKE_CASE for values
    INACTIVE = 1;
}

model UserAccount {     // PascalCase for model name
    string user_name = 1;      // snake_case for fields
    uint32 account_id = 2;
    Status account_status = 3;
}
```

### Namespace Naming

- Use alphanumeric and underscores
- Lowercase recommended: `myapp`, `user_auth`, `telemetry_v2`
- No dots: Use `company_module` not `company.module`

### Enum Naming

- **Enum name**: PascalCase - `Status`, `UserRole`, `LogLevel`
- **Values**: UPPER_SNAKE_CASE - `ACTIVE`, `LOGGED_IN`, `ERROR_STATE`

### Model Naming

- **Model name**: PascalCase - `User`, `LoginRequest`, `SensorData`
- **Fields**: snake_case - `user_name`, `created_at`, `is_active`

## Complete Examples

### Example 1: User Authentication

```cpp
namespace auth;

enum AuthProvider {
    LOCAL = 0;
    GOOGLE = 1;
    GITHUB = 2;
    FACEBOOK = 3;
}

enum UserRole {
    GUEST = 0;
    USER = 1;
    MODERATOR = 2;
    ADMIN = 3;
}

model User {
    string username = 1;
    string email = 2;
    uint64 user_id = 3;
    UserRole role = 4;
    bool email_verified = 5;
    uint64 created_at = 6;
    optional string avatar_url = 7;
    repeated string permissions = 8;
}

model LoginRequest {
    string username = 1;
    string password = 2;
    optional bool remember_me = 3;
    AuthProvider provider = 4;
}

model LoginResponse {
    bool success = 1;
    optional string token = 2;
    optional User user = 3;
    optional string error_message = 4;
}
```

### Example 2: IoT Telemetry with Optimizations

```cpp
namespace iot_telemetry;

enum SensorType {
    TEMPERATURE = 0;
    HUMIDITY = 1;
    PRESSURE = 2;
    MOTION = 3;
}

model SensorReading {
    uint64 timestamp = 1;
    SensorType sensor_type = 2;
    float value = 3;
    interned string location = 4;  // "room_1", "room_2", etc.
    optional string unit = 5;
}

model DeviceStatus {
    string device_id = 1;
    packed repeated float temperature_log = 2;  // Optimized array
    bitmap repeated bool alert_flags = 3;       // Bit-packed
    interned string zone = 4;                   // Deduplicated
}

model TelemetryBatch {
    repeated SensorReading readings = 1;
    uint32 batch_id = 2;
    uint64 collected_at = 3;
}
```

### Example 3: Game State

```cpp
namespace game;

enum PlayerState {
    IDLE = 0;
    MOVING = 1;
    ATTACKING = 2;
    DEAD = 3;
}

model Vector3 {
    float x = 1;
    float y = 2;
    float z = 3;
}

model Player {
    uint32 player_id = 1;
    string name = 2;
    Vector3 position = 3;
    Vector3 velocity = 4;
    PlayerState state = 5;
    uint32 health = 6;
    repeated uint32 inventory = 7;
}

model GameState {
    repeated Player players = 1;
    uint64 timestamp = 2;
    uint32 round_number = 3;
}
```

## Validation Rules

The compiler validates schemas and reports errors for:

### Duplicate Names

```cpp
// ❌ ERROR: Duplicate model name
model User { ... }
model User { ... }

// ❌ ERROR: Duplicate enum name
enum Status { ... }
enum Status { ... }

// ❌ ERROR: Model and enum with same name
model Status { ... }
enum Status { ... }
```

### Duplicate Field Numbers

```cpp
// ❌ ERROR
model User {
    string name = 1;
    uint32 id = 1;  // Duplicate number!
}
```

### Invalid Field Numbers

```cpp
// ❌ ERROR: Out of range
model User {
    string name = 0;           // Too small
    string email = 536870912;  // Too large
    string bio = 19500;        // Reserved range
}
```

### Unknown Types

```cpp
// ❌ ERROR: Status not defined
model User {
    Status status = 1;  // Status enum doesn't exist
}

// ✅ FIX: Define the enum first
enum Status { ACTIVE = 0; }
model User {
    Status status = 1;
}
```

### Invalid Modifiers

```cpp
// ❌ ERROR: packed requires repeated
model Data {
    packed uint32 value = 1;
}

// ❌ ERROR: interned only for strings
model Data {
    interned uint32 value = 1;
}

// ❌ ERROR: bitmap requires repeated bool
model Data {
    bitmap bool flag = 1;
}

// ❌ ERROR: can't combine packed and bitmap
model Data {
    packed bitmap repeated bool flags = 1;
}
```

## Migration and Versioning

### Adding Fields

Safe - always backward compatible:

```cpp
// Version 1
model User {
    string name = 1;
}

// Version 2 - added field
model User {
    string name = 1;
    uint32 age = 2;  // ✅ Safe to add
}
```

### Removing Fields

Keep the number reserved:

```cpp
// Version 1
model User {
    string name = 1;
    string old_field = 2;
}

// Version 2 - removed field
model User {
    string name = 1;
    // Reserved: 2 was old_field - don't reuse!
}
```

### Changing Field Types

❌ **DANGEROUS** - can break compatibility:

```cpp
// Version 1
model User {
    uint32 age = 1;
}

// Version 2
model User {
    string age = 1;  // ❌ Changed type - will fail!
}
```

Don't change field types. Add a new field instead.

## See Also

- [Tutorial](tutorial.md) - Step-by-step guide
- [API Reference](api_reference.md) - Generated C++ code usage
- [Wire Format](WIRE_FORMAT.md) - Binary encoding details
- [Examples](../examples/) - Real-world schemas
