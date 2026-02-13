# Contributing to SerialKit

Thank you for your interest in contributing to SerialKit! This document provides guidelines for contributing to the project.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/your-username/serial-kit.git
   cd serial-kit
   ```
3. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Setup

### Prerequisites

- CMake 3.15+
- C++20 compatible compiler
  - GCC 10+
  - Clang 10+
  - MSVC 2019+

### Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Running Tests

```bash
cd build
ctest --output-on-failure
```

All tests should pass before submitting a PR.

## Code Style

### C++ Guidelines

- **C++20 standard** - use modern C++ features
- **Naming conventions**:
  - Classes: `PascalCase`
  - Functions/methods: `snake_case()`
  - Variables: `snake_case`
  - Constants: `UPPER_SNAKE_CASE`
  - Private members: trailing underscore `member_`

- **Formatting**:
  - 2 spaces for indentation (no tabs)
  - 80-100 character line limit
  - Braces on same line for functions
  - Use `auto` when type is obvious

### Example

```cpp
class CodeGenerator {
public:
  std::string generate_header();
  
private:
  void generate_field(const Field& field);
  const Schema& schema_;
};
```

## Project Structure

```
serial-kit/
├── compiler/        # Schema compiler
│   ├── include/    # Public headers
│   └── source/     # Implementation
├── tests/          # Unit tests
├── examples/       # Schema examples
└── docs/           # Documentation
```

## Making Changes

### 1. Compiler Changes

If modifying the compiler (`compiler/`):

- Update relevant tests in `tests/`
- Ensure all 103 tests pass
- Update documentation if adding features
- Add example schema if appropriate

### 2. Adding Features

For new features:

1. **Discuss first**: Open an issue to discuss the feature
2. **Write tests**: Add tests before implementation
3. **Implement**: Write the code
4. **Document**: Update relevant documentation
5. **Examples**: Add examples if needed

### 3. Bug Fixes

For bug fixes:

1. **Add test**: Create a failing test that reproduces the bug
2. **Fix**: Implement the fix
3. **Verify**: Ensure the test now passes
4. **Document**: Update docs if behavior changes

## Testing

### Writing Tests

Tests use Google Test framework:

```cpp
TEST_F(CodeGenTest, TestName) {
  // Arrange
  Schema schema = create_test_schema();
  
  // Act
  CodeGenerator gen(schema);
  std::string result = gen.generate_header();
  
  // Assert
  EXPECT_TRUE(result.find("class User") != std::string::npos);
}
```

### Test Categories

- `LexerTest` - Tokenization tests
- `ParserTest` - AST parsing tests
- `ValidatorTest` - Schema validation tests
- `CodeGenTest` - Code generation tests
- `ArgParserTest` - CLI argument parsing

### Running Specific Tests

```bash
./bin/serialkit_tests --gtest_filter="CodeGenTest.*"
./bin/serialkit_tests --gtest_filter="*EnumGeneration*"
```

## Documentation

Update documentation when making changes:

- **README.md** - For major features or changes
- **docs/tutorial.md** - For user-facing features
- **docs/dsl_reference.md** - For DSL syntax changes
- **docs/api_reference.md** - For generated API changes
- **docs/WIRE_FORMAT.md** - For encoding changes

## Submitting Changes

### Pull Request Process

1. **Ensure tests pass**:
   ```bash
   cd build
   ctest
   ```

2. **Write clear commit messages**:
   ```
   Add support for fixed-size arrays
   
   - Implement fixed<N> type in parser
   - Add validation for size parameter
   - Generate std::array<T, N> in codegen
   - Add tests for fixed-size arrays
   ```

3. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

4. **Create Pull Request** on GitHub

5. **Address review comments**

### PR Checklist

- [ ] All tests pass (`ctest`)
- [ ] Code follows style guidelines
- [ ] Documentation updated
- [ ] Commit messages are clear
- [ ] No unrelated changes included
- [ ] Examples added if appropriate

## Areas for Contribution

### Good First Issues

- Add more example schemas
- Improve error messages
- Add validation rules
- Documentation improvements
- Performance optimizations

### Advanced Features

- Additional wire format optimizations
- Schema migration tools
- Benchmarking framework
- Language bindings (Python, Rust, etc.)
- IDE integration

## Code Review

All submissions go through code review:

- Be respectful and constructive
- Address all review comments
- Be patient - reviews may take time
- Update your PR based on feedback

## Questions?

- **Issues**: Open a GitHub issue
- **Discussions**: Use GitHub Discussions
- **Email**: Contact maintainer

## License

By contributing to SerialKit, you agree that your contributions will be licensed under the MIT License.

## Recognition

Contributors are recognized in:
- GitHub contributors list
- Release notes for significant features
- Project documentation

Thank you for contributing to SerialKit! 🚀
