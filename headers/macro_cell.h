#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <unordered_map>
#include <vector>

template<typename Policy>
class MacroCell {
 private:
  using cell_id_t = uint32_t;
  using leaf_t = typename Policy::leaf_t;

  struct cell_t {
    int n;
    union {
      leaf_t value;
      uint32_t child_ids[4];
    };

    bool operator==(const cell_t& other) const {
      if (n == 0) return Policy::equal(value, other.value);
      if (n != other.n) return false;
      return std::memcmp(child_ids, other.child_ids, sizeof(child_ids)) == 0;
    }
  };

  struct cell_hash {
    size_t operator()(const cell_t& c) const {
      if (c.n == 0) return Policy::hash(c.value);
      uint64_t h = uint64_t(c.n) * 0x9E3779B97F4A7C15ULL;

      h ^= uint64_t(c.child_ids[0]) + 0x9E3779B97F4A7C15ULL + (h << 6) +
           (h >> 2);
      h ^= uint64_t(c.child_ids[1]) + 0x9E3779B97F4A7C15ULL + (h << 6) +
           (h >> 2);
      h ^= uint64_t(c.child_ids[2]) + 0x9E3779B97F4A7C15ULL + (h << 6) +
           (h >> 2);
      h ^= uint64_t(c.child_ids[3]) + 0x9E3779B97F4A7C15ULL + (h << 6) +
           (h >> 2);
      return h;
    }
  };

  std::vector<cell_id_t> results;
  std::vector<cell_t> cells;
  std::unordered_map<cell_t, cell_id_t, cell_hash> cell_ids;
  cell_id_t root_id;

  cell_id_t _get_or_create_id(const cell_t&);
  cell_id_t _get_or_create_id(const leaf_t);
  cell_id_t _get_or_create_id(const cell_id_t childs[4]);

  cell_id_t _get_empty_cell(const unsigned int n);

  cell_id_t _cell_from_grid_aux(const leaf_t* grid, size_t size);
  void _cell_to_grid_aux(leaf_t* grid, size_t size,
                         const cell_id_t cell_id) const;

  cell_id_t _get_cell_result(cell_id_t);

 public:
  MacroCell() {};
  MacroCell(std::vector<leaf_t>);
  operator std::vector<leaf_t>() const;
  ~MacroCell();

  void double_wrap();
  void double_empty();

  void evolve();

  std::size_t cells_count() { return cell_ids.size(); }
  unsigned int time_jumps_size() { return 1u << cells[root_id].n; }
};


#include "macro_cell.tpp"