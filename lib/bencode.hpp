#pragma once

#include <cstddef>
#include <expected>
#include <span>

enum class ParseError {
  incomplete,
  invalid,
  length_overflow,
};

using ByteView = std::span<const std::byte>;

struct DecodedByteString {
  ByteView value;
  std::size_t consumed;
};

std::expected<DecodedByteString, ParseError>
decode_bencode_string(ByteView input_stream);
