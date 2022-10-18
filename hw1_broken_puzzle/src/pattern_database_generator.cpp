#include "pattern_database_generator.h"

#include <sstream>
#include <string>

void PatternDatabaseGenerator::setPatterns(vector<vector<int>> patterns) {
    bool exists[M * N] = {false};
    for (vector<int> pattern : patterns) {
        for (int number : pattern) {
            if (number <= 0 || number >= (M * N)) {
                cerr << "Error: Invalid patterns." << endl;
                exit(1);
            }
            if (exists[number]) {
                cerr << "Error: Duplicated numbers in the patterns." << endl;
                exit(1);
            }
            exists[number] = true;
        }
    }
    this->patterns = patterns;
}

void PatternDatabaseGenerator::setBaseBoard(PatternDbBoard baseBoard) {
    // set all movable cells to the largest number to deal with the zeros placed
    // afterwards
    for (int i = 0; i < M * N; i++) {
        if (baseBoard.operator()(i) == 0) baseBoard.operator()(i) = DONT_CARE_CELL;
    }
    this->baseBoard = baseBoard;
}

PatternDatabaseGenerator::PatternDatabaseGenerator(PatternDbBoard baseBoard,
                                                   vector<vector<int>> patterns) {
    this->setBaseBoard(baseBoard);
    this->setPatterns(patterns);
}

int PatternDatabaseGenerator::solveSingleBoard(PatternDbBoard& board) {
    AStarSolver solver;
    Board* boardToSolve = new PatternDbBoard(board);
    solver.init(boardToSolve);
    Board terminalBoard = solver.solve();
    int moveCnt = terminalBoard.getPrevMoves().size();

    solver.deleteAll();
    return moveCnt;
}

void PatternDatabaseGenerator::generateAllZeroOnlyInitials(
    int remainingCnt, int startId, PatternDbBoard& startBoard,
    vector<PatternDbBoard>* results) {
    if (remainingCnt > 0 && startId >= M * N) return;
    if (remainingCnt == 0) {
        results->push_back(startBoard);
        return;
    }
    for (int i = startId; i < M * N; i++) {
        if (startBoard.operator()(i) != DONT_CARE_CELL) continue;
        startBoard.operator()(i) = 0;  // set
        this->generateAllZeroOnlyInitials(remainingCnt - 1, i + 1, startBoard, results);
        startBoard.operator()(i) = DONT_CARE_CELL;  // reset
    }
}

vector<PatternDbBoard> PatternDatabaseGenerator::generateAllZeroOnlyInitials() {
    vector<PatternDbBoard> allZeroOnlyInitials;
    PatternDbBoard startBoard(this->baseBoard);
    this->generateAllZeroOnlyInitials(N_EMPTY, 0, startBoard, &allZeroOnlyInitials);
    return allZeroOnlyInitials;
}

void PatternDatabaseGenerator::generate(PatternDbBoard& startBoard,
                                        vector<int>& remaining, PatternDatabase& pattDb) {
    // all numbers are filled in, write result to database
    if (remaining.size() == 0) {
        int moveCnt = this->solveSingleBoard(startBoard);
        pattDb.write(startBoard, moveCnt);
        return;
    }

    // fill in the last number
    int numToFillIn = remaining.back();
    remaining.pop_back();
    for (int pos = 0; pos < M * N; pos++) {
        if (startBoard.operator()(pos) == DONT_CARE_CELL) {
            startBoard.operator()(pos) = numToFillIn;  // set number
            this->generate(startBoard, remaining, pattDb);
            startBoard.operator()(pos) = DONT_CARE_CELL;  // reset number
        }
    }
    remaining.push_back(numToFillIn);
}

void PatternDatabaseGenerator::generate() {
    for (vector<int> pattern : this->patterns) {
        PatternDatabase pattDb(PATT_DB_INITIAL_DIR + pattern2Str(pattern), Mode::write);
        vector<PatternDbBoard> allZeroOnlyInitials = this->generateAllZeroOnlyInitials();
        for (PatternDbBoard& startBoard : allZeroOnlyInitials) {
            startBoard.init(pattern);
            this->generate(startBoard, pattern, pattDb);
        }
        cout << PATT_DB_INITIAL_DIR + pattern2Str(pattern) << " generated" << endl;
    }
}

void readPatterns(PatternDbBoard* baseBoard, vector<vector<int>>* patterns) {
    cin >> (Board*)baseBoard;
    cin.ignore();  // ignore newline

    string line;
    while (getline(cin, line)) {
        stringstream ss(line);
        vector<int> numbers;
        int num;
        while (ss >> num) {
            numbers.push_back(num);
        }
        patterns->push_back(numbers);
    }
}

int main() {
    PatternDbBoard pattDbBoard;
    vector<vector<int>> patterns;
    readPatterns(&pattDbBoard, &patterns);

    PatternDatabaseGenerator pattDbGen(pattDbBoard, patterns);
    pattDbGen.generate();

    return 0;
}