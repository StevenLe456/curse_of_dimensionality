#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <stdlib.h>
#include <string>

#include <Array.hpp>
#include <Dictionary.hpp>
#include <File.hpp>
#include <JSON.hpp>
#include <JSONParseResult.hpp>
#include <OS.hpp>
#include <ProjectSettings.hpp>
#include <Vector2.hpp>

#include "maze_generator.h"

using namespace godot;

void MazeGenerator::_register_methods() {
  register_method("_process", &MazeGenerator::_process);
}

MazeGenerator::MazeGenerator() {
  //Nothing to see here!
}

MazeGenerator::~MazeGenerator() {
  //Nothing to see here!
}

void MazeGenerator::_init() {
  this->configs[2] = std::vector<int>{1000, 1000};
  this->configs[3] = std::vector<int>{500, 500, 10};
  this->configs[4] = std::vector<int>{300, 300, 10, 10};
  this->configs[5] = std::vector<int>{150, 150, 8, 8, 8};
  this->configs[6] = std::vector<int>{55, 55, 8, 8, 8, 8};
  this->configs[7] = std::vector<int>{20, 20, 8, 8, 8, 8, 8};
  this->configs[8] = std::vector<int>{8, 8, 8, 8, 8, 8, 8, 8};
  this->num_dims = 2;
  std::string in_n_out[2] = {"I", "O"};
  int counter = 0;
  for (int i = 1; i <= 8; i++) {
    for (std::string s : in_n_out) {
      bits_dir[std::to_string(i) + s] = std::pow(2, counter);
      counter++;
    }
  }
  int idx = 0;
  for (int i = 1; i <= 8; i++) {
    for (std::string s : in_n_out) {
      std::vector<int> dummy(10, 0);
      dummy[idx] = s == "I" ? 1 : -1;
      cell_walls[dummy] = std::to_string(i) + s;
    }
  }
  this->dims = this->configs[this->num_dims];
  this->init_maze();
  this->carve_maze();
  this->draw_maze(this->get_grid(this->curr_pos));
}

void MazeGenerator::_process(float delta) {
  //Nothing to see here!
}

void MazeGenerator::init_maze() {
  int s = 1;
  for (int i = 0; i < this->dims.size(); i++) {
    s *= int(this->dims[i]);
  }
  int filler = 0;
  std::string in_n_out[2] = {"I", "O"};
  for (int i = 1; i <= this->num_dims; i++) {
    for (std::string s : in_n_out) {
      filler |= this->bits_dir[std::to_string(i) + s];
    }
  }
  this->maze.resize(s);
  std::fill(this->maze.begin(), this->maze.end(), filler);
}

int MazeGenerator::find_index(std::vector<int> pos) {
  int index = 0;
  for (int i = 0; i < this->num_dims; i++) {
    index += std::pow(int(this->dims[i]), i) * pos[i];
  }
  return index;
}

std::map<int, std::vector<int>> MazeGenerator::check_neighbors(std::vector<int> cell, std::vector<std::vector<int>> unvisited) {
  std::map<int, std::vector<int>> neighbors;
  for (int i = 0; i < 2 * this->num_dims; i++) {
    std::vector<int> bin(num_dims, 0);
    if (i % 2 == 0) {
      bin[i / 2] = 1;
    }
    else {
      bin[i / 2] = -1;
    }
    if (std::find(unvisited.begin(), unvisited.end(), add_vects(cell, bin)) != unvisited.end()) {
      neighbors[i / 2] = add_vects(cell, bin);
    }
  }
  return neighbors;
}

std::vector<int> MazeGenerator::get_random_cell(std::map<int, std::vector<int>> cells) {
  std::vector<int> weights;
  for (std::map<int, std::vector<int>>::iterator it = cells.begin(); it != cells.end(); it++) {
    weights.push_back(this->dist.at(it->first));
  }
  std::discrete_distribution<std::size_t> d(weights.begin(), weights.end());
  std::vector<int> sampled = cells[d(this->gen)];
  return sampled;
}

void MazeGenerator::carve_maze() {
  std::vector<std::vector<int>> unvisited;
  std::vector<std::vector<int>> stack;
  int num = 1;
  for (int i = 0; i < this->num_dims; i++) {
    num *= int(this->dims[i]);
  }
  for (int i = 0; i < num; i++) {
    std::vector<int> cell;
    int num = i;
    for (int j = 0; j < this->num_dims; j++) {
      cell.push_back(num % int(this->dims[i]));
      num /= int(this->dims[i]);
    }
    unvisited.push_back(cell);
  }
  std::vector<int> current(this->num_dims, 0);
  unvisited.erase(std::find(unvisited.begin(), unvisited.end(), current));
  while (unvisited.size() > 0) {
    std::map<int, std::vector<int>> neighbors = check_neighbors(current, unvisited);
    if (!neighbors.empty()) {
      std::vector<int> next = get_random_cell(neighbors);
      stack.push_back(current);
      std::vector<int> dir = subtract_vects(next, current);
      int current_walls = this->maze[find_index(current)] - this->bits_dir[this->cell_walls[dir]];
      int next_walls = this->maze[find_index(next)] - this->bits_dir[this->cell_walls[negate_vect(dir)]];
      this->maze[find_index(current)] = current_walls;
      this->maze[find_index(next)] = next_walls;
      current = next;
      unvisited.erase(std::find(unvisited.begin(), unvisited.end(), current));
    }
    else if (stack.size() > 0) {
      current = stack[stack.size() - 1];
      stack.pop_back();
    }
  }
}

std::vector<std::vector<int>> MazeGenerator::get_grid(std::vector<int> pos) {
  int idx = this->find_index(pos);
  std::vector<std::vector<int>> grid;
  for (int i = 0; i < pos[0]; i++) {
    std::vector<int> dummy;
    for (int j = 0; j < pos[1]; j++) {
      dummy.push_back(this->maze[idx++]);
    }
    grid.push_back(dummy);
  }
  return grid;
}

void MazeGenerator::draw_maze(std::vector<std::vector<int>> grid) {
  int num = this->bits_dir["1I"] | this->bits_dir["1O"] | this->bits_dir["2I"] | this->bits_dir["2O"];
  for (int i = 0; i < int(this->dims[0]); i++) {
    for (int j = 0; j < int(this->dims[1]); j++) {
      set_cellv(Vector2(i, j), grid[i][j] & num);
    }
  }
}

std::vector<int> MazeGenerator::add_vects(std::vector<int> arr1, std::vector<int> arr2) {
  std::vector<int> arr;
  for (int i = 0; i < arr1.size(); i++) {
    arr.push_back(arr1[i] + arr2[i]);
  }
  return arr;
}

std::vector<int> MazeGenerator::subtract_vects(std::vector<int> arr1, std::vector<int> arr2) {
  std::vector<int> arr;
  for (int i = 0; i < arr1.size(); i++) {
    arr.push_back(arr1[i] - arr2[i]);
  }
  return arr;
}

std::vector<int> MazeGenerator::negate_vect(std::vector<int> arr) {
  std::vector<int> res;
  for (int i : arr) {
    res.push_back(-i);
  }
  return res;
}
