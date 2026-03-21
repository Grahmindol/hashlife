#ifndef HASHLIFE_H
#define HASHLIFE_H

#include <stdbool.h>
#include <stddef.h>

typedef unsigned int cell_id_t;

// lifecycle
void hl_reset();

// construction
cell_id_t hl_cell_from_grid(bool* grid, size_t size);
void hl_cell_to_grid(cell_id_t id, bool* grid);

// ops
cell_id_t hl_double_wrap(cell_id_t id);
cell_id_t hl_double_empty(cell_id_t id);
cell_id_t hl_get_result(cell_id_t id);

// utils
void hl_print_cell(cell_id_t id);
size_t hl_memory_usage();

#endif