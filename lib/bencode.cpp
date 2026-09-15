#include "bencode.hpp"

#include <cstddef>
#include <expected>
#include <limits>

std::expected<DecodedByteString, ParseError>
decode_bencode_string(ByteView input) {
  if (input.empty())
    return std::unexpected{ParseError::incomplete};

  std::size_t cursor = 0;
  std::size_t length = 0;

  constexpr auto maximum = std::numeric_limits<std::size_t>::max();

  while (cursor < input.size()) {
    const auto character = std::to_integer<unsigned char>(input[cursor]);

    if (character == ':') {

      if (cursor == 0) {
        return std::unexpected{ParseError::invalid};
      }

      break;
    }

    if (character < '0' || character > '9') {
      return std::unexpected{ParseError::invalid};
    }

    const auto digit = static_cast<std::size_t>(character - '0');

    if (length > (maximum - digit) / 10) {
      return std::unexpected{ParseError::length_overflow};
    }

    length = length * 10 + digit;

    ++cursor;
  }

  if (cursor == input.size()) {
    return std::unexpected{ParseError::incomplete};
  }

  const std::size_t payload_start = cursor + 1;
  const std::size_t available = input.size() - payload_start;

  if (length > available) {
    return std::unexpected{ParseError::incomplete};
  }

  const ByteView value = input.subspan(payload_start, length);
  const std::size_t consumed = payload_start + length;

  return DecodedByteString{value, consumed};
}
