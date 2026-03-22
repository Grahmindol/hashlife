#include "hashlife.h"


#include <stdio.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"


typedef unsigned int cell_id_t;
typedef struct cell_struct_t {
  int n;
  union {
    bool values[4][4];
    cell_id_t child_ids[4];
  };
} cell_struct_t;

typedef struct hash_node_t {
  struct cell_struct_t key;
  cell_id_t value;
} hash_node_t;

typedef struct cell_t {
  struct cell_struct_t structure;
  cell_id_t result;
} cell_t;


static cell_t* g_cells_arena  = NULL;
static hash_node_t* g_cells_map  = NULL;


static cell_id_t _hl_get_or_create_cell_from_structure(const cell_struct_t structure) {
  //print_bytes(&structure, sizeof(cell_struct_t));
  long i = hmgeti(g_cells_map , structure);
  if (i < 0) {
    

    cell_t cell = (cell_t){.structure = structure, .result = -1};
    i = arrlen(g_cells_arena );

    hmput(g_cells_map , structure, i);
    arrpush(g_cells_arena , cell);

    printf("NEW CELL %d\n", i);
    //print_cell(i);
    //printf("END CELL\n");
  }
  return (cell_id_t)i;
}

static cell_id_t _hl_get_or_create_cell_from_vals(bool vals[4][4]) {
  cell_struct_t structure = {0};
  structure.n = 2;
  memcpy(structure.values, vals, sizeof(structure.values));
  return _hl_get_or_create_cell_from_structure(structure);
}

static cell_id_t _hl_get_or_create_cell_from_childs(cell_id_t childs[4]) {
  cell_struct_t structure = {0};
  structure.n = g_cells_arena [childs[0]].structure.n + 1;
  memcpy(structure.child_ids, childs, sizeof(structure.child_ids));
  return _hl_get_or_create_cell_from_structure(structure);
}

static cell_id_t _hl_cell_from_grid_aux(bool* grid, size_t grid_size,
                            unsigned int i, unsigned int j, unsigned int size) {
  if ((size & (size - 1)) != 0) {
    printf("ERROR: size doit etre une puissnace de 2 !!! \n");
    exit(1);
  }

  if (size == 4) {
    bool vals[4][4];
    for (int x = 0; x < 4; x++)
      for (int y = 0; y < 4; y++)
        vals[x][y] = grid[(i + x) * grid_size + (j + y)];
    return _hl_get_or_create_cell_from_vals(vals);
  }
  unsigned int half = size / 2;
  cell_id_t childs[4] = {
      _hl_cell_from_grid_aux(grid, grid_size, i, j, half),
      _hl_cell_from_grid_aux(grid, grid_size, i, j + half, half),
      _hl_cell_from_grid_aux(grid, grid_size, i + half, j, half),
      _hl_cell_from_grid_aux(grid, grid_size, i + half, j + half, half),
  };
  return _hl_get_or_create_cell_from_childs(childs);
}

static void _hl_cell_to_grid_aux(cell_id_t cell_id, bool* grid, size_t grid_size,
                  unsigned int i, unsigned int j) {
  cell_t cell = g_cells_arena [cell_id];
  if (cell.structure.n == 2) {
    for (int x = 0; x < 4; x++)
      for (int y = 0; y < 4; y++){
        grid[(i + x) * grid_size + (j + y)] = cell.structure.values[x][y];
      }
        
    return;
  }
  unsigned int half = 0b1 << (cell.structure.n - 1);
  _hl_cell_to_grid_aux(cell.structure.child_ids[0], grid, grid_size, i, j);
  _hl_cell_to_grid_aux(cell.structure.child_ids[1], grid, grid_size, i, j + half);
  _hl_cell_to_grid_aux(cell.structure.child_ids[2], grid, grid_size, i + half, j);
  _hl_cell_to_grid_aux(cell.structure.child_ids[3], grid, grid_size, i + half, j + half);
}


cell_id_t get_empty_cell(int n) {
  // cas de base
  if (n == 2) {
    bool vals[4][4] = {0};
    return _hl_get_or_create_cell_from_vals(vals);
  }

  // récursif
  cell_id_t e = get_empty_cell(n - 1);

  cell_id_t childs[4] = {
    e, e,
    e, e
  };

  return _hl_get_or_create_cell_from_childs(childs);
}


//------------------------------
// public
//------------------------------



void hl_reset() {
  hmfree(g_cells_map);
  arrfree(g_cells_arena);
}

