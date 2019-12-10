// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file greedy.cpp
	\brief baseline agents
	 -D GREEDY, greedy movement evaluated by eval() below
	 -D CONSERVATIVE, conservative strategy, eats opponent, or do hor/vert,
	  if none of the above move exists, do random
	 -D RANDOM, do random move from move_list()
	 Add log: -D PLAYER1/PLAYER2
	\author Yueh-Ting Chen (eopXD)
	\course Theory of Computer Game (TCG)
*/

#include <cstdlib>
#include <ctime>
#include <cmath>

#include <iostream>
#include <fstream>
#include <utility>
#include <chrono>
#include <algorithm>
#include <random>

#include "einstein.hpp"

// MCTS parameters
const float UCB_C = sqrt(2);
const int MAX_ITERATION = 50000; // 0: unlimited
const float MAX_SECOND = 10.0;
const int SIMULATION_BATCH = 50;
// PP parameters
const int PP_MIN_SIM = 100;
const float PP_NUM_SIGMA = 0.2;
const float PP_SIGMA_EPSILON = 0.4;

char start;
char init[2][NUM_CUBE+1] = {};
BOARD_GUI *b, tmp_b;
bool myturn;
inline void flip_bit ( bool &_ ) { _ = !_; }
char num, dir;
std::fstream flog;
void logger ( std::string logfile ) {
	flog.open(logfile, std::fstream::out);
	if ( !flog.is_open() ) {
		throw std::runtime_error("error opening file\n");
	}
}
using PII = std::pair<int, int>;

typedef struct _POS{
	int x;
	int y;
} POS;

float simulation(BOARD_GUI b);

POS idxToPos(const int &idx){
	POS newPOS;
	newPOS.x = idx/BOARD_SZ;
	newPOS.y = idx%BOARD_SZ;
	return newPOS;
}

