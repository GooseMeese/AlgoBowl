#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

int R, C;
vector<vector<int>> grid;   // bottom-up: grid[row][col]

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: ./a.out <input_file>\n";
        return 1;
    }

    ifstream fin(argv[1]);
    if (!fin.is_open()) {
        cerr << "Error: could not open input file.\n";
        return 1;
    }

    fin >> R >> C;

    vector<string> raw(R);
    for (int i = 0; i < R; i++) {
        fin >> raw[i];
    }

    // Convert input (top-down) to bottom-up grid
    grid.assign(R, vector<int>(C));
    for (int i = 0; i < R; i++) {
        for (int j = 0; j < C; j++) {
            grid[R - 1 - i][j] = raw[i][j] - '0';
        }
    }



    return 0;
}
