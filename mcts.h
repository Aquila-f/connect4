#include "connect4.h"

using namespace connect4_judge;
struct node{
	double win_count;
	double totalmove_count;
	int available_node_count;
	int move;
	std::vector<node*> level_vector;
	std::vector<int> random_vector;

	node() : win_count(0),totalmove_count(0),available_node_count(-1){}
};

class mcts_dawei{
public:
    mcts_dawei(Game& g, double c) : 
    mcts_g(g),c_(c),counter(0)
    {
        for(int i=0;i<7;i++){
            for(int j=0;j<7;j++){
                random_move_vec.push_back(j);
            }
        }
    };
	

	int do_mcts(int count){
		node* rootnode1 = new node[1];
		Game sim_g;
		while(counter < count){
			sim_g = mcts_g;
			win_flag = false;
			play_one_sequence(rootnode1, sim_g);
			updatenode(win_flag);
			counter++;
		}
		double max = 0;
		int move = 0;
		int tot = 0;
		for(auto a : rootnode1->level_vector){
			// outputnode(a);
			tot += a->totalmove_count;
			if(max <= a->totalmove_count){
				max = a->totalmove_count;
				move = a->move;
			}
		}
		for(auto a : release_node_vector){
			delete [] a;
		}
		
		if(tot != count){
			std::cout << tot << " : " << count << " : " << counter << "\n";
			exit(0);
		}
		// std::cout << move << "\n";
		return move;
	}

	void play_one_sequence(node*& rootnode , Game state){
		update_node_vector.push_back(rootnode);
		if(state.status == Game::P2_WIN) return;

		Game after = state;
		if(rootnode->available_node_count == -1){
			int heurtmp = 0;

			node* tmpnode = new node[7];
			release_node_vector.push_back(tmpnode);
			for (int i = 0; i < 7; i++){
				// node* tmpnode = new node[1];
				rootnode->level_vector.push_back(&tmpnode[i]);
				after = state;
				if(after.play(i) != Game::INVALID_MOVE){
					// std::cout << after;
					tmpnode[i].move = i;
					rootnode->random_vector.push_back(i);
					heurtmp ++;

				}
			}

			rootnode->available_node_count = heurtmp;
			if(heurtmp == 0) return;
			std::shuffle(rootnode->random_vector.begin(), rootnode->random_vector.end(), engine);
		}
		if(rootnode->random_vector.size() == 0) return;
		
		

		double maxtnoval = -1;
		double v;
		int max_board_place = 0;

		double n_n, n_w;

		for(auto board_place : rootnode->random_vector){
			if(rootnode->level_vector[board_place]->totalmove_count == 0){
				state.play(board_place);
				update_node_vector.push_back(rootnode->level_vector[board_place]);
				if(state.status == Game::P1_WIN){
					win_flag = true;
					return;
				}
				simulate_one_b(state);

				return;

			}else{
				
				n_n = rootnode->level_vector[board_place]->totalmove_count;
				n_w = rootnode->level_vector[board_place]->win_count;


				//normal mcts
				v = n_w/n_n+c_*sqrt(log(rootnode->totalmove_count)/n_n);
				
				if(maxtnoval <= v){
					maxtnoval = v;
					max_board_place = board_place;
				}
			}
		}
		
		state.play(max_board_place);
		
		
		

		// play_sequence.push_back(max_board_place);

		simulate_enemy_move(rootnode->level_vector[max_board_place], state);
	}

