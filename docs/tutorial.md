# SerialKit Tutorial

A step-by-step guide to getting started with SerialKit binary serialization.

## Table of Contents

1. [Installation](#installation)
2. [Your First Schema](#your-first-schema)
3. [Generating Code](#generating-code)
4. [Using Generated Code](#using-generated-code)
5. [Working with Enums](#working-with-enums)
6. [Optional and Repeated Fields](#optional-and-repeated-fields)
7. [Nested Models](#nested-models)
8. [Optimizations](#optimizations)
9. [Best Practices](#best-practices)

## Installation

### Building from Source

```bash
git clone https://github.com/khasanyanovk/serial-kit.git
cd serial-kit
mkdir build && cd build
cmake ..
cmake --build .
```

The compiler will be available at `build/bin/serialkit-compiler` (or `.exe` on Windows).

### Verify Installation

```bash
./build/bin/serialkit-compiler --version
# Output: SerialKit Compiler version 1.0.0
```

## Your First Schema

Create a file `person.skit`:

```cpp
namespace demo;

model Person {
    string name = 1;
    uint32 age = 2;
    string email = 3;
}
```

### Schema Syntax Rules

- Schema files use `.skit` extension
- Each file starts with a `namespace` declaration
- Models contain fields with: `type name = field_number;`
- Field numbers must be unique within a model (1-536870911)
- Field numbers 19000-19999 are reserved

## Generating Code

Generate C++ code from your schema:

```bash
./build/bin/serialkit-compiler person.skit -o generated/
```

This creates two files:
- `generated/demo.hpp` - Header with class definitions
- `generated/demo.cpp` - Implementation of serialize/deserialize

### Compiler Options

```bash
# Custom output filename
./build/bin/serialkit-compiler person.skit -o gen/ -f person_schema

# Verbose output
./build/bin/serialkit-compiler person.skit -o gen/ --verbose
```

## Using Generated Code

Create `main.cpp`:

```cpp
#include "generated/demo.hpp"
#include <iostream>
#include <fstream>

int main() {
    // Create and populate
    demo::Person person;
    person.name = "Alice Smith";
    person.age = 30;
    person.email = "alice@example.com";
    
    // Serialize to binary
    std::vector<uint8_t> data = person.serialize();
    std::cout << "Serialized size: " << data.size() << " bytes\n";
    
    // Save to file
    std::ofstream file("person.bin", std::ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
    
    // Load from file
    std::ifstream infile("person.bin", std::ios::binary);
    infile.seekg(0, std::ios::end);
    size_t size = infile.tellg();
    infile.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> loaded_data(size);
    infile.read(reinterpret_cast<char*>(loaded_data.data()), size);
    infile.close();
    
    // Deserialize
    demo::Person loaded_person;
    if (loaded_person.deserialize(loaded_data)) {
        std::cout << "Name: " << loaded_person.name << "\n";
        std::cout << "Age: " << loaded_person.age << "\n";
        std::cout << "Email: " << loaded_person.email << "\n";
    }
    
    return 0;
}
```

Compile and run:

```bash
g++ -std=c++20 main.cpp generated/demo.cpp -o demo
./demo
```

## Working with Enums

Schema with enum:

```cpp
namespace demo;

enum Role {
    GUEST = 0;
    USER = 1;
    ADMIN = 2;
}

model Account {
    string username = 1;
    Role role = 2;
}
```

Usage:

```cpp
demo::Account account;
account.username = "admin_user";
account.role = demo::Role::ADMIN;

// Serialize
auto data = account.serialize();

// Deserialize
demo::Account loaded;
loaded.deserialize(data);

if (loaded.role == demo::Role::ADMIN) {
    std::cout << "Admin access granted\n";
}
```

## Optional and Repeated Fields

Schema:

```cpp
namespace demo;

model UserProfile {
    string username = 1;
    optional string bio = 2;           // May or may not be present
    repeated string tags = 3;           // Array of strings
    optional uint32 age = 4;
}
```

Usage:

```cpp
demo::UserProfile profile;
profile.username = "john_doe";

// Optional field
profile.bio = "Software developer";  // Sets the optional
// Or check if set:
if (profile.bio.has_value()) {
    std::cout << "Bio: " << *profile.bio << "\n";
}

// Repeated field
profile.tags.push_back("developer");
profile.tags.push_back("opensource");
profile.tags.push_back("cpp");

// Iterate repeated fields
for (const auto& tag : profile.tags) {
    std::cout << "Tag: " << tag << "\n";
}

// Serialize
auto data = profile.serialize();

// Deserialize
demo::UserProfile loaded;
loaded.deserialize(data);

std::cout << "Tags count: " << loaded.tags.size() << "\n";
```

## Nested Models

Schema:

```cpp
namespace demo;

model Address {
    string street = 1;
    string city = 2;
    string country = 3;
}

model Person {
    string name = 1;
    Address address = 2;           // Nested model
    repeated Address prev_addresses = 3;  // Array of nested models
}
```

Usage:

```cpp
demo::Person person;
person.name = "Alice";

// Set nested model
person.address.street = "123 Main St";
person.address.city = "New York";
person.address.country = "USA";

// Add to repeated nested models
demo::Address prev;
prev.street = "456 Oak Ave";
prev.city = "Boston";
prev.country = "USA";
person.prev_addresses.push_back(prev);

// Serialize/deserialize works the same
auto data = person.serialize();
demo::Person loaded;
loaded.deserialize(data);
```

## Optimizations

SerialKit provides three optimization modifiers to reduce binary size.

### Packed Arrays

For numeric and boolean repeated fields:

```cpp
namespace demo;

model Metrics {
    // Without packed: Each value has a tag (2+ bytes per element)
    repeated uint32 values = 1;
    
    // With packed: One tag for entire array (30-70% smaller)
    packed repeated uint32 optimized_values = 2;
}
```

**When to use**: Large arrays of numbers where size matters.

### Interned Strings

For repeated string values (like categories, tags):

```cpp
namespace demo;

model LogEntry {
    // Without interned: Full string each time
    string level = 1;
    
    // With interned: String stored once in table, referenced by index
    interned string module = 2;  // "auth", "db", "api" etc.
    string message = 3;          // Unique messages - don't intern
}
```

**When to use**: Strings that repeat frequently (enums as strings, categories, types).

### Bitmap for Booleans

For boolean arrays:

```cpp
namespace demo;

model FeatureFlags {
    // Without bitmap: 1 byte + tag per boolean
    repeated bool flags = 1;
    
    // With bitmap: Packed into bits (87% smaller)
    bitmap repeated bool feature_enabled = 2;
}
```

**When to use**: Arrays of boolean flags or binary features.

### Optimization Example

```cpp
namespace demo;

model TelemetryData {
    uint64 timestamp = 1;
    interned string sensor_type = 2;      // "temperature", "pressure", etc.
    packed repeated float readings = 3;   // [23.5, 23.6, 23.7, ...]
    bitmap repeated bool alerts = 4;      // [false, false, true, false]
}
```

## Best Practices

### 1. Field Numbering

```cpp
// ✅ GOOD: Sequential numbering
model User {
    string name = 1;
    uint32 id = 2;
    string email = 3;
}

// ❌ AVOID: Large gaps waste tag space
model User {
    string name = 1;
    uint32 id = 1000;
    string email = 5000;
}
```

### 2. Field Types

```cpp
// ✅ GOOD: Use appropriate integer sizes
model SmallData {
    uint8 count = 1;      // 0-255
    uint32 user_id = 2;   // Up to 4 billion
}

// ❌ WASTEFUL: uint64 when uint32 is enough
model SmallData {
    uint64 count = 1;     // Overkill for small counts
}
```

### 3. Optional vs Required

```cpp
// ✅ GOOD: Optional for truly optional data
model UserProfile {
    string username = 1;           // Always present
    optional string avatar = 2;    // May be missing
}

// ❌ AVOID: Everything optional reduces type safety
model UserProfile {
    optional string username = 1;  // Username should be required
}
```

### 4. Optimization Usage

```cpp
// ✅ GOOD: Optimize high-volume data
model SensorData {
    packed repeated float values = 1;     // 1000s of readings
    interned string sensor_type = 2;      // "temp", "pressure"
}

// ❌ WASTEFUL: Optimizing unique data
model Message {
    interned string content = 1;  // Each message is unique!
}
```

### 5. Namespaces

```cpp
// ✅ GOOD: Descriptive namespaces
namespace myapp_telemetry;
namespace company_auth_v2;

// ❌ AVOID: Too generic or nested dots
namespace data;  // Too vague
namespace my.app.module.v1;  // Use underscores, not dots
```

## Next Steps

- Read [DSL Reference](dsl_reference.md) for complete syntax
- Check [API Reference](api_reference.md) for generated code API
- See [examples/](../examples/) for real-world schemas
- Review [WIRE_FORMAT.md](WIRE_FORMAT.md) for protocol details

## Troubleshooting

### Compilation Errors

```bash
# Error: Unknown option
./serialkit-compiler --input=file.skit
# Fix: Use -o not --input
./serialkit-compiler file.skit -o gen/
```

### Deserialization Fails

```cpp
if (!object.deserialize(data)) {
    std::cerr << "Deserialization failed - corrupted data?\n";
}
```

Check:
- Data wasn't truncated
- Using same schema version for serialize/deserialize
- Binary data wasn't modified

### Missing Fields After Deserialize

- Optional fields may not be set - check with `.has_value()`
- Field numbers must match between serialize and deserialize
- Check that field exists in schema

## Support

- Open issues on GitHub
- Check existing examples in `examples/`
- Review test files in `tests/` for usage patterns
