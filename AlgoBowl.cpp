#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <set>

using namespace std;

constexpr int MAXN = 100;
constexpr int COLORS = 8;
constexpr int DX[4] = {0, 1, 0, -1};
constexpr int DY[4] = {1, 0, -1, 0};
constexpr int BEAM_WIDTH = 200;
constexpr int SIMULATIONS = 30;

using Board = vector<vector<int>>;

struct Move {
    int color, count, row, col;
};

struct State {
    Board board;
    int score;
    vector<Move> moves;
    double eval = 0;

    bool operator<(const State& other) const {
        return eval > other.eval; // max-heap
    }
};

int score(int count) {
    return (count - 1) * (count - 1);
}

bool in_bounds(int x, int y, int H, int W) {
    return x >= 0 && x < H && y >= 0 && y < W;
}

int flood_fill(Board& b, int x, int y, int color, vector<pair<int,int>>& group) {
    int H = b.size(), W = b[0].size();
    if (!in_bounds(x, y, H, W) || b[x][y] != color) return 0;
    queue<pair<int,int>> q;
    q.push({x, y});
    b[x][y] = -1;
    group.push_back({x, y});
    int cnt = 1;
    while (!q.empty()) {
        auto [cx, cy] = q.front(); q.pop();
        for (int d = 0; d < 4; ++d) {
            int nx = cx + DX[d], ny = cy + DY[d];
            if (in_bounds(nx, ny, H, W) && b[nx][ny] == color) {
                b[nx][ny] = -1;
                q.push({nx, ny});
                group.push_back({nx, ny});
                cnt++;
            }
        }
    }
    return cnt;
}

void apply_gravity(Board& b) {
    int H = b.size(), W = b[0].size();
    for (int col = 0; col < W; ++col) {
        int bottom = 0;
        for (int row = 0; row < H; ++row) {
            if (b[row][col] >= 1) {
                swap(b[bottom++][col], b[row][col]);
            }
        }
        while (bottom < H) b[bottom++][col] = 0;
    }
    // collapse columns
    int write = 0;
    for (int col = 0; col < W; ++col) {
        bool has_tile = false;
        for (int row = 0; row < H; ++row) {
            if (b[row][col] > 0) has_tile = true;
        }
        if (has_tile) {
            if (col != write) {
                for (int row = 0; row < H; ++row)
                    swap(b[row][write], b[row][col]);
            }
            write++;
        }
    }
    for (int col = write; col < W; ++col)
        for (int row = 0; row < H; ++row)
            b[row][col] = 0;
}

vector<State> generate_moves(const State& s) {
    int H = s.board.size(), W = s.board[0].size();
    Board mark = s.board;
    vector<State> next_states;
    set<pair<int,int>> visited;
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            if (mark[i][j] >= 1) {
                vector<pair<int,int>> group;
                int color = mark[i][j];
                int count = flood_fill(mark, i, j, color, group);
                if (count >= 2) {
                    Board new_board = s.board;
                    for (auto [x, y] : group) new_board[x][y] = 0;
                    apply_gravity(new_board);
                    State ns = {new_board, s.score + score(count), s.moves};
                    ns.moves.push_back({color, count, i + 1, j + 1});
                    next_states.push_back(ns);
                }
            }
        }
    }
    return next_states;
}

int monte_carlo(const State& s, mt19937& rng) {
    State curr = s;
    while (true) {
        auto moves = generate_moves(curr);
        if (moves.empty()) break;
        uniform_int_distribution<int> dist(0, moves.size() - 1);
        curr = moves[dist(rng)];
    }
    return curr.score;
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    ifstream fin(argv[1]);
    if (!fin) {
        cerr << "Error opening input file." << endl;
        return 1;
    }

    int H, W;
    fin >> H >> W;
    Board b(H, vector<int>(W));
    for (int i = H - 1; i >= 0; --i) {
        string line; fin >> line;
        for (int j = 0; j < W; ++j)
            b[i][j] = line[j] - '0';
    }

    auto start_time = chrono::steady_clock::now();
    vector<State> beam = {{b, 0, {}}};
    State best = beam[0];
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

    while (!beam.empty()) {
        vector<State> candidates;
        for (auto& s : beam) {
            auto next = generate_moves(s);
            for (auto& ns : next) {
                int total = 0;
                for (int i = 0; i < SIMULATIONS; ++i)
                    total += monte_carlo(ns, rng);
                ns.eval = total / (double)SIMULATIONS;
                if (ns.score > best.score) best = ns;
                candidates.push_back(ns);
            }
        }
        sort(candidates.begin(), candidates.end());
        if (candidates.size() > BEAM_WIDTH) candidates.resize(BEAM_WIDTH);
        beam = move(candidates);
        if (chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count() > 180) break;
    }

    ofstream fout("output.txt");
    fout << best.score << '\n';
    fout << best.moves.size() << '\n';
    for (auto& m : best.moves) {
        fout << m.color << ' ' << m.count << ' ' << m.row << ' ' << m.col << '\n';
    }
    fout.close();

    return 0;
}
