#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

enum class ParseError {
  incomplete,
  invalid,
  length_overflow,
  integer_out_of_range
};

using ByteView = std::span<const std::byte>;

struct DecodedByteString {
  ByteView value;
  std::size_t consumed;
};

std::expected<DecodedByteString, ParseError>
decode_bencode_string(ByteView input);

struct DecodedInteger {
  std::int64_t value;
  std::size_t consumed;
};

std::expected<DecodedInteger, ParseError>
decode_bencode_integer(ByteView input);
