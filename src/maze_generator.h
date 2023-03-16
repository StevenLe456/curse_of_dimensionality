#ifndef MAZE_GENERATOR_H
#define MAZE_GENERATOR_H

#include <Godot.hpp>
#include <TileMap.hpp>

#include <chrono>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace godot {
  class MazeGenerator : public TileMap {
    GODOT_CLASS(MazeGenerator, TileMap)
    private:
      //Class Variables
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine gen{seed};
      std::map<int, std::vector<int>> configs;
      int num_dims = 2;
      std::vector<int> dims;
      std::map<std::string, int> bits_dir;
      std::map<std::vector<int>, std::string> cell_walls;
      std::vector<float> dist{1000, 1000, 100, 10, 1, 0.1, 0.01, 0.001};
      std::vector<int> maze;
      std::vector<int> prev_pos{num_dims, 0};
      std::vector<int> curr_pos{num_dims, 0};
      std::map<int, int> tiles;
      //Class methods
      void init_maze();
      std::map<int, std::vector<int>> check_neighbors(std::vector<int> cell, std::vector<std::vector<int>> unvisited);
      int find_index(std::vector<int> pos);
      std::vector<std::vector<int>> get_grid(std::vector<int> pos);
      std::vector<int> get_random_cell(std::map<int, std::vector<int>> cells);
      void carve_maze();
      void draw_maze(std::vector<std::vector<int>> grid);
      std::vector<int> add_vects(std::vector<int> arr1, std::vector<int> arr2);
      std::vector<int> subtract_vects(std::vector<int> arr1, std::vector<int> arr2);
      std::vector<int> negate_vect(std::vector<int> arr);
      void debug_maze();
    public:
      static void _register_methods();
      MazeGenerator();
      ~MazeGenerator();
      void _init();
      void _process(float delta);
  };
}

#endif
