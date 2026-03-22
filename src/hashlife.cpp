#include "hashlife.h"

#include <cstring>
#include <iostream>
#include <unordered_map>
#include <vector>

using cell_id_t = uint32_t;

struct cell_struct_t {
  int n;

  union {
    uint8_t values[4][4];
    cell_id_t child_ids[4];
  };

  bool operator==(const cell_struct_t& other) const {
    if (n != other.n) return false;
    if (n == 2)
      return std::memcmp(values, other.values, sizeof(values)) == 0;
    else
      return std::memcmp(child_ids, other.child_ids, sizeof(child_ids)) == 0;
  }
};

struct CellHash {
  size_t operator()(const cell_struct_t& c) const {
    uint64_t h = uint64_t(c.n) * 0x9E3779B97F4A7C15ULL;
    if (c.n == 2) {
      const uint64_t* p = reinterpret_cast<const uint64_t*>(c.values);
      h ^= p[0] + 0x9E3779B97F4A7C15ULL + (h << 6) + (h >> 2);
      h ^= p[1] + 0x9E3779B97F4A7C15ULL + (h << 6) + (h >> 2);
    } else {
      for (int i = 0; i < 4; i++)
        h ^= uint64_t(c.child_ids[i]) + 0x9E3779B97F4A7C15ULL + (h << 6) +
             (h >> 2);
    }
    return h;
  }
};

struct cell_t {
  cell_struct_t structure;
  cell_id_t result = -1u;
};

static std::vector<cell_t> g_cells_arena;
static std::unordered_map<cell_struct_t, cell_id_t, CellHash> g_cells_map;

static cell_id_t _get_or_create_cell_from_structure(
    const cell_struct_t& structure) {
  auto it = g_cells_map.find(structure);

  if (it == g_cells_map.end()) {
    cell_id_t id = g_cells_arena.size();

    g_cells_arena.push_back({structure, (cell_id_t)-1});
    g_cells_map.emplace(structure, id);
    return id;
  }

  return it->second;
}

static cell_id_t _get_or_create_cell_from_vals(const uint8_t vals[4][4]) {
  cell_struct_t s{};
  s.n = 2;
  std::memcpy(s.values, vals, sizeof(s.values));
  return _get_or_create_cell_from_structure(s);
}

static cell_id_t _get_or_create_cell_from_childs(const cell_id_t childs[4]) {
  cell_struct_t s{};
  s.n = g_cells_arena[childs[0]].structure.n + 1;
  std::memcpy(s.child_ids, childs, sizeof(s.child_ids));
  return _get_or_create_cell_from_structure(s);
}

static cell_id_t _get_empty_cell(const unsigned int n) {
  if (n == 2) {
    uint8_t vals[4][4] = {0};
    return _get_or_create_cell_from_vals(vals);
  }
  cell_id_t e = _get_empty_cell(n - 1);
  cell_id_t childs[4] = {e, e, e, e};
  return _get_or_create_cell_from_childs(childs);
}

static cell_id_t _cell_from_grid_aux(const uint8_t* grid,
                                     const size_t grid_size,
                                     const unsigned int i, const unsigned int j,
                                     const unsigned int size) {
  if ((size & (size - 1)) != 0) {
    printf("ERROR: size doit etre une puissnace de 2 !!! \n");
    exit(1);
  }

  if (size == 4) {
    uint8_t vals[4][4];
    for (int x = 0; x < 4; x++)
      for (int y = 0; y < 4; y++)
        vals[x][y] = grid[(i + x) * grid_size + (j + y)];
    return _get_or_create_cell_from_vals(vals);
  }
  unsigned int half = size / 2;
  cell_id_t childs[4] = {
      _cell_from_grid_aux(grid, grid_size, i, j, half),
      _cell_from_grid_aux(grid, grid_size, i, j + half, half),
      _cell_from_grid_aux(grid, grid_size, i + half, j, half),
      _cell_from_grid_aux(grid, grid_size, i + half, j + half, half),
  };
  return _get_or_create_cell_from_childs(childs);
}

static void _cell_to_grid_aux(const cell_id_t cell_id, uint8_t* grid,
                              const size_t grid_size, const unsigned int i,
                              const unsigned int j) {
  cell_t cell = g_cells_arena[cell_id];
  if (cell.structure.n == 2) {
    for (int x = 0; x < 4; x++)
      for (int y = 0; y < 4; y++) {
        grid[(i + x) * grid_size + (j + y)] = cell.structure.values[x][y];
      }

    return;
  }
  unsigned int half = 0b1 << (cell.structure.n - 1);
  _cell_to_grid_aux(cell.structure.child_ids[0], grid, grid_size, i, j);
  _cell_to_grid_aux(cell.structure.child_ids[1], grid, grid_size, i, j + half);
  _cell_to_grid_aux(cell.structure.child_ids[2], grid, grid_size, i + half, j);
  _cell_to_grid_aux(cell.structure.child_ids[3], grid, grid_size, i + half,
                    j + half);
}