	void simulate_enemy_move(node*& rootnode, Game state){
		update_node_vector.push_back(rootnode);
		if(state.status == Game::P1_WIN){
			win_flag = true;
			return;
		}
		
		Game after = state;

		
		if(rootnode->available_node_count == -1){
			int heurtmp = 0;
			node* tmpnode = new node[7];
			release_node_vector.push_back(tmpnode);
			for (int i = 0; i < 7; i++){
				// node* tmpnode = new node[1];
				rootnode->level_vector.push_back(&tmpnode[i]);
				after = state;// std::cout << state;

				if(after.play(i) != Game::INVALID_MOVE){
					tmpnode[i].move = i;
					rootnode->random_vector.push_back(i);
					heurtmp ++;
				}
			}


			rootnode->available_node_count = heurtmp;
			if(heurtmp == 0){
				win_flag = true;
				return;
			}
			std::shuffle(rootnode->random_vector.begin(), rootnode->random_vector.end(), engine);
		}

		

		
		double maxtnoval = -1;
		double v;
		int max_board_place = -1;

		double n_n, n_w;

		for(auto board_place : rootnode->random_vector){
			if(rootnode->level_vector[board_place]->totalmove_count == 0){
				state.play(board_place);
				update_node_vector.push_back(rootnode->level_vector[board_place]);
				if(state.status == Game::P2_WIN){
					return;
				}
				
				simulate_one_w(state);
				// std::cout << state;
				return;
				
				exit(0);
			}else{
				n_n = rootnode->level_vector[board_place]->totalmove_count;
				n_w = rootnode->level_vector[board_place]->win_count;
				
				v = 1-(n_w/n_n)+c_*sqrt(log(rootnode->totalmove_count)/n_n);
				

				if(maxtnoval <= v){
					maxtnoval = v;
					max_board_place = board_place;
				}
			}
		}

		state.play(max_board_place);
		if(state.status == Game::P2_WIN){
			return;
		}
		// play_sequence.push_back(max_board_place);
		play_one_sequence(rootnode->level_vector[max_board_place], state);
	}

	void updatenode(bool win){
		if(win){
			for(int i=0;i<update_node_vector.size();i++){
				update_node_vector[i]->totalmove_count += 1;
				update_node_vector[i]->win_count += 1;
			}

		}else{
			for(int i=0;i<update_node_vector.size();i++){
				
				update_node_vector[i]->totalmove_count += 1;

			}
		}
		update_node_vector.clear();
	}


	void simulate_one_b(Game& gg){
		if(gg.status == Game::P2_WIN){win_flag = true; return;}
		// std::cout << "-b-\n";

		std::shuffle(random_move_vec.begin(), random_move_vec.end(), engine);
		int b = random_move_vec.size()-1;
		for(int i=0;i<random_move_vec.size();i++){
			if(gg.play(random_move_vec[i]) != Game::INVALID_MOVE){
				// std::cout << gg;
				if(gg.status == Game::P2_WIN){return ;}
				for(;b>=0;b--){
					if(gg.play(random_move_vec[b]) != Game::INVALID_MOVE){
						// std::cout << gg;
						if(gg.status == Game::P1_WIN){ win_flag = true; return;}
						break;
					}
				}
			}
		}
		return;
	}
	void simulate_one_w(Game& gg){
		if(gg.status == Game::P2_WIN) return;

		// std::cout << "-w-\n";

		std::shuffle(random_move_vec.begin(), random_move_vec.end(), engine);
		int b = random_move_vec.size()-1;
		for(int i=0;i<random_move_vec.size();i++){
			if(gg.play(random_move_vec[i]) != Game::INVALID_MOVE){
				// std::cout << gg;
				if(gg.status == Game::P1_WIN){win_flag = true;  return;}
				
				for(;b>=0;b--){
					if(gg.play(random_move_vec[b]) != Game::INVALID_MOVE){
						// std::cout << gg;
						if(gg.status == Game::P2_WIN){return ;}

						break;
					}
				}
			}
		}
		return;
	}


	void outputnode(const node* n){
		std::cout << " move : " << n->move;
		std::cout << " total : " << n->totalmove_count;
		std::cout << " win : " << n->win_count << "\n";
	}

    void info(){std::cout << mcts_g; return;}
    void table(){
        for(auto s : random_move_vec){
            std::cout << s << ",";
        }
        std::cout << "\n";
        return;
    }
protected:
	std::default_random_engine engine;
    
private:
    Game mcts_g;
	bool sss = false;

    std::vector<int> random_move_vec;
	std::vector<node*> update_node_vector;
	std::vector<node*> release_node_vector;
	bool win_flag;
	double counter;
	double c_;

};