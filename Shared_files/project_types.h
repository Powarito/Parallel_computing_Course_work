#pragma once

#include <cstdint>
#include <string>

// ========================================
// Type aliases widely used in the program:
// ========================================

// A general-purpose unsigned integer used for IDs, counters, numbers in indexes and tables in this project
using id_type = std::uint32_t;
using big_id_type = std::uint64_t;

using code_type = unsigned char;

// A wide string type used for processing file contents, working with words and file names anywhere in the program
// Warning: std::ctype can't corretly lower non-ASCII char16_t and char32_t characters, so wchar_t is preferred
using string_type = std::wstring;