//------------------------------
// public
//------------------------------

void hashlife::reset() {
  g_cells_map.clear();
  g_cells_arena.clear();
}

hashlife::cell_id_t hashlife::cell_from_grid(const uint8_t* grid,
                                             size_t grid_size) {
  return _cell_from_grid_aux(grid, grid_size, 0, 0, grid_size);
}

void hashlife::cell_to_grid(const cell_id_t cell_id, uint8_t* grid) {
  const cell_t& cell = g_cells_arena[cell_id];
  size_t grid_size = 0b1 << cell.structure.n;
  return _cell_to_grid_aux(cell_id, grid, grid_size, 0, 0);
}

cell_id_t hashlife::double_wrap(const cell_id_t cell_id) {
  const cell_t& cell = g_cells_arena[cell_id];
  cell_id_t shifted;
  if (cell.structure.n == 2) {
    const uint8_t (*v)[4] = cell.structure.values;
    uint8_t values_shift[4][4] = {{v[2][2], v[2][3], v[2][0], v[2][1]},  // D C
                                  {v[3][2], v[3][3], v[3][0], v[3][1]},
                                  {v[0][2], v[0][3], v[0][0], v[0][1]},  // B A
                                  {v[1][2], v[1][3], v[1][0], v[1][1]}};
    shifted = _get_or_create_cell_from_vals(values_shift);

  } else {
    const cell_id_t* c = cell.structure.child_ids;
    cell_id_t child_shifted[4] = {c[3], c[2], c[1], c[0]};
    shifted = _get_or_create_cell_from_childs(child_shifted);
  }

  cell_id_t childs[4] = {
      shifted,
      shifted,
      shifted,
      shifted,
  };
  return _get_or_create_cell_from_childs(childs);
}

cell_id_t hashlife::double_empty(const cell_id_t cell_id) {
  const cell_t& cell = g_cells_arena[cell_id];

  cell_id_t childs[4];

  if (cell.structure.n == 2) {
    const uint8_t (*v)[4] = cell.structure.values;

    // NW (vide sauf coin bas droite)
    uint8_t NW[4][4] = {{0, 0, 0, 0},
                        {0, 0, 0, 0},
                        {0, 0, v[0][0], v[0][1]},
                        {0, 0, v[1][0], v[1][1]}};

    // NE
    uint8_t NE[4][4] = {{0, 0, 0, 0},
                        {0, 0, 0, 0},
                        {v[0][2], v[0][3], 0, 0},
                        {v[1][2], v[1][3], 0, 0}};

    // SW
    uint8_t SW[4][4] = {{0, 0, v[2][0], v[2][1]},
                        {0, 0, v[3][0], v[3][1]},
                        {0, 0, 0, 0},
                        {0, 0, 0, 0}};

    // SE
    uint8_t SE[4][4] = {{v[2][2], v[2][3], 0, 0},
                        {v[3][2], v[3][3], 0, 0},
                        {0, 0, 0, 0},
                        {0, 0, 0, 0}};

    childs[0] = _get_or_create_cell_from_vals(NW);
    childs[1] = _get_or_create_cell_from_vals(NE);
    childs[2] = _get_or_create_cell_from_vals(SW);
    childs[3] = _get_or_create_cell_from_vals(SE);

  } else {
    const cell_id_t* c = cell.structure.child_ids;

    // cellules vides du bon niveau
    cell_id_t empty = _get_empty_cell(cell.structure.n - 1);

    // NW (vide sauf coin bas droite)
    cell_id_t NW[4] = {empty, empty, empty, c[0]};

    // NE
    cell_id_t NE[4] = {empty, empty, c[1], empty};

    // SW
    cell_id_t SW[4] = {empty, c[2], empty, empty};

    // SE
    cell_id_t SE[4] = {c[3], empty, empty, empty};

    childs[0] = _get_or_create_cell_from_childs(NW);
    childs[1] = _get_or_create_cell_from_childs(NE);
    childs[2] = _get_or_create_cell_from_childs(SW);
    childs[3] = _get_or_create_cell_from_childs(SE);
  }

  return _get_or_create_cell_from_childs(childs);
}

