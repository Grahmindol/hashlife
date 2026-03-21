
#include <stdio.h>
#include "hashlife.h"


void print_bytes(void* ptr, size_t size) {
  unsigned char* p = (unsigned char*)ptr;
  for (size_t i = 0; i < size; i++) {
    printf("%02X ", p[i]);
  }
  printf("\n");
}

int main() {
  hl_reset();

  // motif test 🔥
  bool grid[8][8] = {
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 1, 1, 1, 0},
      {0, 0, 0, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
  };

  

  // build cellule
  cell_id_t id = hl_cell_from_grid((bool*)grid, 8);
  printf("id : \n");
  hl_print_cell(id);

  cell_id_t doubled = hl_double_empty(id);
  printf("extended : \n");
  hl_print_cell(doubled);

  // wrap
  cell_id_t wrapped_id = hl_double_wrap(doubled);
  printf("wrapped : \n");
  hl_print_cell(wrapped_id);

  
  
  printf("become : \n");
  // wrap
  cell_id_t res_id = hl_get_result(wrapped_id);
  hl_print_cell(res_id);

  return 0;
}
