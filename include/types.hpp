#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <array>
#include <exception>
#include <cstdio>

using std::string;
using std::vector;
using std::array;
using std::unique_ptr;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef   uint8_t u8;
typedef  int64_t i64;
typedef  int32_t i32;
typedef  int16_t i16;
typedef    int8_t i8;

#define BIT_TEST(n, b) (((n) >> (b)) & 1)