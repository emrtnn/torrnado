#include "bencode.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <span>
#include <string>
#include <string_view>

namespace {

ByteView as_bytes(std::string_view value) {
  return std::as_bytes(std::span{value});
}

void expect_success(std::string_view encoded, std::string_view expected_value,
                    std::size_t expected_consumed) {
  const auto result = decode_bencode_string(as_bytes(encoded));

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->consumed, expected_consumed);

  const ByteView expected_bytes = as_bytes(expected_value);
  ASSERT_EQ(result->value.size(), expected_bytes.size());

  for (std::size_t index = 0; index < expected_bytes.size(); ++index) {
    EXPECT_EQ(std::to_integer<unsigned int>(result->value[index]),
              std::to_integer<unsigned int>(expected_bytes[index]))
        << "byte index " << index;
  }
}

void expect_error(std::string_view encoded, ParseError expected_error) {
  const auto result = decode_bencode_string(as_bytes(encoded));

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), expected_error);
}

} // namespace

TEST(DecodeBencodeString, DecodesByteString) {
  expect_success("4:spam", "spam", 6);
}

TEST(DecodeBencodeString, DecodesEmptyByteString) {
  expect_success("0:", "", 2);
}

TEST(DecodeBencodeString, LeavesTrailingInputUnconsumed) {
  expect_success("4:spamNEXT", "spam", 6);
}

TEST(DecodeBencodeString, TreatsColonAsPayloadData) {
  expect_success("1::", ":", 3);
}

TEST(DecodeBencodeString, PreservesEmbeddedNullBytes) {
  const std::string_view encoded{"3:a\0b", 5};
  const std::string_view expected_value{"a\0b", 3};

  expect_success(encoded, expected_value, 5);
}

TEST(DecodeBencodeString, ReportsEmptyInputAsIncomplete) {
  expect_error("", ParseError::incomplete);
}

TEST(DecodeBencodeString, ReportsMissingColonAsIncomplete) {
  expect_error("12", ParseError::incomplete);
}

TEST(DecodeBencodeString, ReportsTruncatedPayloadAsIncomplete) {
  expect_error("4:spa", ParseError::incomplete);
}

TEST(DecodeBencodeString, RejectsEmptyLength) {
  expect_error(":spam", ParseError::invalid);
}

TEST(DecodeBencodeString, RejectsNonDigitInLength) {
  expect_error("12x:payload", ParseError::invalid);
}

TEST(DecodeBencodeString, RejectsAnotherBencodeType) {
  expect_error("i42e", ParseError::invalid);
}

TEST(DecodeBencodeString, ReportsLengthOverflow) {
  const std::string encoded =
      std::to_string(std::numeric_limits<std::size_t>::max()) + "0:";

  expect_error(encoded, ParseError::length_overflow);
}
