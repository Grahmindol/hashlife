#pragma once

#include <cstddef>
#include <cstdint>

namespace hashlife {

using cell_id_t = uint32_t;

// lifecycle
void reset();

// construction
cell_id_t cell_from_grid(const uint8_t* grid, std::size_t size);
void cell_to_grid(const cell_id_t id, uint8_t* grid);

// ops
cell_id_t double_wrap(const cell_id_t id);
cell_id_t double_empty(const cell_id_t id);
cell_id_t get_result(const cell_id_t id);

// utils
void print_cell(const cell_id_t id);
std::size_t memory_usage();
int get_order(const cell_id_t id);

}  // namespace hashlife