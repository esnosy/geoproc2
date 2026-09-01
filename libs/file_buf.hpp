#pragma once

#include <iostream>

#include <cctype>

#include "fast_float.h"

struct File_Buf {
  char *buf = nullptr;
  size_t offset = 0;
  size_t size = 0;
  File_Buf(char *buf, size_t size) : buf(buf), offset(0), size(size) {}
  void skip_spaces() {
    while (offset < size && isspace(buf[offset])) {
      offset++;
    }
  }
  void skip_token() {
    while (offset < size && !isspace(buf[offset])) {
      offset++;
    }
  }
  void skip_until(char delimiter) {
    while (offset < size && buf[offset] != delimiter) {
      offset++;
    }
  }
  template <typename T> void parse_token_as_number(T &out) {
    auto answer = fast_float::from_chars(buf + offset, buf + size, out);
    if (answer.ec != std::errc()) {
      std::cerr << "Failed to parse number" << std::endl;
    }
    offset = answer.ptr - buf;
  }
  bool compare_token(const char *string) {
    while (offset < size && *string) {
      if (buf[offset] != *string) {
        skip_token();
        return false;
      }
      string++;
      offset++;
    }
    return *string == 0;
  }
};