// calcul du resultat au laps de temps 2^n-2
cell_id_t hashlife::get_result(const cell_id_t cell_id) {
  // cell_t& cell = g_cells_arena[cell_id];

  // déjà calculé
  if (g_cells_arena[cell_id].result != -1u)
    return g_cells_arena[cell_id].result;

  if (g_cells_arena[cell_id].structure.n == 3) {
    uint8_t grid[8][8];
    uint8_t temp[8][8];
    _cell_to_grid_aux(cell_id, (uint8_t*)grid, 8, 0, 0);

    for (int x = 1; x < 7; x++)
      for (int y = 1; y < 7; y++) {
        int count = grid[x - 1][y - 1] + grid[x - 1][y] + grid[x - 1][y + 1] +
                    grid[x][y - 1] + +grid[x][y + 1] + grid[x + 1][y - 1] +
                    grid[x + 1][y] + grid[x + 1][y + 1];

        temp[x][y] = (count == 3) || (grid[x][y] && count == 2);
      }

    for (int x = 2; x < 6; x++)
      for (int y = 2; y < 6; y++) {
        int count = temp[x - 1][y - 1] + temp[x - 1][y] + temp[x - 1][y + 1] +
                    temp[x][y - 1] + temp[x][y + 1] + temp[x + 1][y - 1] +
                    temp[x + 1][y] + temp[x + 1][y + 1];

        grid[x][y] = (count == 3) || (temp[x][y] && count == 2);
      }

    return g_cells_arena[cell_id].result =
               _cell_from_grid_aux((uint8_t*)grid, 8, 2, 2, 4);
  }

  cell_t const* child[4];
  for (int i = 0; i < 4; i++)
    child[i] = &g_cells_arena[g_cells_arena[cell_id].structure.child_ids[i]];

  cell_id_t top[4] =
      {
          child[0]->structure.child_ids[1],
          child[1]->structure.child_ids[0],
          child[0]->structure.child_ids[3],
          child[1]->structure.child_ids[2],
      },
            bottom[4] =
                {
                    child[2]->structure.child_ids[1],
                    child[3]->structure.child_ids[0],
                    child[2]->structure.child_ids[3],
                    child[3]->structure.child_ids[2],
                },
            left[4] =
                {
                    child[0]->structure.child_ids[2],
                    child[0]->structure.child_ids[3],
                    child[2]->structure.child_ids[0],
                    child[2]->structure.child_ids[1],
                },
            right[4] =
                {
                    child[1]->structure.child_ids[2],
                    child[1]->structure.child_ids[3],
                    child[3]->structure.child_ids[0],
                    child[3]->structure.child_ids[1],
                },
            kern[4] = {
                child[0]->structure.child_ids[3],
                child[1]->structure.child_ids[2],
                child[2]->structure.child_ids[1],
                child[3]->structure.child_ids[0],
            };

  cell_id_t large_center[3][3] = {
      {g_cells_arena[cell_id].structure.child_ids[0],
       _get_or_create_cell_from_childs((cell_id_t*)top),
       g_cells_arena[cell_id].structure.child_ids[1]},
      {_get_or_create_cell_from_childs((cell_id_t*)left),
       _get_or_create_cell_from_childs((cell_id_t*)kern),
       _get_or_create_cell_from_childs((cell_id_t*)right)},
      {g_cells_arena[cell_id].structure.child_ids[2],
       _get_or_create_cell_from_childs((cell_id_t*)bottom),
       g_cells_arena[cell_id].structure.child_ids[3]},
  };

  cell_id_t large_center_results[3][3];
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      large_center_results[i][j] = hashlife::get_result(large_center[i][j]);

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
      hashlife::get_result(
          _get_or_create_cell_from_childs((cell_id_t*)NW_childs)),
      hashlife::get_result(
          _get_or_create_cell_from_childs((cell_id_t*)NE_childs)),
      hashlife::get_result(
          _get_or_create_cell_from_childs((cell_id_t*)SW_childs)),
      hashlife::get_result(
          _get_or_create_cell_from_childs((cell_id_t*)SE_childs)),
  };

  g_cells_arena[cell_id].result = _get_or_create_cell_from_childs(final_child);

  return g_cells_arena[cell_id].result;
}

void hashlife::print_cell(const cell_id_t cell_id) {
  const cell_t& cell = g_cells_arena[cell_id];
  int size = 1 << cell.structure.n;

  std::vector<uint8_t> grid(size * size);

  hashlife::cell_to_grid(cell_id, grid.data());

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      std::cout << grid[i * size + j] << " ";
    }
    std::cout << "\n";
  }
}

std::size_t hashlife::memory_usage() {
  return g_cells_map.size() * (sizeof(cell_struct_t) + sizeof(cell_id_t)) +
         g_cells_arena.size() * sizeof(cell_t);
}

int hashlife::get_order(const cell_id_t id) {
  return g_cells_arena[id].structure.n;
}