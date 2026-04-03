#pragma once

#include "macro_cell.h"

#include <iostream>

using cell_id_t = uint32_t;

template<typename Policy>
cell_id_t MacroCell<Policy>::_get_or_create_id(const cell_t& c) {
  auto it = cell_ids.find(c);

  if (it == cell_ids.end()) {
    cell_id_t id = cells.size();
    if (id == 4294967295U) throw std::out_of_range("too many cells in cache !!");

    cells.push_back(c);
    results.push_back(-1);
    cell_ids.emplace(c, id);
    return id;
  }

  return it->second;
}

template<typename Policy>
cell_id_t MacroCell<Policy>::_get_or_create_id(const MacroCell<Policy>::leaf_t value) {
  cell_t s{};
  s.n = 0;
  s.value = value;
  return _get_or_create_id(s);
}

template<typename Policy>
cell_id_t MacroCell<Policy>::_get_or_create_id(const cell_id_t childs[4]) {
  cell_t s{};
  s.n = cells[childs[0]].n + 1;
  std::memcpy(s.child_ids, childs, sizeof(s.child_ids));
  return _get_or_create_id(s);
}

template<typename Policy>
cell_id_t MacroCell<Policy>::_get_empty_cell(const unsigned int n) {
  if (n == 0) {
    MacroCell<Policy>::leaf_t vals = 0;
    return _get_or_create_id(vals);
  }
  cell_id_t e = _get_empty_cell(n - 1);
  cell_id_t childs[4] = {e, e, e, e};
  return _get_or_create_id(childs);
}

template<typename Policy>
cell_id_t MacroCell<Policy>::_cell_from_grid_aux(const MacroCell<Policy>::leaf_t* grid, size_t size) {
  if(size == 0) throw std::invalid_argument("Size must be non zero..");
  
  if (size == 1) {
    MacroCell<Policy>::leaf_t val = *grid;
    return _get_or_create_id(val);
  }
  unsigned int quarter = size / 4;
  cell_id_t childs[4] = {
      _cell_from_grid_aux(grid, quarter),
      _cell_from_grid_aux(grid + quarter, quarter),
      _cell_from_grid_aux(grid + quarter + quarter, quarter),
      _cell_from_grid_aux(grid + quarter + quarter + quarter, quarter),
  };
  return _get_or_create_id(childs);
}

template<typename Policy>
void MacroCell<Policy>::_cell_to_grid_aux(MacroCell<Policy>::leaf_t* grid, size_t size, const cell_id_t cell_id) const {
  const cell_t& cell = cells[cell_id];

  if(size <= 1 && cell.n != 0) throw std::logic_error("grid size do not match cell size");
  
  if (cell.n == 0) {
    *grid = cell.value;
    return;
  }

  unsigned int quarter = size / 4;

  _cell_to_grid_aux(grid, quarter, cell.child_ids[0]);
  _cell_to_grid_aux(grid + quarter, quarter, cell.child_ids[1]);
  _cell_to_grid_aux(grid + quarter + quarter, quarter, cell.child_ids[2]);
  _cell_to_grid_aux(grid + quarter + quarter + quarter, quarter, cell.child_ids[3]);
}

