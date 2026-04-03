#pragma once

#include "macro_cell.h"

class HashLife {
 private:
  struct LifePolicy {
    using leaf_t = uint16_t;  // 4x4 bits en Z-order

    // ------------------------------------
    static bool equal(const leaf_t& a, const leaf_t& b) { return a == b; }
    static size_t hash(const leaf_t& v) {
      return uint64_t(v) * 0x9E3779B97F4A7C15ULL;
    }

    // ------------------------------------
    static leaf_t empty() { return 0; }
    static leaf_t wrap(const leaf_t& l) {
      leaf_t r = 0;
      for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
          set(r, x, y, get(l, (x + 2) % 4, (y + 2) % 4));
      return r;
    }

    // ------------------------------------
    // Evolve 4x4 central d'une grande cellule 8x8 (Z-order)
    static leaf_t evolve(const leaf_t& a, const leaf_t& b, const leaf_t& c,
                         const leaf_t& d) {
      uint8_t grid[8][8] = {};
      uint8_t temp[8][8] = {};

      // --- unpack Z-order 8x8 ---
      for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++) {
          grid[y][x] = get(a, x, y);
          grid[y][x + 4] = get(b, x, y);
          grid[y + 4][x] = get(c, x, y);
          grid[y + 4][x + 4] = get(d, x, y);
        }

      // --- evolve 8x8 ---
      for (int y = 1; y < 7; y++)
        for (int x = 1; x < 7; x++) {
          int cnt = grid[y - 1][x - 1] + grid[y - 1][x] + grid[y - 1][x + 1] +
                    grid[y][x - 1] + grid[y][x + 1] + grid[y + 1][x - 1] +
                    grid[y + 1][x] + grid[y + 1][x + 1];
          temp[y][x] = (cnt == 3) || (grid[y][x] && cnt == 2);
        }

      // --- central 4x4 ---
      leaf_t res = 0;
      for (int y = 2; y < 6; y++)
        for (int x = 2; x < 6; x++) set(res, x - 2, y - 2, temp[y][x]);

      return res;
    }

    // convert row-major byte array to z-order packed array
    static std::vector<uint16_t> from_byte(const std::vector<uint8_t> data) {
      size_t N = data.size();

      if ((N & (N - 1)) != 0)
        throw std::invalid_argument("input must be power of 2");

      int k = __builtin_ctzll(N);
      if (k & 1) throw std::invalid_argument("input must be square");

      size_t dim = 1u << (k / 2);
      size_t blocks = dim / 4;

      std::vector<leaf_t> out(blocks * blocks);

      for (size_t by = 0; by < blocks; by++) {
        for (size_t bx = 0; bx < blocks; bx++) {
          leaf_t l = 0;

          for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
              size_t gx = bx * 4 + x;
              size_t gy = by * 4 + y;

              uint8_t v = data[gy * dim + gx] & 1;
              set(l, x, y, v);
            }
          }

          out[interleave(bx, by)] = l;
        }
      }

      return out;
    }

    // convert z-order packed array to row-major byte array
    static std::vector<uint8_t> to_byte(const std::vector<uint16_t> data) {
      size_t B = data.size();

      if ((B & (B - 1)) != 0)
        throw std::invalid_argument("input size must be power of 2");

      int k = __builtin_ctzll(B);
      if (k & 1)
        throw std::invalid_argument("input size must be square (2^(2n))");

      size_t blocks = 1u << (k / 2);
      size_t dim = blocks * 4;

      std::vector<uint8_t> out(dim * dim, 0);

      for (size_t by = 0; by < blocks; by++) {
        for (size_t bx = 0; bx < blocks; bx++) {
          leaf_t l = data[interleave(bx, by)];

          for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
              size_t gx = bx * 4 + x;
              size_t gy = by * 4 + y;
              out[gy * dim + gx] = get(l, x, y);
            }
          }
        }
      }

      return out;
    }

   private:
    // interleave pour Z-order
    static inline uint64_t part1by1(uint32_t x) {
      uint64_t v = x;
      v = (v | (v << 16)) & 0x0000FFFF0000FFFFULL;
      v = (v | (v << 8)) & 0x00FF00FF00FF00FFULL;
      v = (v | (v << 4)) & 0x0F0F0F0F0F0F0F0FULL;
      v = (v | (v << 2)) & 0x3333333333333333ULL;
      v = (v | (v << 1)) & 0x5555555555555555ULL;
      return v;
    }

    static inline uint64_t interleave(uint32_t x, uint32_t y) {
      return part1by1(x) | (part1by1(y) << 1);
    }

    // ------------------------------------
    // Accès bit Z-order
    static uint8_t get(const leaf_t& l, int x, int y) {
      int z = (interleave(x, y));
      return (l >> z) & 1;
    }

    static void set(leaf_t& l, int x, int y, uint8_t v) {
      int z = (interleave(x, y));
      l &= ~(1u << z);
      l |= (v & 1u) << z;
    }
  };

  MacroCell<LifePolicy> tree;

 public:
  HashLife() {};
  HashLife(std::vector<uint8_t> grid);
  operator std::vector<uint8_t>() const;
  ~HashLife();
  void evolve(void);
  int cells_count();
  int time_jumps_size();
};

HashLife::HashLife(std::vector<uint8_t> grid) {
  tree = MacroCell<LifePolicy>(LifePolicy::from_byte(grid));
}

HashLife::operator std::vector<uint8_t>() const {
  return LifePolicy::to_byte(static_cast<std::vector<uint16_t>>(tree));
}

HashLife::~HashLife() {}

void HashLife::evolve(void) {
  tree.double_wrap();
  tree.evolve();
}

int HashLife::cells_count(void) { return tree.cells_count(); }

int HashLife::time_jumps_size(void) { return tree.time_jumps_size(); }