typedef struct _NODE{
	using ULL = unsigned long long;
	using PII = std::pair<int, int>;
	using PSS = std::pair<std::string, std::string>;
	using VII = std::vector<PII>;
	
	_NODE* parent;
	float value;
	float sumOfSquaredValue;
	int num_visits;
	bool pruned;
	int numChildLeft;
	BOARD_GUI board;
	std::vector<_NODE*> child;
	VII moveToExpand;
	VII movedList; // used for debug

	_NODE(){}

	float getStdDev(){
		return sqrt((sumOfSquaredValue / num_visits) - pow(value/num_visits, 2));
	}

	float getWinRate(){
		// flog << "value: " << value << ", num_visit: " << (float)num_visits << std::endl;
		return (value / (float)num_visits);
	}

	_NODE* getBestChild(bool decideMove = false){
		
		// Edge cases check
		if(child.size() == 1){
			// flog << "Only one child" << std::endl;
			return child[0];
		}else if(numChildLeft == 1){
			for(int i=0; i<child.size(); ++i){
				if(!child[i]->pruned){
					return child[i];
				}
			}
		}

		// Progressive Pruning
		if(!decideMove){
			std::vector<int> vIdxForPP;
			for(int i=0; i<child.size(); ++i){
				// hard pruning
				if(!child[i]->pruned && 
					child[i]->num_visits >= PP_MIN_SIM &&
					child[i]->getStdDev() < PP_SIGMA_EPSILON){
					// flog << "Found confident candidate" << std::endl;
					vIdxForPP.push_back(i);
				}
			}
			if(vIdxForPP.size() > 1){
				float bestWinRate = child[vIdxForPP[0]]->getWinRate();
				int bestIdx = vIdxForPP[0];
				float lowerBound;
				for(int i=1; i<vIdxForPP.size(); ++i){
					float tmp_winrate = child[vIdxForPP[i]]->getWinRate();
					if(tmp_winrate > bestWinRate){
						bestWinRate = tmp_winrate;
						bestIdx = vIdxForPP[i];
					}
				}
				
				flog << "best win rate: " << bestWinRate << ", stdDev: " << child[bestIdx]->getStdDev() << std::endl;
				lowerBound = bestWinRate - (PP_NUM_SIGMA * child[bestIdx]->getStdDev());
				for(int i=0; i<vIdxForPP.size(); ++i){
					if(vIdxForPP[i] == bestIdx){
						continue;
					}

					float tmp_winrate = child[vIdxForPP[i]]->getWinRate();
					float tmp_stdDev = child[vIdxForPP[i]]->getStdDev();
					float confidentUpperBound = tmp_winrate + (PP_NUM_SIGMA * tmp_stdDev);

					// flog << "Lower bound: " << lowerBound << " | upper: " << confidentUpperBound << std::endl;					
					if(lowerBound > confidentUpperBound){
						child[vIdxForPP[i]]->pruned = true;
						--numChildLeft;
						flog << "\tPruned one child lower: " << lowerBound << " | upper: " << confidentUpperBound << std::endl;

					}
				}
			}
		}

		// Getting best child
		float bestUCB = -99999.0;
		_NODE* bestChild = NULL;

		bool turn = board._turn;

		// flog << "\tGetting best child..., child size: " << child.size() << std::endl;
		if(decideMove){
			for(int i=0; i<child.size(); ++i){
				// Skip pruned children
				if(child[i]->pruned) continue;

				float winRate = (turn == 0)? (float)child[i]->value / (child[i]->num_visits) : -(float)child[i]->value / (child[i]->num_visits);

				// flog << "\t\tUCT score: " << uct_score << std::endl;

				if(winRate > bestUCB){
					bestChild = child[i];
					bestUCB = winRate;
				}
			}
		}else{
			for(int i=0; i<child.size(); ++i){
				// Skip pruned children
				if(child[i]->pruned) continue;

				float uct_exploitation = (turn == 0)? (float)child[i]->value / (child[i]->num_visits) : -(float)child[i]->value / (child[i]->num_visits);
				float uct_exploration = sqrt( log((float)num_visits) / (child[i]->num_visits) );
				float uct_score = uct_exploitation + UCB_C * uct_exploration;

				// flog << "\t\tUCT score: " << uct_score << std::endl;

				if(uct_score > bestUCB){
					bestChild = child[i];
					bestUCB = uct_score;
				}
			}
		}
		// flog << "\tGetting best child..., child: " << (bestChild == NULL) << " | " << bestUCB << std::endl;
		return bestChild;
	}

	void construct(BOARD_GUI b, _NODE* p){
		board = b;
		parent = p;
		num_visits = 0;
		numChildLeft = 0;
		value = 0.0;
		sumOfSquaredValue = 0.0;
		pruned = false;
	}

	_NODE* addChildWithMove(PII &m){
		// flog << "adding child... " << board.send_move(m) << std::endl;
		_NODE* newNode = new _NODE;
		// flog << "adding child...0" << std::endl;
		newNode->construct(board, this);

		// BOARD_GUI b_tmp = newNode->board;
		// auto ml = b_tmp.move_list();
		// flog << "==========MoveList========== P:" << enum2int(b_tmp.turn()) << std::endl;
		// for(int i=0; i<ml.size(); ++i){
		// 	flog << "[" << ml.at(i).first << ", " << ml.at(i).second << "] ";
		// }
		// flog << std::endl << "==========MoveList==========" << std::endl;
		// for(int i=0; i<6; ++i){
		// 	for(int j=0; j<6; ++j){
		// 		int idx = (i*6)+j;
		// 		if(!b_tmp.now[idx].c){
		// 			flog << "  ";
		// 		}else if(b_tmp.now[idx].c->c == Color::R){
		// 			char c = b_tmp.now[idx].c->num + 'A';
		// 			flog << c << ' ';
		// 		}else{
		// 			flog << b_tmp.now[idx].c->num << ' ';
		// 		}
		// 	}
		// 	flog << std::endl;
		// }

		// flog << "adding child...1" << std::endl;
		newNode->board.do_move(m);
		newNode->moveToExpand = newNode->board.move_list();
		auto rng = std::default_random_engine {};
		std::shuffle(newNode->moveToExpand.begin(), newNode->moveToExpand.end(), rng);
		// flog << "adding child...2" << std::endl;
		child.push_back(newNode);
		// flog << "adding child...3" << std::endl;
		return newNode;
	}

	void expandOneLevel(){
		auto ml = board.move_list();
		for(int i=0; i<ml.size(); ++i){
			auto m = ml.at(i);
			_NODE* newNode = addChildWithMove(m);
			child.push_back(newNode);
		}
	}

	_NODE* expandOneLeaf(){
		if(fullExpanded()) return NULL;
		else{
			
			// flog << "==========MoveList========== P:" << enum2int(board.turn()) << "(in expandOneLeaf)" << std::endl;
			// for(int i=0; i<moveToExpand.size(); ++i){
			// 	flog << "[" << moveToExpand.at(i).first << ", " << moveToExpand.at(i).second << "] ";
			// }
			// flog << std::endl << "==========MoveList==========" << std::endl;
			
			// for(int i=0; i<movedList.size(); ++i){
			// 	flog << "[" << movedList.at(i).first << ", " << movedList.at(i).second << "] ";
			// }
			// flog << std::endl << "==========MovedList==========" << std::endl;


			// flog << "expanding..." << std::endl;
			auto m = moveToExpand.back();
			movedList.push_back(m);
			// flog << "expanding...0" << std::endl;
			moveToExpand.pop_back();
			// flog << "expanding...1" << std::endl;
			_NODE* newChild = addChildWithMove(m);
			++numChildLeft;
			// flog << "expanding...2" << std::endl;
			return newChild;
		}
	}

	_move getBestMove(){
		return getBestChild()->board.history.back();
	}

	void doSimulation(int batchSize = SIMULATION_BATCH){
		float additionalSimVal = 0.0;
		float additionalSimValSquare = 0.0;

		for(int i=0; i<batchSize; ++i){
			float simVal = simulation(board);
			additionalSimVal += simVal;
			additionalSimValSquare += pow(simVal, 2.0);
		}

		_NODE* node = this;
		while(node){
			node->num_visits += batchSize;
			node->value += additionalSimVal;
			node->sumOfSquaredValue += additionalSimValSquare;
			node = node->parent;
		}

		return;
	}

	// 0 = not over
	// 1 = player R wins, 2 = player B wins, 3 = draw
	bool isTerminal(){
		return (board.state() != 0);
	}

	bool fullExpanded(){
		return (moveToExpand.size() == 0);
	}
} NODE;

