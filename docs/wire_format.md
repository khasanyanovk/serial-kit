# SerialKit Wire Format Specification

Technical specification of SerialKit's binary serialization format.

## Table of Contents

1. [Overview](#overview)
2. [Varint Encoding](#varint-encoding)
3. [Wire Types](#wire-types)
4. [Field Encoding](#field-encoding)
5. [Type Encoding](#type-encoding)
6. [Optimizations](#optimizations)
7. [Examples](#examples)

## Overview

SerialKit uses a tag-based binary format inspired by Protocol Buffers, with additional optimization features.

### Design Goals

- **Compact**: Variable-length encoding for integers
- **Efficient**: Minimal overhead for small values
- **Forward compatible**: Unknown fields can be skipped
- **Self-describing**: Field tags identify data
- **Optimized**: Special encodings for common patterns

### Key Concepts

- **Field Tag**: Identifies field number and wire type
- **Varint**: Variable-length integer encoding
- **Wire Type**: Encoding method for field data
- **Length-delimited**: Size prefix for variable-length data

## Varint Encoding

Variable-length encoding for integers - smaller values use fewer bytes.

### Algorithm

1. Encode 7 bits of value per byte
2. Set MSB (bit 7) if more bytes follow
3. Clear MSB on final byte
4. Little-endian byte order

### Examples

| Value | Binary | Bytes | Hex |
|-------|--------|-------|-----|
| 0 | `00000000` | 1 | `00` |
| 1 | `00000001` | 1 | `01` |
| 127 | `01111111` | 1 | `7F` |
| 128 | `10000000 00000001` | 2 | `80 01` |
| 300 | `10101100 00000010` | 2 | `AC 02` |
| 65535 | `11111111 11111111 00000011` | 3 | `FF FF 03` |

### Encoding Process

```
Value: 300 (decimal) = 0x12C

1. Binary: 0000 0001 0010 1100
2. Split to 7-bit chunks: 0000010 0101100
3. Reverse order: 0101100 0000010
4. Add continuation bits: 10101100 00000010
5. Result: AC 02
```

### Decoding Process

```
Bytes: AC 02

1. Binary: 10101100 00000010
2. Extract 7-bit chunks: 0101100 0000010
3. Reverse and combine: 0000010 0101100
4. Result: 0x12C = 300
```

### Varint Properties

- **1 byte**: 0 to 127
- **2 bytes**: 128 to 16,383
- **3 bytes**: 16,384 to 2,097,151
- **5 bytes**: uint32 max
- **10 bytes**: uint64 max

## Wire Types

Each field tag encodes both the field number and wire type.

### Wire Type Values

| Value | Name | Encoding | Used For |
|-------|------|----------|----------|
| 0 | VARINT | Variable-length integer | int32, int64, uint32, uint64, bool, enum |
| 1 | FIXED64 | 8 bytes | double |
| 2 | LENGTH_DELIMITED | Length prefix + data | string, bytes, nested models |
| 3 | PACKED_ARRAY | Length prefix + packed values | Optimized repeated numerics |
| 5 | FIXED32 | 4 bytes | float |
| 6 | STRING_TABLE | String interning | Optimized repeated strings |
| 7 | BITMAP | Bit-packed booleans | Optimized repeated bools |

### Field Tag Encoding

```
tag = (field_number << 3) | wire_type
```

**Example**:
- Field 1, wire type 0 (VARINT): `(1 << 3) | 0 = 8`
- Field 2, wire type 2 (LENGTH_DELIMITED): `(2 << 3) | 2 = 18`
- Field 3, wire type 0 (VARINT): `(3 << 3) | 0 = 24`

## Field Encoding

### VARINT (Wire Type 0)

Used for: integers, booleans, enums

```
[field_tag:varint] [value:varint]
```

**Example - uint32 field = 150**:
```
Field 1:
  Tag: (1 << 3) | 0 = 8
  Value: 150
  Bytes: 08 96 01
```

**Example - bool field = true**:
```
Field 2:
  Tag: (2 << 3) | 0 = 16
  Value: 1
  Bytes: 10 01
```

### FIXED32 (Wire Type 5)

Used for: float

```
[field_tag:varint] [value:4 bytes little-endian]
```

**Example - float field = 3.14**:
```
Field 3:
  Tag: (3 << 3) | 5 = 29
  Value: 3.14 = 0x4048F5C3
  Bytes: 1D C3 F5 48 40
```

### FIXED64 (Wire Type 1)

Used for: double

```
[field_tag:varint] [value:8 bytes little-endian]
```

### LENGTH_DELIMITED (Wire Type 2)

Used for: strings, models, bytes

```
[field_tag:varint] [length:varint] [data:length bytes]
```

**Example - string field = "hello"**:
```
Field 1:
  Tag: (1 << 3) | 2 = 10
  Length: 5
  Data: "hello" = 68 65 6C 6C 6F
  Bytes: 0A 05 68 65 6C 6C 6F
```

**Example - nested model**:
```
Field 2 = nested User { name = "Bob" }
  Tag: (2 << 3) | 2 = 18
  Model data: 0A 03 42 6F 62  (serialized User)
  Length: 5
  Bytes: 12 05 0A 03 42 6F 62
```

## Type Encoding

### Integers

All integers use varint encoding (wire type 0):

```cpp
uint32 value = 1;    // Field 1
// Bytes: 08 01

uint64 big = 1000000;  // Field 2
// Bytes: 10 C0 84 3D
```

### Floating Point

```cpp
float temp = 23.5;   // Field 1, wire type 5
// Bytes: 0D 00 00 BC 41

double precise = 3.14159;  // Field 2, wire type 1
// Bytes: 11 6E 86 1B F0 F9 21 09 40
```

### Booleans

Encoded as varint (0 = false, 1 = true):

```cpp
bool flag = true;   // Field 1
// Bytes: 08 01
```

### Strings

Length-delimited:

```cpp
string name = "Alice";  // Field 1
// Bytes: 0A 05 41 6C 69 63 65
//        ^  ^  ^-----------^
//        |  |  string data
//        |  length (5)
//        tag (field 1, type 2)
```

### Enums

Encoded as varint with enum's integer value:

```cpp
enum Status { ACTIVE = 1, INACTIVE = 0 }
Status status = ACTIVE;  // Field 1
// Bytes: 08 01
```

### Optional Fields

Only serialized if value is present:

```cpp
optional string bio = "Developer";  // Field 1
// Bytes: 0A 09 44 65 76 65 6C 6F 70 65 72

optional string bio;  // Not set
// Bytes: (nothing - field omitted)
```

### Repeated Fields (Standard)

Each element has its own tag:

```cpp
repeated uint32 ids = [10, 20, 30];  // Field 1
// Bytes:
//   08 0A  (field 1, value 10)
//   08 14  (field 1, value 20)
//   08 1E  (field 1, value 30)
```

### Nested Models

Serialized as length-delimited:

```cpp
model Address {
    string city = 1;
}
model Person {
    Address addr = 1;
}

Person p;
p.addr.city = "NYC";
// Person serialization:
//   0A 05  (field 1, length 5)
//   0A 03 4E 59 43  (Address: field 1 = "NYC")
```

## Optimizations

SerialKit includes three optimization types.

### 1. Packed Arrays (Wire Type 3)

Standard repeated numeric fields waste bytes on per-element tags:

```
repeated uint32 values = [1, 2, 3]
Standard: 08 01  08 02  08 03  (6 bytes)
```

Packed encoding uses one tag for all elements:

```
packed repeated uint32 values = [1, 2, 3]
Packed: 1A 03 01 02 03  (5 bytes)
        ^  ^  ^-----^
        |  |  values
        |  length
        tag (field 3, type 3)
```

#### Packed Encoding

```
[field_tag:varint with wire_type=3]
[total_length:varint]
[value1:varint]
[value2:varint]
...
```

#### Savings

- **Small values**: ~50% size reduction
- **Large arrays**: ~40-70% reduction depending on values

### 2. String Interning (Wire Type 6)

For repeated strings (like categories, log levels):

**Without interning**:
```
string level = "ERROR";  // repeated many times
string level = "ERROR";
string level = "ERROR";
// Each: 0A 05 45 52 52 4F 52 (7 bytes) = 21 bytes total
```

**With interning**:
```
interned string level = "ERROR";
// First occurrence: Adds to string table
// Subsequent: Reference by index
// String table: ["ERROR"]
// References use varint index instead of full string
```

#### String Table Format

```
[string_table_tag]  // Special field for table
[table_length:varint]
[num_strings:varint]
  [string1_length:varint] [string1_data]
  [string2_length:varint] [string2_data]
  ...

[field_tag with wire_type=6]
[string_index:varint]  // Index into table
```

#### Savings

- **40-60%** for frequently repeated strings
- More effective with more repetition

### 3. Bitmap Booleans (Wire Type 7)

Standard repeated bools use 1 byte each:

```
repeated bool flags = [true, false, true, false, true, true, false, false]
Standard: 08 01  08 00  08 01  08 00  08 01  08 01  08 00  08 00  (16 bytes)
```

Bitmap packing uses 1 byte per 8 bools:

```
bitmap repeated bool flags = [T, F, T, F, T, T, F, F]
Bitmap: 3A 01 35  (3 bytes)
        ^  ^  ^
        |  |  0b00110101 = TFTTFFTF (reversed for little endian)
        |  length (1 byte of bits)
        tag (field 7, type 7)
```

#### Bitmap Encoding

```
[field_tag:varint with wire_type=7]
[num_bytes:varint]
[byte1] [byte2] ... [byteN]
```

Each byte packs 8 bools (bit 0 = first bool, bit 7 = 8th bool).

#### Savings

- **87.5%** size reduction (1 bit vs 1 byte per bool)

## Complete Examples

### Example 1: Simple Model

Schema:
```cpp
model User {
    string name = 1;
    uint32 id = 2;
    bool active = 3;
}
```

Data:
```cpp
User user;
user.name = "Alice";
user.id = 42;
user.active = true;
```

Binary encoding:
```
Field 1 (name = "Alice"):
  0A 05 41 6C 69 63 65

Field 2 (id = 42):
  10 2A

Field 3 (active = true):
  18 01

Complete: 0A 05 41 6C 69 63 65 10 2A 18 01
          (11 bytes total)
```

### Example 2: Optional Fields

Schema:
```cpp
model Profile {
    string username = 1;
    optional string bio = 2;
    optional uint32 age = 3;
}
```

With all fields:
```cpp
Profile p;
p.username = "alice";
p.bio = "Developer";
p.age = 30;

Binary: 0A 05 61 6C 69 63 65  12 09 44 65 76 65 6C 6F 70 65 72  18 1E
```

Without optional fields:
```cpp
Profile p;
p.username = "alice";
// bio and age not set

Binary: 0A 05 61 6C 69 63 65
        (only username field present)
```

### Example 3: Repeated Fields with Optimization

Schema:
```cpp
model Data {
    packed repeated uint32 values = 1;
    bitmap repeated bool flags = 2;
}
```

Data:
```cpp
Data data;
data.values = {10, 20, 30, 40};
data.flags = {true, false, true, true, false, false, true, false};
```

Binary:
```
Field 1 (packed values):
  0A 04 0A 14 1E 28
  ^  ^  ^---------^
  |  |  varint values
  |  length (4 bytes)
  tag (field 1, wire type 3)

Field 2 (bitmap flags):
  12 01 4D
  ^  ^  ^
  |  |  bits: 0b01001101 = TFTTFFTF
  |  length (1 byte)
  tag (field 2, wire type 7)

Complete: 0A 04 0A 14 1E 28  12 01 4D
          (11 bytes)

Without optimization would be: ~30 bytes
Savings: 63%
```

## Compatibility

### Forward Compatibility

Newer schemas can add fields without breaking old deserializers:

```cpp
// Old schema
model User {
    string name = 1;
}

// New schema
model User {
    string name = 1;
    uint32 id = 2;     // Added field
}
```

Old deserializer will skip field 2 (unknown field).

### Backward Compatibility

Old schemas can read data from newer schemas:

- New required fields: Will be set to defaults
- New optional fields: Will be unset
- New repeated fields: Will be empty

### Breaking Changes

❌ Don't do these:
- Change field numbers
- Change field types
- Remove required fields
- Change wire type encoding

## Performance Characteristics

### Encoding Overhead

- **Field tag**: 1-5 bytes (usually 1-2)
- **Varint**: 1-10 bytes depending on value
  - 0-127: 1 byte
  - 128-16383: 2 bytes
  - Most uint32: 5 bytes max
- **Length prefix**: 1-5 bytes (usually 1-2 for strings < 16KB)

### Optimizations Impact

| Optimization | Best Case | Typical | Use When |
|--------------|-----------|---------|----------|
| packed | 70% smaller | 40-60% | Large numeric arrays |
| interned | 80% smaller | 40-60% | Repeated strings |
| bitmap | 87% smaller | 87% | Boolean arrays |

## See Also

- [Tutorial](tutorial.md) - Getting started
- [DSL Reference](dsl_reference.md) - Schema syntax
- [API Reference](api_reference.md) - Using generated code
- [Examples](../examples/) - Real-world schemas
