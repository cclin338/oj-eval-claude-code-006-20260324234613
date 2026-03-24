#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <queue>
#include <set>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Global variables for the client
std::vector<std::vector<char>> grid;  // Current visible grid
std::vector<std::vector<bool>> known;   // True if the grid state is known (visited or marked)
std::set<std::pair<int, int>> unknown;  // Unknown grids

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize global variables
  grid.clear();
  known.clear();
  unknown.clear();

  // Execute first move
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  // Re-initialize the grid
  grid.resize(rows, std::vector<char>(columns));
  known.resize(rows, std::vector<bool>(columns, false));
  unknown.clear();

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      std::cin >> grid[i][j];
      if (grid[i][j] != '?') {
        known[i][j] = true;
      } else {
        known[i][j] = false;
        unknown.insert({i, j});
      }
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // Strategy 1: Auto-explore obvious safe grids
  // If a visited grid has k marked mines and its number is k, all unknown neighbors are safe
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (grid[i][j] >= '0' && grid[i][j] <= '8') {
        int num = grid[i][j] - '0';
        int marked_count = 0;
        int unknown_count = 0;
        std::vector<std::pair<int, int>> unknown_neighbors;

        int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

        for (int k = 0; k < 8; ++k) {
          int ni = i + dx[k];
          int nj = j + dy[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (grid[ni][nj] == '@') {
              marked_count++;
            } else if (grid[ni][nj] == '?') {
              unknown_count++;
              unknown_neighbors.push_back({ni, nj});
            }
          }
        }

        // If marked count equals number, all unknown neighbors are safe
        if (marked_count == num && unknown_count > 0) {
          // Auto-explore this grid
          Execute(i, j, 2);
          return;
        }
      }
    }
  }

  // Strategy 2: Mark obvious mines
  // If a visited grid has k unknown neighbors and its number is k, all neighbors are mines
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (grid[i][j] >= '0' && grid[i][j] <= '8') {
        int num = grid[i][j] - '0';
        int marked_count = 0;
        int unknown_count = 0;
        std::pair<int, int> unknown_neighbor = {-1, -1};

        int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

        for (int k = 0; k < 8; ++k) {
          int ni = i + dx[k];
          int nj = j + dy[k];
          if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
            if (grid[ni][nj] == '@') {
              marked_count++;
            } else if (grid[ni][nj] == '?') {
              unknown_count++;
              unknown_neighbor = {ni, nj};
            }
          }
        }

        // If unknown count equals number - marked count, mark the unknown neighbor as mine
        if (unknown_count == num - marked_count && unknown_count > 0) {
          Execute(unknown_neighbor.first, unknown_neighbor.second, 1);
          return;
        }
      }
    }
  }

  // Strategy 3: Visit a safe unknown grid (heuristic)
  // Prefer grids with more visited neighbors (lower probability of being a mine)
  std::pair<int, int> best_grid = {-1, -1};
  int best_score = -1;

  for (const auto& p : unknown) {
    int i = p.first;
    int j = p.second;

    int visited_neighbors = 0;
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int k = 0; k < 8; ++k) {
      int ni = i + dx[k];
      int nj = j + dy[k];
      if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && grid[ni][nj] != '?') {
        visited_neighbors++;
      }
    }

    if (visited_neighbors > best_score) {
      best_score = visited_neighbors;
      best_grid = p;
    }
  }

  if (best_grid.first != -1) {
    Execute(best_grid.first, best_grid.second, 0);
    return;
  }

  // Fallback: Visit the first unknown grid
  if (!unknown.empty()) {
    Execute(unknown.begin()->first, unknown.begin()->second, 0);
    return;
  }
}

#endif