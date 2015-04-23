#include <iostream>
#include <algorithm>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <utility>
#include <iterator>
#include <iostream>
using namespace std;
#include "Matrix.h"

//#include <wx\wx.h>


// print number of moves considered and time used
#define ANALYSE
#ifdef ANALYSE
#ifdef WIN32
#include <windows.h>
#else
#include <ctime>
#endif    
#endif

// This solution only starts saves boards if DEPTH >= 3 and DEPTH <= MAX_DEPTH
// For DEPTH < 3 no transpositions can be found
// For DEPTH > MAX_DEPTH calculation is faster than lookup 

const int MAX_DEPTH = 16;
const int MIN_DEPTH = 3;

class HEX {
public:
	enum Side { EMPTY, HUMAN, COMPUTER };
	enum Value { HUMAN_WINS = -1, DRAW, COMPUTER_WINS, UNDECIDED };
	HEX()
#ifdef ANALYSE
		: movesConsidered(0)
#endif
	{
		fill(board.begin(), board.end(), EMPTY);
	}
	Value chooseComputerMove(int& bestRow, int& bestColumn, Value alpha = HUMAN_WINS, Value beta = COMPUTER_WINS, int depth = 1);
	Value chooseHumanMove(int& bestRow, int& bestColumn, Value alpha = HUMAN_WINS, Value beta = COMPUTER_WINS, int depth = 1);
	Side side(int row, int column) const;
	bool isUndecided() const;
	bool playMove(Side s, int row, int column);
	bool boardIsFull() const;
	bool isAWin(Side s) const;
	bool checkEdge(Side s) const;
	bool checkPath(Side s, int x, int y, set<pair<int, int>> &pos) const;
	void filVector();
	int numCols() const;
	int numRows() const;
#ifdef ANALYSE
	int getAndResetMovesConsidered() {
		int i = movesConsidered;
		movesConsidered = 0;
		return i;
	}
	int getMapSize() const {
		return boards.size();
	}
#endif
private:
	typedef matrix<Side, 3, 3> Board;
	Board board;
	Value value() const;
	vector<pair<int, int>> location;
	class BoardWrapper {
	public:
		BoardWrapper(const Board& b) : board(b) {
		}
		bool operator<(const BoardWrapper& rhs) const;
	private:
		Board board;
	};
	class ValueAndBestMove {
	public:
		ValueAndBestMove() = default;
		ValueAndBestMove(Value v, int r, int c) : value(v), bestRow(r), bestColumn(c) {
		}
		Value value;
		int bestRow;
		int bestColumn;
	};
	map<BoardWrapper, ValueAndBestMove> boards;
#ifdef ANALYSE
	int movesConsidered;
#endif
};

class ConsoleHEXGame {
public:
	explicit ConsoleHEXGame(bool computerGoesFirst);
	void play();
private:
	void printBoard() const;
	void doComputerMove();
	HEX t;
	char computerSymbol;
	char humanSymbol;
};

#ifdef ANALYSE
class StopWatch {
public:
	StopWatch();
	void start();
	void stop();
	double time() const;
private:
	bool running;
	void printError() const;
#ifdef WIN32
	double f;
	long long start_time;
	long long total_time;
	long long getExecutionTime() const;
#else
	clock_t start_time;
	clock_t total_time;
	clock_t getExecutionTime() const;
#endif
};
ostream& operator<<(ostream& o, const StopWatch& sw);
#endif

HEX::Value HEX::value() const {
	return isAWin(COMPUTER) ? COMPUTER_WINS : isAWin(HUMAN) ? HUMAN_WINS : boardIsFull() ? DRAW : UNDECIDED;
}

HEX::Value HEX::chooseComputerMove(int& bestRow, int& bestColumn, Value alpha, Value beta, int depth) {
#ifdef ANALYSE
	++movesConsidered;
#endif
	if (depth >= MIN_DEPTH && depth <= MAX_DEPTH) {
		auto itr = boards.find(BoardWrapper(board));
		if (itr != boards.end()) {
			bestRow = itr->second.bestRow;
			bestColumn = itr->second.bestColumn;
			return itr->second.value;
		}
	}
	Value bestValue = value();
	if (bestValue == UNDECIDED) {
		for (int row = 0; alpha < beta && row < board.numRows(); ++row) {
			for (int column = 0; alpha < beta && column < board.numCols(); ++column) {
				if (board(row, column) == EMPTY) {
					board(row, column) = COMPUTER;
					int dummyRow, dummyColumn;
					Value value = chooseHumanMove(dummyRow, dummyColumn, alpha, beta, (depth + 1));
					board(row, column) = EMPTY;
					if (value >= alpha) {
						alpha = value;
						bestRow = row;
						bestColumn = column;
					}
				}
			}
		}
		bestValue = alpha;
	}
	if (depth >= MIN_DEPTH && depth <= MAX_DEPTH) {
		boards[BoardWrapper(board)] = ValueAndBestMove(bestValue, bestRow, bestColumn);
	}
	return bestValue;
}

