#include<iostream>
#include<stdio.h>
#include<vector>
#include<array>
#include <random>
#include <algorithm>

#include "mcts.h"
// #include"connect4.h"

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
// #include <limits.h>
#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX - 1)
// #include <sstream>
// #include"connect4.h"
using namespace std;
using namespace connect4_judge;
//functions
void printBoard(vector<vector<int> >&);
int userMove();
void makeMove(vector<vector<int> >&, int, unsigned int);
void errorMessage(int);
int aiMove();
vector<vector<int> > copyBoard(vector<vector<int> >);
bool winningMove(vector<vector<int> >&, unsigned int);
int scoreSet(vector<unsigned int>, unsigned int);
int tabScore(vector<vector<int> >, unsigned int);
array<int, 2> miniMax(vector<vector<int> >&, unsigned int, int, int, unsigned int);
int heurFunction(unsigned int, unsigned int, unsigned int);
//parameters
int PLAYER=1, COMPUTER=2, NUM_COL=7,NUM_ROW=6,turns=0;
vector<vector<int>> board(NUM_ROW, vector<int>(NUM_COL));


void initBoard() {
	for (unsigned int r = 0; r < NUM_ROW; r++) {
		for (unsigned int c = 0; c < NUM_COL; c++) {
			board[r][c] = 0; // make sure board is empty initially
		}
	}
}



array<int, 2> miniMax(vector<vector<int> > &b, unsigned int d, int alf, int bet, unsigned int p) {
	/**
	 * if we've reached minimal depth allowed by the program
	 * we need to stop, so force it to return current values
	 * since a move will never (theoretically) get this deep,
	 * the column doesn't matter (-1) but we're more interested
	 * in the score
	 *
	 * as well, we need to take into consideration how many moves
	 * ie when the board is full
	 */
    int last_possible_column=-1;
	if (d == 0 || d >= (NUM_COL * NUM_ROW) - turns) {
		// get current score to return
		return array<int, 2>{tabScore(b, COMPUTER), -1};
	}
	if (p == COMPUTER) { // if AI player
		array<int, 2> moveSoFar = {INT_MIN, -1}; // since maximizing, we want lowest possible value
		if (winningMove(b, PLAYER)) { // if player about to win
			return moveSoFar; // force it to say it's worst possible score, so it knows to avoid move
		} // otherwise, business as usual
		for (unsigned int c = 0; c < NUM_COL; c++) { // for each possible move
			if (b[NUM_ROW - 1][c] == 0) { // but only if that column is non-full
                last_possible_column=c;
				vector<vector<int> > newBoard = copyBoard(b); // make a copy of the board
				makeMove(newBoard, c, p); // try the move
				int score = miniMax(newBoard, d - 1, alf, bet, PLAYER)[0]; // find move based on that new board state
				if (score > moveSoFar[0]) { // if better score, replace it, and consider that best move (for now)
					moveSoFar = {score, (int)c};
				}
				alf = max(alf, moveSoFar[0]);
				if (alf >= bet) { break; } // for pruning
			}
		}
        if(moveSoFar[1]==-1){
            moveSoFar[1]=last_possible_column;
        }
		return moveSoFar; // return best possible move
	}
	else {
		array<int, 2> moveSoFar = {INT_MAX, 5}; // since PLAYER is minimized, we want moves that diminish this score
		if (winningMove(b, COMPUTER)) {
			return moveSoFar; // if about to win, report that move as best
		}
		for (unsigned int c = 0; c < NUM_COL; c++) {
			if (b[NUM_ROW - 1][c] == 0) {
				vector<vector<int> > newBoard = copyBoard(b);
				makeMove(newBoard, c, p);
				int score = miniMax(newBoard, d - 1, alf, bet, COMPUTER)[0];
				if (score < moveSoFar[0]) {
					moveSoFar = {score, (int)c};
				}
				bet = min(bet, moveSoFar[0]);
				if (alf >= bet) { break; }
			}
		}
		return moveSoFar;
	}
}

int tabScore(vector<vector<int> > b, unsigned int p) {
	int score = 0;
	vector<unsigned int> rs(NUM_COL);
	vector<unsigned int> cs(NUM_ROW);
	vector<unsigned int> set(4);
	/**
	 * horizontal checks, we're looking for sequences of 4
	 * containing any combination of AI, PLAYER, and empty pieces
	 */
	for (unsigned int r = 0; r < NUM_ROW; r++) {
		for (unsigned int c = 0; c < NUM_COL; c++) {
			rs[c] = b[r][c]; // this is a distinct row alone
		}
		for (unsigned int c = 0; c < NUM_COL - 3; c++) {
			for (int i = 0; i < 4; i++) {
				set[i] = rs[c + i]; // for each possible "set" of 4 spots from that row
			}
			score += scoreSet(set, p); // find score
		}
	}
	// vertical
	for (unsigned int c = 0; c < NUM_COL; c++) {
		for (unsigned int r = 0; r < NUM_ROW; r++) {
			cs[r] = b[r][c];
		}
		for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
			for (int i = 0; i < 4; i++) {
				set[i] = cs[r + i];
			}
			score += scoreSet(set, p);
		}
	}
	// diagonals
	for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
		for (unsigned int c = 0; c < NUM_COL; c++) {
			rs[c] = b[r][c];
		}
		for (unsigned int c = 0; c < NUM_COL - 3; c++) {
			for (int i = 0; i < 4; i++) {
				set[i] = b[r + i][c + i];
			}
			score += scoreSet(set, p);
		}
	}
	for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
		for (unsigned int c = 0; c < NUM_COL; c++) {
			rs[c] = b[r][c];
		}
		for (unsigned int c = 0; c < NUM_COL - 3; c++) {
			for (int i = 0; i < 4; i++) {
				set[i] = b[r + 3 - i][c + i];
			}
			score += scoreSet(set, p);
		}
	}
	return score;
}