// calcul du resultat au laps de temps 2^n-2
template<typename Policy>
cell_id_t MacroCell<Policy>::_get_cell_result(const cell_id_t cell_id) {
  if (cells[cell_id].n == 0) throw std::invalid_argument("cell order must be not null !");
  if (results[cell_id] != -1u) return results[cell_id];

  cell_t const* child[4];
  for (int i = 0; i < 4; i++) child[i] = &cells[cells[cell_id].child_ids[i]];

  if (cells[cell_id].n == 1) {
    leaf_t v = Policy::evolve(child[0]->value, child[1]->value,
                                         child[2]->value, child[3]->value);

    return results[cell_id] = _get_or_create_id(v);
  }

  cell_id_t top[4] =
      {
          child[0]->child_ids[1],
          child[1]->child_ids[0],
          child[0]->child_ids[3],
          child[1]->child_ids[2],
      },
            bottom[4] =
                {
                    child[2]->child_ids[1],
                    child[3]->child_ids[0],
                    child[2]->child_ids[3],
                    child[3]->child_ids[2],
                },
            left[4] =
                {
                    child[0]->child_ids[2],
                    child[0]->child_ids[3],
                    child[2]->child_ids[0],
                    child[2]->child_ids[1],
                },
            right[4] =
                {
                    child[1]->child_ids[2],
                    child[1]->child_ids[3],
                    child[3]->child_ids[0],
                    child[3]->child_ids[1],
                },
            kern[4] = {
                child[0]->child_ids[3],
                child[1]->child_ids[2],
                child[2]->child_ids[1],
                child[3]->child_ids[0],
            };

  cell_id_t large_center[3][3] = {
      {cells[cell_id].child_ids[0], _get_or_create_id((cell_id_t*)top),
       cells[cell_id].child_ids[1]},
      {_get_or_create_id((cell_id_t*)left), _get_or_create_id((cell_id_t*)kern),
       _get_or_create_id((cell_id_t*)right)},
      {cells[cell_id].child_ids[2], _get_or_create_id((cell_id_t*)bottom),
       cells[cell_id].child_ids[3]},
  };

  cell_id_t large_center_results[3][3];
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      large_center_results[i][j] = _get_cell_result(large_center[i][j]);

  cell_id_t NW_childs[4] =
      {
          large_center_results[0][0],
          large_center_results[0][1],
          large_center_results[1][0],
          large_center_results[1][1],
      },
            NE_childs[4] =
                {
                    large_center_results[0][1],
                    large_center_results[0][2],
                    large_center_results[1][1],
                    large_center_results[1][2],
                },
            SW_childs[4] =
                {
                    large_center_results[1][0],
                    large_center_results[1][1],
                    large_center_results[2][0],
                    large_center_results[2][1],
                },
            SE_childs[4] = {
                large_center_results[1][1],
                large_center_results[1][2],
                large_center_results[2][1],
                large_center_results[2][2],
            };

  const cell_id_t final_child[4] = {
      _get_cell_result(_get_or_create_id((cell_id_t*)NW_childs)),
      _get_cell_result(_get_or_create_id((cell_id_t*)NE_childs)),
      _get_cell_result(_get_or_create_id((cell_id_t*)SW_childs)),
      _get_cell_result(_get_or_create_id((cell_id_t*)SE_childs)),
  };

  results[cell_id] = _get_or_create_id(final_child);

  return results[cell_id];
}

template<typename Policy>
MacroCell<Policy>::MacroCell(std::vector<MacroCell<Policy>::leaf_t> data) {
  size_t N = data.size();

  if ((N & (N - 1)) != 0)
      throw std::invalid_argument("input size must be power of 2");
  
  int k = __builtin_ctz(N);  // nombre de zéros à droite
  if (k & 0b1) throw std::invalid_argument("input must be square");

  root_id = _cell_from_grid_aux(data.data(), data.size());
}

template<typename Policy>
MacroCell<Policy>::operator std::vector<MacroCell<Policy>::leaf_t>() const {
  size_t blocks = 1u << cells[root_id].n;
  std::vector<leaf_t> grid(blocks * blocks);

  _cell_to_grid_aux(grid.data(), grid.size(), root_id);
  return grid;
}

template<typename Policy>
MacroCell<Policy>::~MacroCell() {}

template<typename Policy>
void MacroCell<Policy>::double_wrap() {
  const cell_t& cell = cells[root_id];
  cell_id_t shifted;
  if (cell.n == 0) {
    MacroCell<Policy>::leaf_t values_shift = Policy::wrap(cell.value);
    shifted = _get_or_create_id(values_shift);

  } else {
    const cell_id_t* c = cell.child_ids;
    cell_id_t child_shifted[4] = {c[3], c[2], c[1], c[0]};
    shifted = _get_or_create_id(child_shifted);
  }

  cell_id_t childs[4] = {
      shifted,
      shifted,
      shifted,
      shifted,
  };
  root_id = _get_or_create_id(childs);
}

template<typename Policy>
void MacroCell<Policy>::double_empty() {
  const cell_t& cell = cells[root_id];

  cell_id_t childs[4];

  if (cell.n == 0) {
    throw std::logic_error("TO DO");
  } else {
    const cell_id_t* c = cell.child_ids;

    // cellules vides du bon niveau
    const cell_id_t empty = _get_empty_cell(cell.n - 1);
    const cell_id_t NW[4] = {empty, empty, empty, c[0]};
    const cell_id_t NE[4] = {empty, empty, c[1], empty};
    const cell_id_t SW[4] = {empty, c[2], empty, empty};
    const cell_id_t SE[4] = {c[3], empty, empty, empty};

    childs[0] = _get_or_create_id(NW);
    childs[1] = _get_or_create_id(NE);
    childs[2] = _get_or_create_id(SW);
    childs[3] = _get_or_create_id(SE);
  }

  root_id = _get_or_create_id(childs);

  
}

template<typename Policy>
void MacroCell<Policy>::evolve() { root_id = _get_cell_result(root_id); }