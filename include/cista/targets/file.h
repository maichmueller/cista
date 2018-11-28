#pragma once

#include <cstdio>

#include "cista/offset_t.h"
#include "cista/verify.h"

namespace cista {

struct sfile {
  sfile(char const* path, char const* mode) : f_(std::fopen(path, mode)) {
    cista_verify(f_ != nullptr, "unable to open file");
  }

  ~sfile() {
    if (f_ != nullptr) {
      std::fclose(f_);
    }
    f_ = nullptr;
  }

  template <typename T>
  void write(offset_t const pos, T const& val) {
    std::fseek(f_, static_cast<long>(pos), SEEK_SET);
    auto const w = std::fwrite(reinterpret_cast<unsigned char const*>(&val), 1,
                               sizeof(val), f_);
    cista_verify(w == sizeof(val), "write error");
  }

  offset_t write(void const* ptr, offset_t const size, offset_t alignment) {
    auto curr_offset = size_;
    if (alignment != 0) {
      auto unaligned_ptr = reinterpret_cast<void*>(size_);
      auto space = static_cast<size_t>(alignment) * 8u;
      auto const aligned_ptr =
          std::align(alignment, size, unaligned_ptr, space);
      curr_offset =
          aligned_ptr ? reinterpret_cast<offset_t>(aligned_ptr) : curr_offset;
      std::fseek(f_, static_cast<long>(curr_offset), SEEK_SET);
    } else {
      std::fseek(f_, 0, SEEK_END);
    }
    auto const w = std::fwrite(ptr, 1, size, f_);
    cista_verify(w == size, "write error");
    size_ = curr_offset + size;
    return curr_offset;
  }

  offset_t size_ = 0u;
  FILE* f_;
};

}  // namespace cista