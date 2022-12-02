#include "board.hpp"

#include <assert.h>

#include "random/rand_gen.hpp"

ostream& operator<<(ostream& os, Cube cube) {
    switch (cube.color) {
        case Color::Red:
            return (os << "R" << cube.num);
        case Color::Blue:
            return (os << "B" << cube.num);
        case Color::Empty:
            return (os << "_"
                       << "_");
    }
}

void Cube::flipColor() {
    if (this->color == Color::Red)
        this->color = Color::Blue;
    else if (this->color == Color::Blue)
        this->color = Color::Red;
}

map<tuple<int, int, char, Direction>, Ply> Ply::initPlyInstances() {
    map<tuple<int, int, char, Direction>, Ply> instances;
    Direction directions[] = {Direction::Horizontal, Direction::Vertical,
                              Direction::Diagonal};
    for (int r = 0; r < N_ROW; r++) {
        for (int c = 0; c < N_COL; c++) {
            for (int num = 0; num < 6; num++) {
                for (Direction& dir : directions) {
                    instances.insert(make_pair(make_tuple(r, c, '0' + num, dir),
                                               Ply(r, c, '0' + num, dir)));
                }
            }
        }
    }
    return instances;
}

const map<tuple<int, int, char, Direction>, Ply> Ply::plyInstances =
    Ply::initPlyInstances();

const Ply& Ply::getPly(int r, int c, char num, Direction dir) {
    return plyInstances.at(make_tuple(r, c, num, dir));
}

ostream& operator<<(ostream& os, Ply ply) {
    os << "(" << ply.row << ", " << ply.col << ") " << ply.num << " ";
    switch (ply.dir) {
        case Direction::Horizontal:
            return (os << "horizontal");
        case Direction::Vertical:
            return (os << "vertical");
        case Direction::Diagonal:
            return (os << "diagonal");
    }
}

void Board::swap(int r1, int c1, int r2, int c2) {
    Cube temp = cubes[r1][c1];
    this->setCube(r1, c1, this->cubes[r2][c2]);
    this->setCube(r2, c2, temp);
}

void Board::flipNextTurn() {
    this->nextTurn = (this->nextTurn == Color::Red ? Color::Blue : Color::Red);
}

void Board::flip() {
    // horizontal flip
    for (int r = 0; r < N_ROW; r++) {
        for (int c = 0; c < N_COL / 2; c++) {
            this->swap(r, c, r, N_COL - c - 1);
        }
    }
    // vertical flip
    for (int c = 0; c < N_COL; c++) {
        for (int r = 0; r < N_ROW / 2; r++) {
            this->swap(r, c, N_ROW - 1 - r, c);
        }
    }
    // exchange colors
    for (int r = 0; r < N_ROW; r++) {
        for (int c = 0; c < N_COL; c++) {
            this->cubes[r][c].flipColor();
        }
    }
}

bool Board::isCompleted() const {
    // both colors have reached the corner
    const Cube upperLeft = this->cubes.front().front();
    const Cube lowerRight = this->cubes.back().back();
    if (upperLeft.color == Color::Blue && lowerRight.color == Color::Red) return true;

    // all red/blue cubes are captured
    int redCnt = 0, blueCnt = 0;
    for (const auto& row : this->cubes) {
        for (const auto& cube : row) {
            if (cube.color == Color::Red)
                redCnt += 1;
            else if (cube.color == Color::Blue)
                blueCnt += 1;
        }
    }
    return (redCnt == 0 || blueCnt == 0);
}

// should only apply to boards that are completed
Color Board::getWinner() const {
    // both colors have reached the corner
    const Cube upperLeft = this->cubes.front().front();
    const Cube lowerRight = this->cubes.back().back();
    if (upperLeft.color == Color::Blue && lowerRight.color == Color::Red) {
        if (upperLeft.num < lowerRight.num)
            return Color::Blue;
        else if (upperLeft.num > lowerRight.num)
            return Color::Red;
        return Color::Empty;
    }

    // all red/blue cubes are captured
    // the previous player must be the winner
    return (this->nextTurn == Color::Red ? Color::Blue : Color::Red);
}

vector<Ply>* Board::getAllValidPly() {
    this->validPlys.clear();
    for (int r = 0; r < N_ROW; r++) {
        for (int c = 0; c < N_COL; c++) {
            Cube cube = this->cubes[r][c];
            if (cube.color != this->nextTurn) continue;
            // horizontal: (r, c) -> Red (r, c + 1), Blue (r, c - 1)
            if (((this->nextTurn == Color::Red) && (c + 1 < N_COL) &&
                 (this->cubes[r][c + 1].color != this->nextTurn)) ||
                ((this->nextTurn == Color::Blue) && (c - 1 >= 0) &&
                 (this->cubes[r][c - 1].color != this->nextTurn))) {
                this->validPlys.push_back(
                    Ply::getPly(r, c, cube.num, Direction::Horizontal));
            }
            // vertical: (r, c) -> Red (r + 1, c), Blue (r - 1, c)
            if (((this->nextTurn == Color::Red) && (r + 1 < N_ROW) &&
                 (this->cubes[r + 1][c].color != this->nextTurn)) ||
                ((this->nextTurn == Color::Blue) && (r - 1 >= 0) &&
                 this->cubes[r - 1][c].color != this->nextTurn)) {
                this->validPlys.push_back(
                    Ply::getPly(r, c, cube.num, Direction::Vertical));
            }
            // diagonal: (r, c) -> Red (r + 1, c + 1), Blue (r - 1, c - 1)
            if (((this->nextTurn == Color::Red) && (r + 1 < N_ROW && c + 1 < N_COL) &&
                 (this->cubes[r + 1][c + 1].color != this->nextTurn)) ||
                ((this->nextTurn == Color::Blue) && (r - 1 >= 0) && (c - 1 >= 0) &&
                 (this->cubes[r - 1][c - 1].color != this->nextTurn))) {
                this->validPlys.push_back(
                    Ply::getPly(r, c, cube.num, Direction::Diagonal));
            }
        }
    }
    return &(this->validPlys);
}

void Board::applyPly(const Ply ply) {
    // move the cube
    Cube cubeToMove = this->cubes[ply.row][ply.col];
    assert(cubeToMove.color == this->nextTurn);
    assert(cubeToMove.num == ply.num);
    int destRow = (this->nextTurn == Color::Red ? ply.row + 1 : ply.row - 1);
    int destCol = (this->nextTurn == Color::Red ? ply.col + 1 : ply.col - 1);
    if (ply.dir == Direction::Horizontal) {
        this->cubes[ply.row][destCol] = cubeToMove;
    } else if (ply.dir == Direction::Vertical) {
        this->cubes[destRow][ply.col] = cubeToMove;
    } else if (ply.dir == Direction::Diagonal) {
        this->cubes[destRow][destCol] = cubeToMove;
    }
    this->cubes[ply.row][ply.col] = Cube();

    // update this->nextTurn
    this->flipNextTurn();
}

Color Board::playRandTillEnd() {
    while (!this->isCompleted()) {
        vector<Ply>* validPlys = this->getAllValidPly();
        Ply& randPly = validPlys->at(PcgRandGen::getRandNum(validPlys->size()));
        this->applyPly(randPly);
    }
    return this->getWinner();
}

ostream& operator<<(ostream& os, Board board) {
    // board
    for (int r = 0; r < N_ROW; r++) {
        for (int c = 0; c < N_COL; c++) {
            os << setw(4) << board.get(r, c);
        }
        os << endl;
    }
    return os;
}