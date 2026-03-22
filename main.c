
#include <stdio.h>
#include <string.h>
#include "hashlife.h"
#include <GL/glut.h>
#include <ctype.h>
#include <time.h>


int g_size;
bool* g_grid;
cell_id_t g_id;

double g_total_iters = 0;
double g_last_time = 0;
double g_ips = 0;


double now() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

bool* load_rle(const char* filename, int* out_w, int* out_h) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    perror("open RLE");
    return NULL;
  }

  int w = 0, h = 0;

  // lire header
  char line[256];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#') continue;
    if (sscanf(line, "x = %d, y = %d", &w, &h) == 2) break;
  }

  *out_w = w;
  *out_h = h;

  bool* grid = calloc(w * h, sizeof(bool));

  int x = 0, y = 0;
  int count = 0;

  while (fgets(line, sizeof(line), f)) {
    for (char* c = line; *c; c++) {

      if (isdigit(*c)) {
        count = count * 10 + (*c - '0');
      }
      else if (*c == 'b' || *c == 'o') {
        if (count == 0) count = 1;

        for (int i = 0; i < count; i++) {
          if (*c == 'o') grid[y*w + x] = 1;
          x++;
        }
        count = 0;
      }
      else if (*c == '$') {
        if (count == 0) count = 1;
        y += count;
        x = 0;
        count = 0;
      }
      else if (*c == '!') {
        fclose(f);
        return grid;
      }
    }
  }

  fclose(f);
  return grid;
}

#include <unistd.h>

void print_grid(bool* grid, int size) {
  printf("\033[H"); // curseur en haut
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      printf(grid[i*size + j] ? "██" : "  ");
    }
    printf("\n");
  }
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);

  int pixel = 2;

  glBegin(GL_QUADS);

  for (int y = 0; y < g_size; y++) {
    for (int x = 0; x < g_size; x++) {

      if (g_grid[y*g_size + x])
        glColor3f(1.0f, 0.0f, 0.0f); // rouge
      else
        glColor3f(0.0f, 0.0f, 0.0f); // noir

      float px = x * pixel;
      float py = y * pixel;

      glVertex2f(px, py);
      glVertex2f(px + pixel, py);
      glVertex2f(px + pixel, py + pixel);
      glVertex2f(px, py + pixel);
    }
  }

  glEnd();
  glFlush();
}

void update(int v) {
  double t0 = now();

  

  g_id = hl_double_empty(g_id);

  double iters = 1 << (hl_get_order(g_id) - 2);
  g_last_time = now();

  g_id = hl_get_result(g_id);

  double t1 = now();
  double dt = t1 - g_last_time;
  g_total_iters += iters;

  g_ips = g_total_iters / dt;
    g_total_iters = 0;
    g_last_time = t1;

  memset(g_grid, 0, g_size*g_size);
  hl_cell_to_grid(g_id, g_grid);

  // mémoire
  double mem_kb = hl_memory_usage() / 1024.0;

  char title[256];
  snprintf(title, sizeof(title),
    "HashLife 🔥 | %.2f IPS | %.1f Ko",
    g_ips, mem_kb);

  glutSetWindowTitle(title);

  glutPostRedisplay();
  glutTimerFunc(50, update, 0);
}

void init_gl(int size) {
  int pixel = 2;
  int win = size * pixel;

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(win, win);
  glutCreateWindow("HashLife 🔥");

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, win, win, 0); // origine en haut gauche

  glClearColor(0,0,0,1);

  glutDisplayFunc(display);
  glutTimerFunc(50, update, 0);
}


void print_bytes(void* ptr, size_t size) {
  unsigned char* p = (unsigned char*)ptr;
  for (size_t i = 0; i < size; i++) {
    printf("%02X ", p[i]);
  }
  printf("\n");
}
int main(int argc, char** argv) {
  hl_reset();

  int w, h;
  bool* rle = load_rle("pattern.rle", &w, &h);

  int size = 8;
  while (size < w || size < h) size <<= 1;

  g_size = size;
  g_grid = calloc(size * size, sizeof(bool));

  int offx = (size - w)/2;
  int offy = (size - h)/2;

  for (int y = 0; y < h; y++)
    for (int x = 0; x < w; x++)
      g_grid[(y+offy)*size + (x+offx)] = rle[y*w + x];

  free(rle);

  g_id = hl_cell_from_grid(g_grid, size);

  glutInit(&argc, argv);
  init_gl(size);

  glutMainLoop();
  return 0;
}