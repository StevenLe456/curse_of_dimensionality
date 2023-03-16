#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
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
  this->configs[2] = std::vector<int>{100, 100};
  this->configs[3] = std::vector<int>{50, 50, 5};
  this->configs[4] = std::vector<int>{30, 30, 5, 5};
  this->configs[5] = std::vector<int>{15, 15, 5, 3, 3};
  this->configs[6] = std::vector<int>{8, 8, 5, 3, 3, 3};
  this->configs[7] = std::vector<int>{5, 5, 5, 3, 3, 3, 3};
  this->configs[8] = std::vector<int>{5, 5, 5, 3, 3, 3, 3, 3};
  this->num_dims = 2;
  this->bits_dir["1O"] = 1;
  this->bits_dir["2I"] = 2;
  this->bits_dir["1I"] = 4;
  this->bits_dir["2O"] = 8;
  this->bits_dir["3O"] = 16;
  this->bits_dir["4I"] = 32;
  this->bits_dir["3I"] = 64;
  this->bits_dir["4O"] = 128;
  this->bits_dir["5O"] = 256;
  this->bits_dir["6I"] = 512;
  this->bits_dir["5I"] = 1024;
  this->bits_dir["6O"] = 2048;
  this->bits_dir["7O"] = 4096;
  this->bits_dir["8I"] = 8192;
  this->bits_dir["7I"] = 16348;
  this->bits_dir["8O"] = 32768;
  int idx = 0;
  std::string in_n_out_dos[2] = {"O", "I"};
  for (int i = 1; i <= this->num_dims; i++) {
    for (std::string s : in_n_out_dos) {
      std::vector<int> dummy(this->num_dims, 0);
      dummy[idx] = s == "I" ? 1 : -1;
      cell_walls[dummy] = std::to_string(i) + s;
    }
    idx++;
  }
  this->dims = this->configs[this->num_dims];
  this->tiles[0] = 15;
  this->tiles[1] = 7;
  this->tiles[2] = 11;
  this->tiles[3] = 3;
  this->tiles[4] = 13;
  this->tiles[5] = 5;
  this->tiles[6] = 9;
  this->tiles[7] = 1;
  this->tiles[8] = 14;
  this->tiles[9] = 6;
  this->tiles[10] = 10;
  this->tiles[11] = 2;
  this->tiles[12] = 12;
  this->tiles[13] = 4;
  this->tiles[14] = 8;
  this->tiles[15] = 0;
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
    s *= this->dims[i];
  }
  int filler = 0;
  std::string in_n_out[2] = {"I", "O"};
  for (int i = 1; i <= this->num_dims; i++) {
    for (std::string s : in_n_out) {
      filler |= this->bits_dir[std::to_string(i) + s];
    }
  }
  for (int i = 0; i < s; i++) {
    this->maze.push_back(filler);
  }
}

int MazeGenerator::find_index(std::vector<int> pos) {
  int index = 0;
  for (int i = 0; i < this->num_dims; i++) {
    index += std::pow(int(this->dims.at(i)), i) * pos.at(i);
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
      cell.push_back(num % this->dims[j]);
      num /= int(this->dims[j]);
    }
    unvisited.push_back(cell);
  }
  std::vector<int> current(this->num_dims, 0);
  unvisited.erase(std::find(unvisited.begin(), unvisited.end(), current));
  std::map<int, std::vector<int>> neighbors;
  while (unvisited.size() > 0) {
    neighbors = check_neighbors(current, unvisited);
    if (!neighbors.empty()) {
      std::vector<int> next;
      if (neighbors.size() == 1) {
        next = neighbors.begin()->second;
      }
      else {
        next = get_random_cell(neighbors);
      }
      stack.push_back(current);
      std::vector<int> dir = subtract_vects(next, current);
      int current_walls = this->maze[find_index(current)] - this->bits_dir[this->cell_walls[dir]];
      int a = this->maze[find_index(next)];
      int b = this->bits_dir[this->cell_walls[negate_vect(dir)]];
      int next_walls = a - b;
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
  for (int i = 0; i < this->dims[0]; i++) {
    std::vector<int> dummy;
    for (int j = 0; j < this->dims[1]; j++) {
      dummy.push_back(this->maze[idx++]);
    }
    grid.push_back(dummy);
  }
  return grid;
}

void MazeGenerator::draw_maze(std::vector<std::vector<int>> grid) {
  for (int i = 0; i < grid.size(); i++) {
    for (int j = 0; j < grid.at(0).size(); j++) {
      set_cellv(Vector2(i, j), this->tiles[grid[i][j] & 15]);
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

void MazeGenerator::debug_maze() {
  std::vector<std::vector<int>> grid = get_grid(this->curr_pos);
  for (int i = 0; i < grid.size(); i++) {
    for (int j = 0; j < grid.at(0).size(); j++) {
      std::cout << (grid[i][j] & 15) << " ";
    }
    std::cout << std::endl;
  }
}