void freeMemNode(NODE* root){
	
	if(root == NULL){
		return;
	}else{
		for(int i=0; i<root->child.size(); ++i){
			freeMemNode(root->child[i]);
		}
		delete root;
	}

	return;
}

float simulation(BOARD_GUI b){
	// bool turn = b._turn; // simulation i.t.o red/blue

	while(b.state() == 0){
		// TODO some good random
		auto ml = b.move_list();
		auto m = ml.at(rand()%ml.size());
		b.do_move(m);
	}

	// TODO some better eval
	float res;
	if(b.state() == 1){ // RED wins
		res = 1.0;
	}else if(b.state() == 2){ // BLUE wins
		res = -1.0;
	}else{
		res = 0.0;
	}

	// res = (turn == 0)? res : -res;
	
	return res;
}

int main () 
{

	logger(".log.progressive");

	auto timer = [] ( bool reset = false ) -> double {
		static decltype(std::chrono::steady_clock::now()) tick, tock;
		if ( reset ) {
			tick = std::chrono::steady_clock::now();
			return (0);
		}
		tock = std::chrono::steady_clock::now();
		return (std::chrono::duration_cast<std::chrono::duration<double>>(tock-tick).count());
	};

	srand(time(NULL));
	// srand(1);

	do {
		/* get initial positions */
		for ( int i=0; i<2; ++i ) { for ( int j=0; j<NUM_CUBE; ++j ) {
				init[i][j] = getchar();
		}}
		init[0][NUM_CUBE] = init[1][NUM_CUBE] = '\0';
		start = getchar();

		flog << init[0] << " " << init[1] << std::endl;
		flog << start << std::endl;

		/* game start s*/
		b = new BOARD_GUI(init[0], init[1]);
		b->no_hl = 1;
		bool passed = false;

		for ( myturn=(start=='f'); b->winner()==Color::OTHER; flip_bit(myturn) ) {
			if ( myturn ) {

				timer(true);
				if(passed){
					PII m = std::make_pair(15, 15);
					// flog << "(PASSED) Turn: " << myturn << " | " << b->send_move(m) << std::endl;
					b->do_move(m);
					std::cout << b->send_move(m) << std::flush;
					continue;
				}

				auto ml = b->move_list();
				if(ml.size() == 1 && ml.at(0) == std::make_pair(15, 15)){
					// flog << "Meet the passed section!" << std::endl;
					// flog << "passed" << std::endl;
					passed = true;
					PII m = std::make_pair(15, 15);
					// flog << "(PASSED) Turn: " << myturn << " | " << b->send_move(m) << std::endl;
					b->do_move(m);
					std::cout << b->send_move(m) << std::flush;
					continue;
				}
				
				// decide move
				int iteration = 0;

				// construct root node
				NODE* root = new NODE;
				root->construct(*b, NULL);
				root->moveToExpand = root->board.move_list();

				while(true){
					// flog << "iter: " << iteration << std::endl;
					
					// Step 1: SELECT
					// - start from the root, top-down traverse according to the UCB scores
					// - stop when meet terminal nodes or nodes not fully expanded yet
					NODE* node = root;
					while(!node->isTerminal() && node->fullExpanded()) {
						// flog << "traverse...  " ;
                        node = node->getBestChild();
						// flog << "traversed." << std::endl;
					}

					// Step 2: EXPAND
					if(!node->fullExpanded() && !node->isTerminal()){
						// flog << "expand" << std::endl;
						node = node->expandOneLeaf();
						if (node == NULL){
							// flog << "\texpand failed" << std::endl;
							exit(0);
						}
						// flog << "expanded." << std::endl;
					}

					// Step 3: SIMULATE
					// Step 4: BACK PROPAGATE
					node->doSimulation();

					iteration += SIMULATION_BATCH;
					if(MAX_ITERATION > 0 && iteration >= MAX_ITERATION) break;
					if(MAX_SECOND > 0.0 && timer() >= MAX_SECOND) break;
				}


				// auto ml = b->move_list();
				// auto m = ml.at(rand()%ml.size());				

				// flog << "Getting last and best move..." << std::endl;
				
				NODE* n = root->getBestChild(true);
				PII m;
				if(n){
					m = n->board.getLastMove();
				}else{
					m = std::make_pair(15, 15);
				}

				flog << "Turn: " << myturn << " | " << b->send_move(m) << std::endl;
				b->do_move(m);
				std::cout << b->send_move(m) << std::flush;
				freeMemNode(root);

				// flog << "Time spent: " << timer() << std::endl;

			}
			else {
				num = getchar()-'0';
				dir = getchar()-'0';
				flog << "Turn: " << myturn << " | " << (int)num << (int)dir << std::endl;
				if ( num == 16 ) {
					b->undo_move();
					b->undo_move();
					flip_bit(myturn);
				}
				else {
					b->do_move(num, dir);
				}
			}
		}
		flog << "winner: " << b->winner() << std::endl;
		/* game end */
		delete b;
	} while ( getchar()=='y' ); 

	return (0);
}