cell_id_t hl_cell_from_grid(bool* grid, size_t grid_size){
  return _hl_cell_from_grid_aux(grid, grid_size, 0,0, grid_size);
}

void hl_cell_to_grid(cell_id_t cell_id, bool* grid){
  cell_t cell = g_cells_arena [cell_id];
  size_t grid_size = 0b1 << cell.structure.n;
  return _hl_cell_to_grid_aux(cell_id, grid, grid_size, 0,0);
}

cell_id_t hl_double_wrap(cell_id_t cell_id) {
  cell_t cell = g_cells_arena [cell_id];
  cell_id_t shifted;
  if (cell.structure.n == 2) {
    bool (*v)[4] = cell.structure.values;
    bool values_shift[4][4] = {
        {v[2][2], v[2][3], v[2][0], v[2][1]},  // D C
        {v[3][2], v[3][3], v[3][0], v[3][1]},
        {v[0][2], v[0][3], v[0][0], v[0][1]},  // B A
        {v[1][2], v[1][3], v[1][0], v[1][1]}};
    shifted = _hl_get_or_create_cell_from_vals(values_shift);

  } else {
    cell_id_t* c = cell.structure.child_ids;
    cell_id_t child_shifted[4] = {c[3], c[2], c[1], c[0]};
    shifted = _hl_get_or_create_cell_from_childs(child_shifted);
  }

  cell_id_t childs[4] = {
      shifted, shifted,
      shifted, shifted,
  };
  return _hl_get_or_create_cell_from_childs(childs);
}

cell_id_t hl_double_empty(cell_id_t cell_id){
  cell_t cell = g_cells_arena [cell_id];

  cell_id_t childs[4];

  if (cell.structure.n == 2) {
    bool (*v)[4] = cell.structure.values;

    // NW (vide sauf coin bas droite)
    bool NW[4][4] = {
        {0,0,0,0},
        {0,0,0,0},
        {0,0,v[0][0], v[0][1]},
        {0,0,v[1][0], v[1][1]}
    };

    // NE
    bool NE[4][4] = {
        {0,0,0,0},
        {0,0,0,0},
        {v[0][2], v[0][3],0,0},
        {v[1][2], v[1][3],0,0}
    };

    // SW
    bool SW[4][4] = {
        {0,0,v[2][0], v[2][1]},
        {0,0,v[3][0], v[3][1]},
        {0,0,0,0},
        {0,0,0,0}
    };

    // SE
    bool SE[4][4] = {
        {v[2][2], v[2][3],0,0},
        {v[3][2], v[3][3],0,0},
        {0,0,0,0},
        {0,0,0,0}
    };

    childs[0] = _hl_get_or_create_cell_from_vals(NW);
    childs[1] = _hl_get_or_create_cell_from_vals(NE);
    childs[2] = _hl_get_or_create_cell_from_vals(SW);
    childs[3] = _hl_get_or_create_cell_from_vals(SE);

  } else {
    cell_id_t* c = cell.structure.child_ids;

    // cellules vides du bon niveau
    cell_id_t empty = get_empty_cell(cell.structure.n - 1);

    childs[0] = _hl_get_or_create_cell_from_childs((cell_id_t[]){
        empty, empty,
        empty, c[0]
    });

    childs[1] = _hl_get_or_create_cell_from_childs((cell_id_t[]){
        empty, empty,
        c[1], empty
    });

    childs[2] = _hl_get_or_create_cell_from_childs((cell_id_t[]){
        empty, c[2],
        empty, empty
    });

    childs[3] = _hl_get_or_create_cell_from_childs((cell_id_t[]){
        c[3], empty,
        empty, empty
    });
  }

  return _hl_get_or_create_cell_from_childs(childs);
}


