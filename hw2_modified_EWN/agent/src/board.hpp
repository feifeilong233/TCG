#include <array>
#include <fstream>
#include <map>
#include <vector>
#define N_ROW 6
#define N_COL 7
#define N_NEXT 18
using namespace std;

// Color
enum class Color { Red, Blue, Empty };

// Cube
class Cube {
   public:
    Color color;
    char num;
    Cube() : color(Color::Empty), num(0) {}
    Cube(Color color, char num) : color(color), num(num) {}
    void flipColor();
};

ostream& operator<<(ostream& os, Cube cube);

// Direction
enum class Direction { Horizontal, Vertical, Diagonal };

// Ply
class Ply {
   private:
    const static map<tuple<int, int, char, Direction>, Ply> plyInstances;
    Ply(int row, int col, char num, Direction dir)
        : row(row), col(col), num(num), dir(dir) {}

   public:
    int row;
    int col;
    char num;
    Direction dir;
    static map<tuple<int, int, char, Direction>, Ply> initPlyInstances();
    static const Ply& getPly(int r, int c, char num, Direction dir);
};

ostream& operator<<(ostream& os, Ply ply);

// Board
class Board {
   private:
    array<array<Cube, N_COL>, N_ROW> cubes;
    Color nextTurn = Color::Red;

   public:
    Cube& get(int r, int c) { return this->cubes[r][c]; }
    void setCube(int r, int c, Cube cube);
    void setNextTurn(Color nextTurn) { this->nextTurn = nextTurn; }
    void swap(int r1, int c1, int r2, int c2);
    void flip();
    Color getWinner() const;
    vector<Ply>* getAllValidPly() const;
    void playRand();
};

ostream& operator<<(ostream& os, Board board);