int scoreSet(vector<unsigned int> v, unsigned int p) {
	unsigned int good = 0; // points in favor of p
	unsigned int bad = 0; // points against p
	unsigned int empty = 0; // neutral spots
	for (unsigned int i = 0; i < v.size(); i++) { // just enumerate how many of each
		good += (v[i] == p) ? 1 : 0;
		bad += (v[i] == PLAYER || v[i] == COMPUTER) ? 1 : 0;
		empty += (v[i] == 0) ? 1 : 0;
	}
	// bad was calculated as (bad + good), so remove good
	bad -= good;
	return heurFunction(good, bad, empty);
}

int heurFunction(unsigned int g, unsigned int b, unsigned int z) {
	int score = 0;
	if (g == 4) { score += 500001; } // preference to go for winning move vs. block
	else if (g == 3 && z == 1) { score += 5000; }
	else if (g == 2 && z == 2) { score += 500; }
	else if (b == 2 && z == 2) { score -= 501; } // preference to block
	else if (b == 3 && z == 1) { score -= 5001; } // preference to block
	else if (b == 4) { score -= 500000; }
	return score;
}

vector<vector<int> > copyBoard(vector<vector<int> > b) {
	vector<vector<int>> newBoard(NUM_ROW, vector<int>(NUM_COL));
	for (unsigned int r = 0; r < NUM_ROW; r++) {
		for (unsigned int c = 0; c < NUM_COL; c++) {
			newBoard[r][c] = b[r][c]; // just straight copy
		}
	}
	return newBoard;
}

bool winningMove(vector<vector<int> > &b, unsigned int p) {
	unsigned int winSequence = 0; // to count adjacent pieces
	// for horizontal checks
	for (unsigned int c = 0; c < NUM_COL - 3; c++) { // for each column
		for (unsigned int r = 0; r < NUM_ROW; r++) { // each row
			for (int i = 0; i < 4; i++) { // recall you need 4 to win
				if ((unsigned int)b[r][c + i] == p) { // if not all pieces match
					winSequence++; // add sequence count
				}
				if (winSequence == 4) { return true; } // if 4 in row
			}
			winSequence = 0; // reset counter
		}
	}
	// vertical checks
	for (unsigned int c = 0; c < NUM_COL; c++) {
		for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
			for (int i = 0; i < 4; i++) {
				if ((unsigned int)b[r + i][c] == p) {
					winSequence++;
				}
				if (winSequence == 4) { return true; }
			}
			winSequence = 0;
		}
	}
	// the below two are diagonal checks
	for (unsigned int c = 0; c < NUM_COL - 3; c++) {
		for (unsigned int r = 3; r < NUM_ROW; r++) {
			for (int i = 0; i < 4; i++) {
				if ((unsigned int)b[r - i][c + i] == p) {
					winSequence++;
				}
				if (winSequence == 4) { return true; }
			}
			winSequence = 0;
		}
	}
	for (unsigned int c = 0; c < NUM_COL - 3; c++) {
		for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
			for (int i = 0; i < 4; i++) {
				if ((unsigned int)b[r + i][c + i] == p) {
					winSequence++;
				}
				if (winSequence == 4) { return true; }
			}
			winSequence = 0;
		}
	}
	return false; // otherwise no winning move
}

void makeMove(vector<vector<int> >& b, int c, unsigned int p) {
	// start from bottom of board going up
	for (unsigned int r = 0; r < NUM_ROW; r++) {
		if (b[r][c] == 0) { // first available spot
			b[r][c] = p; // set piece
			break;
		}
	}
}


int mcts(Game game, int yp_move ){


	Game g = game;
	mcts_dawei mctswu(g, 0.3);

	return mctswu.do_mcts(10000);
}




int decide(Game game, int yp_move){
    
    int my_move;
    if(yp_move==-1){
        initBoard();
    }
    else{
        for(int i=0;i<6;i++){
            if(board[i][yp_move]==0){
                board[i][yp_move]=1;
                break;
            }
        }
    }
	std::cout << game << "\n";
    my_move=miniMax(board,6,0-INT_MAX,INT_MAX,COMPUTER)[1];
    for(int i=0;i<6;i++){
        if(board[i][my_move]==0){
            board[i][my_move]=2;
            break;
        }
    }
    return my_move;//index of the column
}