//calcul du resultat au laps de temps 2^n-2
cell_id_t hl_get_result(cell_id_t cell_id){
  cell_t cell = g_cells_arena [cell_id];

  // déjà calculé
  if(cell.result!= -1u)
    return cell.result;

  if (cell.structure.n == 3){
    bool grid[8][8];
    bool temp[8][8];
    _hl_cell_to_grid_aux(cell_id, (bool*)grid, 8, 0,0);

    for (int x = 1; x < 7; x++)
    for (int y = 1; y < 7; y++)
    {
      int count = 
        grid[x-1][y-1] + grid[x-1][y] + grid[x-1][y+1] +
        grid[x][y-1] +                  + grid[x][y+1] + 
        grid[x+1][y-1] + grid[x+1][y] + grid[x+1][y+1];

      temp[x][y] = (count == 3) || (grid[x][y] && count == 2);
    }

    for (int x = 2; x < 6; x++)
    for (int y = 2; y < 6; y++)
    {
      int count = 
        temp[x-1][y-1] + temp[x-1][y] + temp[x-1][y+1] +
        temp[x][y-1] +                  + temp[x][y+1] + 
        temp[x+1][y-1] + temp[x+1][y] + temp[x+1][y+1];

      grid[x][y] = (count == 3) || (temp[x][y] && count == 2);
    }
    
    return cell.result = _hl_cell_from_grid_aux((bool*)grid, 8, 2,2, 4);
  }

  cell_t child[4];
  for(int i = 0; i < 4; i++)
    child[i] = g_cells_arena [cell.structure.child_ids[i]];

  cell_id_t top[4] = {
    child[0].structure.child_ids[1], child[1].structure.child_ids[0],
    child[0].structure.child_ids[3], child[1].structure.child_ids[2],
  },bottom[4] = {
    child[2].structure.child_ids[1], child[3].structure.child_ids[0],
    child[2].structure.child_ids[3], child[3].structure.child_ids[2],
  },left[4] = {
    child[0].structure.child_ids[2], child[0].structure.child_ids[3],
    child[2].structure.child_ids[0], child[2].structure.child_ids[1],
  },right[4] = {
    child[1].structure.child_ids[2], child[1].structure.child_ids[3],
    child[3].structure.child_ids[0], child[3].structure.child_ids[1],
  },kern[4]  = {
    child[0].structure.child_ids[3], child[1].structure.child_ids[2],
    child[2].structure.child_ids[1], child[3].structure.child_ids[0],
  };

  cell_id_t large_center[3][3] = {
    {cell.structure.child_ids[0], _hl_get_or_create_cell_from_childs((cell_id_t*)top) ,cell.structure.child_ids[1]},
    {_hl_get_or_create_cell_from_childs((cell_id_t*)left), _hl_get_or_create_cell_from_childs((cell_id_t*)kern) ,_hl_get_or_create_cell_from_childs((cell_id_t*)right)},
    {cell.structure.child_ids[2], _hl_get_or_create_cell_from_childs((cell_id_t*)bottom) ,cell.structure.child_ids[3]},
  };

  cell_id_t large_center_results[3][3];
  for (int i = 0; i < 3; i++) for( int j = 0; j < 3; j++) 
    large_center_results[i][j] = hl_get_result(large_center[i][j]);

  cell_id_t NW_childs[4] = {
    large_center_results[0][0], large_center_results[0][1],
    large_center_results[1][0], large_center_results[1][1],
  },NE_childs[4] = {
    large_center_results[0][1], large_center_results[0][2],
    large_center_results[1][1], large_center_results[1][2],
  },SW_childs[4] = {
    large_center_results[1][0], large_center_results[1][1],
    large_center_results[2][0], large_center_results[2][1],
  },SE_childs[4] = {
    large_center_results[1][1], large_center_results[1][2],
    large_center_results[2][1], large_center_results[2][2],
  };

  cell_id_t final_child[4] = {
    hl_get_result(_hl_get_or_create_cell_from_childs((cell_id_t*)NW_childs)), hl_get_result(_hl_get_or_create_cell_from_childs((cell_id_t*)NE_childs)),
    hl_get_result(_hl_get_or_create_cell_from_childs((cell_id_t*)SW_childs)), hl_get_result(_hl_get_or_create_cell_from_childs((cell_id_t*)SE_childs)),
  };
  
  return cell.result = _hl_get_or_create_cell_from_childs((cell_id_t*)final_child);
}

void hl_print_cell(cell_id_t cell_id){
  cell_t cell = g_cells_arena [cell_id];
  int size = 0b1 << cell.structure.n;
  bool *grid = malloc(size*size);

  hl_cell_to_grid(cell_id, grid);

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      printf("%X ", grid[i*size + j]);
    }
    printf("\n");
  }

  free(grid);
}

size_t hl_memory_usage(){
  return hmlenu(g_cells_map ) * sizeof(hash_node_t) +
   arrlenu(g_cells_arena ) * sizeof(cell_t);
}

int hl_get_order(cell_id_t id){
  return g_cells_arena[id].structure.n;
}