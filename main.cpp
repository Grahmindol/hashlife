
#include <iostream>

#include <GL/glut.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "hash_life.h"

int g_size;
std::vector<uint8_t> g_grid;
HashLife g_root_cell;


double now() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

uint8_t* load_rle(const char* filename, int* out_w, int* out_h) {
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

  uint8_t* grid = new uint8_t[w * h];

  int x = 0, y = 0;
  int count = 0;

  while (fgets(line, sizeof(line), f)) {
    for (char* c = line; *c; c++) {
      if (isdigit(*c)) {
        count = count * 10 + (*c - '0');
      } else if (*c == 'b' || *c == 'o') {
        if (count == 0) count = 1;

        for (int i = 0; i < count; i++) {
          if (*c == 'o') grid[y * w + x] = 1;
          x++;
        }
        count = 0;
      } else if (*c == '$') {
        if (count == 0) count = 1;
        y += count;
        x = 0;
        count = 0;
      } else if (*c == '!') {
        fclose(f);
        return grid;
      }
    }
  }

  fclose(f);
  return grid;
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);

  int pixel = 2;

  glBegin(GL_QUADS);

  for (int y = 0; y < g_size; y++) {
    for (int x = 0; x < g_size; x++) {
      if (g_grid[y * g_size + x])
        glColor3f(1.0f, 0.0f, 0.0f);  // rouge
      else
        glColor3f(0.0f, 0.0f, 0.0f);  // noir

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

void update(int) {
  //return;
  double t0 = now();
  g_root_cell.evolve();
  double t1 = now();

  double dt = t1 - t0;
  double IPS = g_root_cell.time_jumps_size() / dt;

  
  g_grid = static_cast<std::vector<uint8_t>>(g_root_cell);

  // mémoire
  double mem_kc = g_root_cell.cells_count() / 1024.;

  char title[256];
  snprintf(title, sizeof(title), "HashLife !! | %.2f IPS | %.1f KCell", IPS,
           mem_kc);

  glutSetWindowTitle(title);

  glutPostRedisplay();
  glutTimerFunc(50, update, 0);
}

void init_gl(int size) {
  int pixel = 2;
  int win = size * pixel;

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(win, win);
  glutCreateWindow("HashLife !!");

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, win, win, 0);  // origine en haut gauche

  glClearColor(0, 0, 0, 1);

  glutDisplayFunc(display);
  glutTimerFunc(50, update, 0);
}

int main(int argc, char** argv) {
  
  int w, h;
  uint8_t* rle = load_rle("pattern.rle", &w, &h);

  int size = 8;
  while (size < w || size < h) size <<= 1;

  g_size = size;
  g_grid = std::vector<uint8_t>(size * size);

  int offx = (size - w) / 2;
  int offy = (size - h) / 2;

  for (int y = 0; y < h; y++)
    for (int x = 0; x < w; x++)
      g_grid[(y + offy) * size + (x + offx)] = rle[y * w + x];

  delete[] (rle);

  g_root_cell = HashLife(g_grid);

  glutInit(&argc, argv);
  init_gl(size);

  glutMainLoop();
  return 0;
}