HEX::Value HEX::chooseHumanMove(int& bestRow, int& bestColumn, Value alpha, Value beta, int depth) {
#ifdef ANALYSE
	++movesConsidered;
#endif
	if (depth >= MIN_DEPTH && depth <= MAX_DEPTH) {
		auto itr = boards.find(BoardWrapper(board));
		if (itr != boards.end()) {
			bestRow = itr->second.bestRow;
			bestColumn = itr->second.bestColumn;
			return itr->second.value;
		}
	}
	Value bestValue = value();
	if (bestValue == UNDECIDED) {
		for (int row = 0; alpha < beta && row < board.numRows(); ++row) {
			for (int column = 0; alpha < beta && column < board.numCols(); ++column) {
				if (board(row, column) == EMPTY) {
					board(row, column) = HUMAN;
					int dummyRow, dummyColumn;
					Value value = chooseComputerMove(dummyRow, dummyColumn, alpha, beta, (depth + 1));
					board(row, column) = EMPTY;
					if (value <= beta) {
						beta = value;
						bestRow = row;
						bestColumn = column;
					}
				}
			}
		}
		bestValue = beta;
	}
	if (depth >= MIN_DEPTH && depth <= MAX_DEPTH) {
		boards[BoardWrapper(board)] = ValueAndBestMove(bestValue, bestRow, bestColumn);
	}
	return bestValue;
}

HEX::Side HEX::side(int row, int column) const {
	return board(row, column);
}

void HEX::filVector()
{
	location = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 } };
}

bool HEX::isUndecided() const {
	return value() == UNDECIDED;
}

bool HEX::playMove(Side s, int row, int column) {
	if (row < 0 || row >= board.numRows() || column < 0 || column >= board.numCols() || board(row, column) != EMPTY)
		return false;
	board(row, column) = s;
	return true;
}

bool HEX::boardIsFull() const {
	return none_of(board.cbegin(), board.cend(), [](Side s) {
		return s == EMPTY;
	});
}

