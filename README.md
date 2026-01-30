# SerialKit

A lightweight C++ library for binary serialization with code generation based on DSL schema definitions.

## Features

- 🚀 **Fast binary serialization** with variable-length encoding
- 📝 **Simple DSL** (.skit files) for schema definition
- ⚡ **Code generation** for C++ classes with serialize/deserialize methods
- 🔄 **Schema versioning** and backward compatibility
- 🎯 **Modern C++17/20** with move semantics and zero-copy optimizations
- 🧪 **Cross-platform** support (Linux, macOS, Windows)

## Quick Start

### 1. Define your schema (.skit file)

```cpp
namespace myapp.models;

enum Status {
    INACTIVE = 0;
    ACTIVE = 1;
    SUSPENDED = 2;
}

model User {
    string username = 1;
    int32 user_id = 2;
    optional string email = 3;
    repeated string tags = 4;
    Status status = 5;
}
```

### 2. Generate C++ code

```bash
serialkit-compiler --input=schema.skit --output=./generated
```

### 3. Use in your code

```cpp
#include "generated/User.hpp"

int main() {
    User user;
    user.set_username("alice");
    user.set_user_id(42);
    user.add_tags("developer");
    
    // Serialize
    auto data = user.serialize();
    
    // Deserialize
    User loaded = User::deserialize(data);
    
    std::cout << loaded.username() << std::endl;
    return 0;
}
```

## Building from Source

### Requirements

- CMake 3.15+
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Build

```bash
git clone https://github.com/khasanyanovk/serial-kit.git
cd serial-kit
mkdir build && cd build
cmake ..
cmake --build .
ctest  # Run tests
```

## Project Structure

```
serial-kit/
├── compiler/          # DSL compiler
│   ├── lexer.*       # Tokenization
│   ├── parser.*      # AST construction
│   ├── ast.*         # Abstract Syntax Tree
│   ├── validator.*   # Schema validation
│   └── codegen.*     # C++ code generation
├── runtime/          # Runtime library
│   ├── wire_format.* # Binary format implementation
│   ├── varint.*      # Variable-length integer encoding
│   └── message.*     # Base message class
├── examples/         # Usage examples
├── tests/            # Unit tests
└── docs/             # Documentation
```

## Supported Types

### Primitive Types
- `int8`, `int16`, `int32`, `int64`
- `uint8`, `uint16`, `uint32`, `uint64`
- `byte` (alias for int8)
- `float`, `double`
- `bool`
- `string`, `bytes`

### Complex Types
- `enum` - enumeration types
- `model` - nested structures
- `optional` - optional fields
- `repeated` - arrays

## Documentation

- [DSL Reference](docs/dsl_reference.md) - Complete .skit syntax guide
- [API Reference](docs/api_reference.md) - Generated C++ API
- [Tutorial](docs/tutorial.md) - Step-by-step guide

## Roadmap

- [x] Project setup
- [ ] Varint encoding/decoding
- [ ] Basic serialization runtime
- [ ] Lexer implementation
- [ ] Parser and AST
- [ ] Code generator
- [ ] Support for all primitive types
- [ ] Support for enums
- [ ] Support for nested models
- [ ] Support for repeated fields
- [ ] CLI compiler tool
- [ ] Benchmarks vs Protobuf
- [ ] Complete documentation

## Contributing

Contributions are welcome! This is a portfolio project showcasing modern C++ development practices.

## License

MIT License - see LICENSE file for details

## Author

**Khasanyanov K.** - C++ Developer

Project created to demonstrate:
- Modern C++ (C++17/20)
- Compiler design (lexer, parser, code generation)
- Binary protocols and serialization
- Software architecture and testing
