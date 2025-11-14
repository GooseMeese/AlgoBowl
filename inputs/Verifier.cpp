#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <cassert>
using namespace std;

constexpr int DX[4] = {0, 1, 0, -1};
constexpr int DY[4] = {1, 0, -1, 0};

using Board = vector<vector<int>>;

bool in_bounds(int x, int y, int H, int W) {
    return x >= 0 && x < H && y >= 0 && y < W;
}

void apply_gravity(Board& board) {
    int H = board.size(), W = board[0].size();
    for (int col = 0; col < W; ++col) {
        int write = 0;
        for (int row = 0; row < H; ++row) {
            if (board[row][col] > 0) {
                swap(board[write++][col], board[row][col]);
            }
        }
        while (write < H) board[write++][col] = 0;
    }
    // collapse columns
    int write_col = 0;
    for (int col = 0; col < W; ++col) {
        bool has_tile = false;
        for (int row = 0; row < H; ++row) {
            if (board[row][col] > 0) has_tile = true;
        }
        if (has_tile) {
            if (write_col != col) {
                for (int row = 0; row < H; ++row) {
                    swap(board[row][write_col], board[row][col]);
                }
            }
            ++write_col;
        }
    }
    for (int col = write_col; col < W; ++col)
        for (int row = 0; row < H; ++row)
            board[row][col] = 0;
}

int flood_fill(Board& board, int x, int y, int color, vector<pair<int,int>>& group) {
    int H = board.size(), W = board[0].size();
    if (!in_bounds(x, y, H, W) || board[x][y] != color) return 0;
    queue<pair<int, int>> q;
    q.push({x, y});
    board[x][y] = -1;
    group.emplace_back(x, y);
    int count = 1;
    while (!q.empty()) {
        auto [cx, cy] = q.front(); q.pop();
        for (int d = 0; d < 4; ++d) {
            int nx = cx + DX[d], ny = cy + DY[d];
            if (in_bounds(nx, ny, H, W) && board[nx][ny] == color) {
                board[nx][ny] = -1;
                q.push({nx, ny});
                group.emplace_back(nx, ny);
                ++count;
            }
        }
    }
    return count;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./output_verifier <input_file> <output_file>\n";
        return 1;
    }

    ifstream fin(argv[1]);
    ifstream fout(argv[2]);
    if (!fin || !fout) {
        cerr << "Error opening input or output file." << endl;
        return 1;
    }

    int H, W;
    fin >> H >> W;
    Board board(H, vector<int>(W));
    for (int i = H - 1; i >= 0; --i) {
        string line; fin >> line;
        for (int j = 0; j < W; ++j) {
            board[i][j] = line[j] - '0';
        }
    }

    int total_score, M;
    fout >> total_score >> M;
    int computed_score = 0;

    for (int move_num = 0; move_num < M; ++move_num) {
        int color, count, row, col;
        fout >> color >> count >> row >> col;
        int x = row - 1, y = col - 1;

        if (!in_bounds(x, y, H, W) || board[x][y] != color) {
            cerr << "Invalid move at step " << move_num + 1 << endl;
            return 1;
        }

        vector<pair<int,int>> group;
        Board copy = board;
        int actual_count = flood_fill(copy, x, y, color, group);

        if (actual_count != count) {
            cerr << "Mismatch count at step " << move_num + 1 << ": expected " << count << ", found " << actual_count << endl;
            return 1;
        }

        for (auto& [gx, gy] : group) board[gx][gy] = 0;
        apply_gravity(board);
        computed_score += (count - 1) * (count - 1);
    }

    if (computed_score != total_score) {
        cerr << "Score mismatch: expected " << total_score << ", computed " << computed_score << endl;
        return 1;
    }

    cout << "Output is valid. Score: " << computed_score << endl;
    return 0;
}