bool HEX::isAWin(Side s) const{
	set<pair<int, int>> pos;
	/* Kijk of we al aan de rand zitten */
	if (!checkEdge(s))
	{
		return false;
	}
	if (s == HUMAN)
	{
		for (int i = 0; i < board.numCols(); i++)
		{
			if (board(i, 0) == s)
			{
				if (checkPath(s, i, 0, pos))
				{
					return true;
				}
			}
		}
	}
	else if (s == COMPUTER)
	{
		for (int i = 0; i < board.numRows(); i++)
		{
			if (board(0, i) == s)
			{
				if (checkPath(s, 0, i, pos))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool HEX::checkPath(Side s, int row, int column, set<pair<int, int>> &pos) const
{
	if (s == HUMAN && column == (board.numCols() - 1))
	{
		return true;		// Einde van het bord bereikt: mens heeft gewonnen
	}
	else if (s == COMPUTER && row == (board.numRows() - 1))
	{
		return true;		// Einde van het bord bereikt: computer heeft gewonnen
	}
	pos.insert(make_pair(row, column));
	for (int i = 0; i <= 5; i++)
	{
		if (row + location[i].first >= 0 && row + location[i].first < board.numRows())
		{
			if (column + location[i].second >= 0 && column + location[i].second < board.numCols())
			{
				if (pos.count(make_pair(row + location[i].first, column + location[i].second)) == NULL)
				{
					if (board(row + location[i].first, column + location[i].second) == s)
					{
						if (checkPath(s, row + location[i].first, column + location[i].second, pos))
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool HEX::checkEdge(Side s) const {
	bool side1 = false, side2 = false;
	if (s == COMPUTER)
	{
		for (int i = 0; i < board.numCols(); i++)
		{
			if (board(0, i) == s)
			{
				side1 = true;
			}
			if (board((board.numCols() - 1), i) == s)
			{
				side2 = true;
			}
		}
		return (side1 && side2);
	}
	else if (s == HUMAN)
	{
		for (int i = 0; i < board.numRows(); i++)
		{
			if (board(i, 0) == s)
			{
				side1 = true;
			}
			if (board(i, (board.numRows() - 1)) == s)
			{
				side2 = true;
			}
		}
		return (side1 && side2);
	}
}

int HEX::numCols() const
{
	return board.numCols();
}

int HEX::numRows() const
{
	return board.numRows();
}

bool HEX::BoardWrapper::operator<(const BoardWrapper& rhs) const {
	for (int i = 0; i < board.numRows(); ++i) {
		for (int j = 0; j < board.numCols(); ++j) {
			if (board(i, j) != rhs.board(i, j)) {
				return board(i, j) < rhs.board(i, j);
			}
		}
	}
	return false;
}

ConsoleHEXGame::ConsoleHEXGame(bool computerGoesFirst) :
computerSymbol(computerGoesFirst ? 'x' : 'o'), humanSymbol(computerGoesFirst ? 'o' : 'x') {
	t.filVector();
	if (computerGoesFirst) {
		doComputerMove();
		cout << endl;
	}
}

void ConsoleHEXGame::printBoard() const {
	string streep(t.numCols()*2, '-');
	cout << streep << endl;
	for (int row = 0; row < t.numRows(); ++row) {
		for (int i = 0; i < row; i++)
		{
			cout << " ";
		}
		cout << "\\";
		for (int column = 0; column < t.numCols(); ++column)
		{
			if (column != 0)
			cout << "|";
			if (t.side(row, column) == HEX::COMPUTER)
				cout << computerSymbol;
			else if (t.side(row, column) == HEX::HUMAN)
				cout << humanSymbol;
			else
				cout << ' ';
		}
		cout << "\\" << endl;
	}
	for (int i = 0; i < t.numCols(); i++)
	{
		cout << " ";
	}
	cout << streep << endl;
}

void ConsoleHEXGame::doComputerMove() {
	int bestRow, bestColumn;
#ifdef ANALYSE
	StopWatch sw;
	sw.start();
#endif
	t.chooseComputerMove(bestRow, bestColumn);
#ifdef ANALYSE
	sw.stop();
	cout << "Calculation time: " << sw << endl;
	cout << "Moves considered: " << t.getAndResetMovesConsidered() << endl;
#endif
	cout << "Computer plays: ROW = " << bestRow << " COLUMN = " << bestColumn << endl;
	t.playMove(HEX::COMPUTER, bestRow, bestColumn);
}

void ConsoleHEXGame::play() {
	do {
		int row, column;
		do {
			printBoard();
			cout << endl << "Enter row and column (starts at 0): ";
			cin >> row >> column;
		} while (!t.playMove(HEX::HUMAN, row, column));
		cout << endl;
		if (t.isUndecided()) {
			printBoard();
			cout << endl;
			doComputerMove();
			cout << endl;
		}
	} while (t.isUndecided());
	printBoard();
	if (t.isAWin(HEX::COMPUTER)) {
		cout << "Computer wins!!" << endl;
	}
	else if (t.isAWin(HEX::HUMAN)) {
		cout << "Human wins!!" << endl;
	}
	else {
		cout << "Draw!!" << endl;
	}
}

#ifdef ANALYSE
StopWatch::StopWatch() : running(false), total_time(0) {
#ifdef WIN32
	LARGE_INTEGER performanceFrequency;
	if (QueryPerformanceFrequency(&performanceFrequency) == 0) {
		printError();
	}
	f = double(performanceFrequency.QuadPart);
#endif
}

void StopWatch::start() {
	if (!running) {
		running = true;
		start_time = getExecutionTime();
	}
}

void StopWatch::stop() {
	if (running) {
		running = false;
		total_time = getExecutionTime() - start_time;
	}
}

#ifdef WIN32
long long StopWatch::getExecutionTime() const {
	LARGE_INTEGER performanceCount;
	if (QueryPerformanceCounter(&performanceCount) == 0) {
		printError();
		return 0;
	}
	return performanceCount.QuadPart;
}
#else
clock_t StopWatch::getExecutionTime() const {
	clock_t t = clock();
	if (t == -1) {
		printError();
		return 0;
	}
	return clock();
}
#endif

void StopWatch::printError() const {
#ifdef WIN32
	LPTSTR lpMsgBuf(0);
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		lpMsgBuf,
		0,
		NULL
		);
	cerr << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
#else
	cerr << "No timer available\n" << endl;
#endif
}

double StopWatch::time() const {
#ifdef WIN32
	long long res = running ? getExecutionTime() - start_time : total_time;
	return res / f;
#else
	clock_t res = running ? getExecutionTime() - start_time : total_time;
	return static_cast<double>(res) / CLK_TCK;
#endif
}

ostream& operator<<(ostream& o, const StopWatch& sw) {
	return o << sw.time() << " sec";
}
#endif

int main() {
	cout << "Welcome to HEX" << endl;
	cout << "Move horizontal" << endl;
	bool computerGoesFirst = true;
	char again;
	do {
		ConsoleHEXGame game(computerGoesFirst);
		game.play();
		do {
			cout << "Play again (y/n)? ";
			cin >> again;
		} while (again != 'y' && again != 'Y' && again != 'n' && again != 'N');
		computerGoesFirst = !computerGoesFirst;
		cout << endl;
	} while (again != 'n' && again != 'N');
	return 0;
}