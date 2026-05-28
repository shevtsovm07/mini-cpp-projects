#include <cstdint>
#include "number.h"
#include <ostream>

void ShiftLeft(int2025_t& value) {
  for (int i = kLen - 2; i >= 0; i--) {
    if ((i != kLen - 2) && (value.data[i] & 0b1000'0000)) {
      value.data[i + 1] |= 1;
    }
    value.data[i] <<= 1;
  }
}

bool GetBit(const int2025_t& value, int n) {
  return ((value.data[n / 8] >> (n % 8)) & 1);
}

void SetBit(int2025_t& value, int n, bool bit) {
  int byte_index = n / 8;
  int bit_index = n % 8;
  if (bit) {
    value.data[byte_index] |= (1 << bit_index);
  } else {
    value.data[byte_index] &= ~(1 << bit_index);
  }
}

int CompareAbs(const int2025_t& lhs, const int2025_t& rhs) {
  for (int i = kLen - 1; i >= 0; i--) {
    if (lhs.data[i] > rhs.data[i]) {
      return 1;
    }
    if (lhs.data[i] < rhs.data[i]) {
      return -1;
    }
  }
  return 0;
}

bool IsNegative(const int2025_t& value) {
  return (value.data[kLen - 1] & 0b1000'0000) != 0;
}

int2025_t AbsoluteValue(const int2025_t& value) {
  if (IsNegative(value)) {
    int2025_t result = value;
    for (int j = 0; j < kLen; j++) {
      result.data[j] = ~result.data[j];
    }
    uint8_t carry = 1;
    for (int j = 0; j < kLen; j++) {
      uint16_t sum = result.data[j] + carry;
      result.data[j] = sum & 0b1111'1111;
      carry = sum >> 8;
    }
    return result;
  } else {
    return value;
  }
}

int2025_t from_int(int32_t i) {
  int2025_t result = {0};
  bool negative = i < 0;
  if (negative) {
    i = -i;
  }
  for (int j = 0; j < 4; j++) {
    result.data[j] = i & 0b1111'1111;
    i >>= 8;
  }
  if (negative) {
    for (int j = 0; j < kLen; j++) {
      result.data[j] = ~result.data[j];
    }
    uint8_t carry = 1;
    for (int j = 0; j < kLen; j++) {
      uint16_t sum = result.data[j] + carry;
      result.data[j] = sum & 0b1111'1111;
      carry = sum >> 8;
    }
  }
  return result;
}

int2025_t from_string(const char* buff) {
  int2025_t result = {0};
  if (buff[0] == '\0') {
    return result;
  }
  bool negative = false;
  if (buff[0] == '-') {
    negative = true;
    buff++;
  }
  if (buff[0] == '+') {
    buff++;
  }
  for (int i = 0; buff[i] != '\0'; i++) {
    if (buff[i] >= '0' && buff[i] <= '9') {
      result = result * from_int(10);
      result = result + from_int(buff[i] - '0');
    } else {
      break;
    }
  }
  if (negative) {
    result = from_int(0) - result;
  }
  return result;
}

int2025_t operator+(const int2025_t& lhs, const int2025_t& rhs) {
  int2025_t result = from_int(0);
  uint16_t carry = 0;
  for (int i = 0; i < kLen; i++) {
    uint16_t sum = lhs.data[i] + rhs.data[i] + carry;
    result.data[i] = sum & 0b1111'1111;
    carry = sum >> 8;
  }
  return result;
}

int2025_t operator-(const int2025_t& lhs, const int2025_t& rhs) {
  int2025_t result = from_int(0);
  int2025_t negative_rhs = rhs;
  for (int i = 0; i < kLen; i++) {
    negative_rhs.data[i] = ~negative_rhs.data[i];
  }
  uint16_t carry = 1;
  for (int i = 0; i < kLen; i++) {
    uint16_t sum = negative_rhs.data[i] + carry;
    negative_rhs.data[i] = sum & 0b1111'1111;
    carry = sum >> 8;
  }
  result = lhs + negative_rhs;
  return result;
}

int2025_t operator*(const int2025_t& lhs, const int2025_t& rhs) {
  int2025_t result = {0};
  int2025_t lhs_abs = AbsoluteValue(lhs);
  int2025_t rhs_abs = AbsoluteValue(rhs);

  for (int i = 0; i < kLen; i++) {
    uint16_t carry = 0;
    for (int j = 0; j < kLen; j++) {
      if (i + j >= kLen) break;
      uint32_t temp =
          result.data[i + j] + lhs_abs.data[i] * rhs_abs.data[j] + carry;
      result.data[i + j] = temp & 0b1111'1111;
      carry = temp >> 8;
    }
  }

  if (IsNegative(lhs) != IsNegative(rhs)) {
    for (int i = 0; i < kLen; i++) {
      result.data[i] = ~result.data[i];
    }
    uint16_t carry = 1;
    for (int i = 0; i < kLen; i++) {
      uint16_t sum = result.data[i] + carry;
      result.data[i] = sum & 0b1111'1111;
      carry = sum >> 8;
    }
  }
  return result;
}

int2025_t operator/(const int2025_t& lhs, const int2025_t& rhs) {
  int2025_t result = {0};
  int2025_t abs_dividend = AbsoluteValue(lhs);
  int2025_t abs_divisor = AbsoluteValue(rhs);
  if (rhs == from_int(0)) {
    return from_int(0);
  }
  int2025_t remainder = {0};
  for (int i = ((kLen - 2) * 8); i >= 0; i--) {
    ShiftLeft(remainder);
    remainder = remainder + from_int(GetBit(abs_dividend, i));
    if (CompareAbs(remainder, abs_divisor) >= 0) {
      remainder = remainder - abs_divisor;
      SetBit(result, i, true);
    }
  }

  if (IsNegative(lhs) != IsNegative(rhs)) {
    result = from_int(0) - result;
  }
  return result;
}

bool operator==(const int2025_t& lhs, const int2025_t& rhs) {
  for (int i = 0; i < kLen; i++) {
    if (lhs.data[i] != rhs.data[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const int2025_t& lhs, const int2025_t& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& stream, const int2025_t& value) {
  stream << std::hex;
  bool nulls_only = true;
  for (int i = kLen - 1; i >= 0; i--) {
    if (value.data[i] != 0) {
      nulls_only = false;
    }
    if (!nulls_only) {
      stream << int(value.data[i]);
    }
  }
  if (nulls_only) {
    stream << '0';
  }
  return stream;
}
