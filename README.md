# SerialKit

A lightweight, high-performance C++ binary serialization library with code generation from schema definitions.

## Features

- 🚀 **Fast binary serialization** with variable-length integer encoding
- 📝 **Simple DSL** (.skit files) for schema definition
- ⚡ **Automatic code generation** - C++ classes with serialize/deserialize methods
- 🎯 **Modern C++20** with type safety and zero-copy optimizations
- 🔧 **Advanced optimizations** - packed arrays, string interning, bitmap compression
- 🧪 **Cross-platform** support (Linux, macOS, Windows)
- ✅ **Unit tests** - comprehensive test coverage

## Quick Start

### 1. Define your schema (.skit file)

```cpp
namespace myapp;

enum Status {
    INACTIVE = 0;
    ACTIVE = 1;
    SUSPENDED = 2;
}

model User {
    string username = 1;
    uint64 user_id = 2;
    optional string email = 3;
    repeated string tags = 4;
    Status status = 5;
}
```

### 2. Generate C++ code

```bash
# Basic usage
serialkit-compiler input.skit -o generated/

# With custom filename
serialkit-compiler schema.skit -o gen/ -f myapp_schema

# Verbose output
serialkit-compiler input.skit -o output/ --verbose
```

### 3. Use in your code

```cpp
#include "generated/myapp.hpp"  // or myapp_schema.hpp with -f option

int main() {
    myapp::User user;
    user.username = "alice";
    user.user_id = 42;
    user.email = "alice@example.com";
    user.tags.push_back("developer");
    user.tags.push_back("admin");
    user.status = myapp::Status::ACTIVE;
    
    // Serialize to binary
    std::vector<uint8_t> data = user.serialize();
    
    // Deserialize from binary
    myapp::User loaded;
    if (loaded.deserialize(data)) {
        std::cout << loaded.username << std::endl;  // Output: alice
    }
    
    return 0;
}
```

## Building from Source

### Requirements

- CMake 3.15+
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- Git (for fetching Google Test)

### Build Steps

```bash
git clone https://github.com/khasanyanovk/serial-kit.git
cd serial-kit
mkdir build && cd build
cmake ..
cmake --build .
ctest  # Run unit tests
```

### Build Options

```bash
cmake -DSERIALKIT_BUILD_TESTS=OFF ..    # Disable tests
cmake -DSERIALKIT_BUILD_COMPILER=OFF .. # Disable compiler
cmake -DCMAKE_BUILD_TYPE=Release ..     # Release build
```

## Get from installer
Download installer  
https://github.com/khasanyanovk/serial-kit/actions/runs/22002116375/artifacts/5505631736  
Or download actual version from actions artifacts

## Project Structure

```
serial-kit/
├── compiler/          # Schema compiler
│   ├── include/      # Public headers
│   │   ├── lexer.hpp      # Tokenization
│   │   ├── parser.hpp     # AST construction
│   │   ├── ast.hpp        # Abstract Syntax Tree definitions
│   │   ├── validator.hpp  # Schema validation
│   │   ├── codegen.hpp    # C++ code generator
│   │   └── arg_parser.hpp # CLI argument parser
│   ├── source/       # Implementation files
│   └── main.cpp      # Compiler entry point
├── examples/         # 7 schema examples
│   ├── 01_basic_types.skit
│   ├── 02_user_auth.skit
│   ├── 03_network_protocol.skit
│   └── ...
├── tests/            # Unit tests
├── docs/             # Documentation
├── gen/              # Generated C++ files (runtime)
└── CMakeLists.txt    # Build configuration
```

## Supported Types

### Primitive Types
- **Integers**: `int8`, `int16`, `int32`, `int64`
- **Unsigned**: `uint8`, `uint16`, `uint32`, `uint64`
- **Floating**: `float`, `double`
- **Other**: `bool`, `string`, `byte`

### Complex Types
- `enum` - Enumeration types with int32 values
- `model` - Structured types (can be nested)
- `optional` - Optional fields (using `std::optional`)
- `repeated` - Arrays (using `std::vector`)

### Optimization Modifiers
- **`packed repeated`** - Compact numeric/bool arrays without per-element tags
  - 30-70% size reduction for numeric arrays
  - Example: `packed repeated uint32 values = 1;`
  
- **`interned string`** - String deduplication via string table
  - 40-60% size reduction for repeated strings
  - Example: `interned string category = 2;`
  
- **`bitmap repeated bool`** - Bit-packed boolean arrays
  - 87% size reduction for bool arrays
  - Example: `bitmap repeated bool flags = 3;`

## Compiler Options

```
serialkit-compiler [options] <input.skit>

Options:
  -h, --help              Show help message
  -v, --verbose           Enable verbose output
  --version               Show version information
  -o, --output <dir>      Output directory (default: ".")
  -f, --filename <name>   Base filename for generated files (default: namespace name)

Examples:
  serialkit-compiler schema.skit -o generated/
  serialkit-compiler auth.skit -o gen/ -f user_auth --verbose
```

## Documentation

- [Tutorial](docs/tutorial.md) - Step-by-step guide for beginners
- [DSL Reference](docs/dsl_reference.md) - Complete .skit syntax
- [Generated API](docs/api_reference.md) - Using generated C++ code
- [Wire Format](docs/WIRE_FORMAT.md) - Binary protocol specification

## Examples

The `examples/` directory contains 7 complete schema examples:

1. **01_basic_types.skit** - Primitive types demonstration
2. **02_user_auth.skit** - User authentication system
3. **03_network_protocol.skit** - Network packet definitions
4. **04_game_state.skit** - Game state serialization
5. **05_configuration.skit** - Configuration file format
6. **06_iot_sensors.skit** - IoT sensor data
7. **07_optimizations.skit** - Optimization techniques showcase

Generate them with:
```bash
./build/bin/serialkit-compiler examples/02_user_auth.skit -o gen/
```

## Testing

SerialKit includes comprehensive test coverage:

- **Unit tests** covering all components
- Lexer, Parser, Validator, CodeGenerator tests
- Command-line argument parsing tests
- Cross-platform CI/CD (Ubuntu, Windows, macOS)

Run tests:
```bash
cd build
ctest --output-on-failure
```

## Performance

Generated code is optimized for performance:
- **Zero-copy** where possible
- **Inline varint encoding** directly in generated code
- **No runtime dependencies** - self-contained generated files
- **Move semantics** for efficient data handling

Optimization modifiers reduce binary size:
- Packed arrays: **30-70% smaller**
- Interned strings: **40-60% smaller**
- Bitmap bools: **87% smaller**

## Contributing

Contributions are welcome! This project showcases:
- Modern C++20 development practices
- Compiler construction techniques
- Binary serialization protocols
- Software architecture and testing

Please feel free to open issues or submit pull requests.

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Author

**Khasanyanov K.** - C++ Developer

This project demonstrates expertise in:
- Modern C++ (C++20)
- Compiler design (lexer, parser, AST, code generation)
- Binary protocols and serialization
- Software architecture and design patterns
- Test-driven development
