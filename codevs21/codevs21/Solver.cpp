#include "Solver.h"
#include "Logger.h"
#include <set>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <queue>
#include <iterator>

#define TURN_CHECK if(fixed_turn != simulator.turn){ ERROR("Turn Error."); }
using namespace std;


template<int W, int H, int T, int S, int N, int P, int Th>
Solver<W,H,T,S,N,P,Th>::Solver(vector<vector<vector<int> > > packs)
  : simulator(packs){
  fire_charged = false;
  chain_created = false;
  bomb_created = false;

  fire_test = false;
  chain_test= false;
  bomb_test = false;
}

// ================================================================================
// チャージAI Fcの値を出来るだけ短いターンで増やす
// ================================================================================
//スコアリング
template<int W, int H, int T, int S, int N, int P, int Th>
long long Solver<W,H,T,S,N,P,Th>::calc_charge_score(){
  long long ret = 0;
  if(setting.RATIO_CHARGE_BLOCK_COUNT){
    ret += simulator.blockcount() * setting.RATIO_CHARGE_BLOCK_COUNT;
  }
  const int before_fire_count = simulator.Fc;
  for(int x=1-T; x<W; ++x){
    for(int r=0; r<4; ++r){
      if(simulator.step(x,r)){
        ret += (simulator.Fc - before_fire_count) * setting.RATIO_CHARGE_STEP_DELTA_FIRE_COUNT;
        ret += simulator.raw_score;
        simulator.restep();
      }
    }
  }
  return ret;
}

//スコアリングによって上位n個をフィルタリング
template<int W, int H, int T, int S, int N, int P, int Th>
vector<vector<pair<int,int> > > Solver<W,H,T,S,N,P,Th>::charge_score_filter(const vector<vector<pair<int,int> > >& nodes, int n){
  if(nodes.size() < n){
    return nodes;
  }

  TURN_CHECK;
  vector<pair<long long,int> > score_nodes(nodes.size());
  for(int i=0; i<nodes.size(); ++i){
    for(int j=0; j<nodes[i].size(); ++j){
      simulator.step(nodes[i][j].first, nodes[i][j].second);
    }

    score_nodes[i].first  = calc_charge_score();
    score_nodes[i].second = i;

    for(int j=0; j<nodes[i].size(); ++j){
      simulator.restep();
    }
  }
  TURN_CHECK;

  sort(score_nodes.rbegin(), score_nodes.rend());

  DEBUG(score_nodes[0].first);

  int N = min((int)score_nodes.size(), setting.WIDTH_CHARGE_SEARCH);
  vector<vector<pair<int,int> > > ret(N);
  for(int i=0; i<N; ++i){
    ret[i] = nodes[ score_nodes[i].second ];
  }

  return ret;
}

//１手先を探索
template<int W, int H, int T, int S, int N, int P, int Th>
vector<vector<pair<int,int> > > Solver<W,H,T,S,N,P,Th>::rec_charge(const vector<vector<pair<int,int> > >& before_nodes, int depth){
  DEBUG(depth);
  DEBUG(before_nodes.size());
  if(depth <= 0){
    return before_nodes;
  }

  vector<vector<pair<int,int> > > nodes = charge_score_filter(before_nodes, setting.WIDTH_CHARGE_SEARCH);
  vector<vector<pair<int,int> > > next_nodes;
  next_nodes.reserve(50000);

  const int before_fc = simulator.Fc;
  bool found = false;
  vector<vector<pair<int,int> > > found_nodes;

  int cut = 0;
  TURN_CHECK;
  for(int i=0; i<nodes.size(); ++i){
    for(int j=0; j<nodes[i].size(); ++j){
      simulator.step(nodes[i][j].first, nodes[i][j].second);
    }

    const int before_bc = simulator.blockcount();

    for(int x=1-T; x<W; ++x){
      for(int r=0; r<4; ++r){
        if(simulator.step(x,r)){
          const int after_fc = simulator.Fc;
          vector<pair<int,int> > node(nodes[i]);
          node.push_back(make_pair(x,r));
          if(before_fc < after_fc){
            found = true;
            found_nodes.push_back(node);
          }else if(before_bc < simulator.blockcount()){
            next_nodes.push_back(node);
          }else{
            cut++;
          }
          simulator.restep();
        }
      }
    }

    for(int j=0; j<nodes[i].size(); ++j){
      simulator.restep();
    }
  }
  TURN_CHECK;

  DEBUG(cut);
  if(found) return found_nodes;
  else      return rec_charge(next_nodes, depth-1);
}

template<int W, int H, int T, int S, int N, int P, int Th>
vector<pair<int,int> > Solver<W,H,T,S,N,P,Th>::charge(int max_depth){
  //1手先を突っ込む
  vector<vector<pair<int,int> > > nodes;
  for(int x=1-T; x<W; ++x){
    for(int r=0; r<4; ++r){
      if(simulator.step(x,r)){
        vector<pair<int,int> > node;
        node.push_back(make_pair(x,r));
        nodes.push_back(node);
        simulator.restep();
      }
    }
  }

  nodes = rec_charge(nodes, max_depth-1);

  int same_score_count = 0;
  //そこまでで一番良い感じの物を爆発させる
  vector<pair<int,int> > ans;
  long long ans_score = 0;
  const int before_fc = simulator.Fc;
  for(int i=0; i<nodes.size(); ++i){
    const int N = (int)nodes[i].size()-1;
    for(int j=0; j<N; ++j){
      simulator.step(nodes[i][j].first,nodes[i][j].second);
      for(int x=1-T; x<W; ++x)for(int r=0; r<4; ++r){
        if(simulator.step(x,r)){
          long long score = 0;
          const int after_fc = simulator.Fc;
          score += simulator.blockcount() * setting.RATIO_CHARGE_FINISH_BLOCK_COUNT;
          score += (after_fc - before_fc) * setting.RATIO_CHARGE_FINISH_DELTA_FIRE_COUNT;
          simulator.restep();
          if(ans_score < score){
            same_score_count = 0;
            ans_score = score;
            ans.clear();
            for(int k=0; k<=j; ++k){
              ans.push_back(nodes[i][k]);
            }
            ans.push_back(make_pair(x,r));
          }
          else if(ans_score == score){
            same_score_count++;
          }
        }
      }
    }
    for(int j=0; j<N; ++j){
      simulator.restep();
    }
  }
  DEBUG(same_score_count);

  if(ans.empty()){
    ans.push_back(make_pair(-100,0));
  }
  return ans;
}






// ================================================================================
// bombAI 不定形で爆弾を作り爆破する
// ================================================================================
template<int W, int H, int T, int S, int N, int P, int Th>
long long Solver<W,H,T,S,N,P,Th>::calc_bomb_score(){
  long long ret = 0;

  if(setting.RATIO_BOMB_BLOCK_COUNT){
    ret += simulator.blockcount() * setting.RATIO_BOMB_BLOCK_COUNT;
  }

  int max_erase_count       = 0;
  int max_last_erase_count  = 0;

  long long max_raw_score   = 0;

  for(int n=1; n<=S/2; ++n){
    if(simulator.checkstep(W/2-1,n)){
      max_erase_count       = max(max_erase_count,      simulator.max_erase_count);
      max_last_erase_count  = max(max_last_erase_count, simulator.last_erase_count);
      max_raw_score         = max(max_raw_score,        simulator.raw_score);
      simulator.restep();
    }
    ret += max_erase_count      * setting.RATIO_BOMB_CHECKSTEP_MAX_ERASE_COUNT;
    ret += max_last_erase_count * setting.RATIO_BOMB_CHECKSTEP_MAX_LAST_ERASE_COUNT;
    ret += max_raw_score        * setting.RATIO_BOMB_CHECKSTEP_MAX_RAW_SCORE;
  }

  return ret;
}

//スコアによって上位n個をフィルタ
template<int W, int H, int T, int S, int N, int P, int Th>
vector<vector<pair<int,int> > > Solver<W,H,T,S,N,P,Th>::bomb_score_filter(const vector<vector<pair<int,int> > >& nodes, int n){
  if(nodes.size() <= n){
    return nodes;
  }

  set<unsigned long> exist;
  TURN_CHECK;
  vector<pair<long long,int> > score_nodes(nodes.size());
  for(int i=0; i<nodes.size(); ++i){
    for(int j=0; j<nodes[i].size(); ++j){
      simulator.step(nodes[i][j].first, nodes[i][j].second);
    }

    score_nodes[i].second = i;

    unsigned int hash = simulator.hash();
    if(exist.find(hash) == exist.end()){
      score_nodes[i].first = calc_bomb_score();
      exist.insert(hash);
    }
    else{
      score_nodes[i].first  = 0;
    }

    for(int j=0; j<nodes[i].size(); ++j){
      simulator.restep();
    }
  }
  TURN_CHECK;

  sort(score_nodes.rbegin(), score_nodes.rend());

  DEBUG(score_nodes[0].first);

  int N = min((int)score_nodes.size(), n);
  vector<vector<pair<int,int> > > ret(N);
  for(int i=0; i<N; ++i){
    ret[i] = nodes[ score_nodes[i].second ];
  }

  return ret;
}

//１手先を探索
template<int W, int H, int T, int S, int N, int P, int Th>
vector<vector<pair<int,int> > > Solver<W,H,T,S,N,P,Th>::rec_bomb(const vector<vector<pair<int,int> > >& before_nodes, int depth){
  DEBUG(depth);
  DEBUG(before_nodes.size());

  if(depth <= 0){
    return before_nodes;
  }
  vector<vector<pair<int,int> > > nodes = bomb_score_filter(before_nodes, setting.WIDTH_BOMB_SEARCH);
  vector<vector<pair<int,int> > > next_nodes;
  next_nodes.reserve(setting.WIDTH_BOMB_SEARCH * (W + T) * 4);

  bool found = false;
  int max_e = 0;

  TURN_CHECK;
  for(int i=0; i<nodes.size(); ++i){
    for(int j=0; j<nodes[i].size(); ++j){
      simulator.step(nodes[i][j].first, nodes[i][j].second);
    }

    for(int n=1; n<=S/2; ++n){
      if(simulator.checkstep(W/2-1,n)){
        max_e = max(max_e, simulator.max_erase_count);
        if(simulator.max_erase_count >= setting.BOMB_END_ERASE_COUNT){
          found = true;
        }
        simulator.restep();
      }
    }

    for(int x=W/2; x<W; ++x)for(int r=0; r<4; ++r){
      if(simulator.step(x,r)){
        simulator.restep();
        vector<pair<int,int> > node(nodes[i]);
        node.push_back(make_pair(x,r));
        next_nodes.push_back(node);
      }
    }

    for(int j=0; j<nodes[i].size(); ++j){
      simulator.restep();
    }
  }
  TURN_CHECK;

  DEBUG(max_e);

  if(found) return nodes;
  return rec_bomb(next_nodes, depth-1);
}

template<int W, int H, int T, int S, int N, int P, int Th>
vector<pair<int,int> > Solver<W,H,T,S,N,P,Th>::bomb(int max_depth){
  //1手先を突っ込む
  vector<vector<pair<int,int> > > nodes;
	for(int x=W/2; x<W; ++x)for(int r=0; r<4; ++r){
    if(simulator.step(x,r)){
      vector<pair<int,int> > node;
      node.push_back(make_pair(x,r));
      nodes.push_back(node);
      simulator.restep();
    }
  }

  nodes = rec_bomb(nodes, max_depth-1);
  nodes = bomb_score_filter(nodes, 1);

  vector<pair<int,int> > ans;
  long long ans_score = 0;
  const int before_fc = simulator.Fc;

  int chain_count = setting.CHAIN_RETRY_COUNT;
  set<int> exist;

  for(int i=0; i<nodes.size(); ++i){
    const int M = nodes[i].size();
    for(int j=0; j<M; ++j){
      simulator.step(nodes[i][j].first,nodes[i][j].second);
    }

    unsigned int hash = simulator.hash();
    if(exist.find(hash) == exist.end() && chain_count){
      exist.insert(hash);
      DEBUG(hash);
      chain_count--;
      int left_turn = N - simulator.turn;
      vector<pair<int,int> > subans = chain(min(setting.DEPTH_CHAIN_SEARCH, left_turn));
      for(int k=0; k<subans.size(); ++k){
        simulator.step(subans[k].first, subans[k].second);
      }
      long long score = simulator.raw_score;
      if(ans_score < score){
        ans_score = score;
        ans = nodes[i];
        for(int k=0; k<subans.size(); ++k){
          ans.push_back(subans[k]);
        }
      }
      for(int k=0; k<subans.size(); ++k){
        simulator.restep();
      }
    }

    for(int j=0; j<M; ++j){
      simulator.restep();
    }
  }

  if(ans.empty()){
    ans.push_back(make_pair(-1,-1));
  }

  return ans;
}



template<int W, int H, int T, int S, int N, int P, int Th>
vector<pair<int,int> > Solver<W,H,T,S,N,P,Th>::chain(int max_depth){
  //1手先を突っ込む
  vector<vector<pair<int,int> > > nodes;
	for(int x=-T; x<W; ++x)for(int r=0; r<4; ++r){
    if(simulator.step(x,r)){
      vector<pair<int,int> > node;
      node.push_back(make_pair(x,r));
      nodes.push_back(node);
      simulator.restep();
    }
  }

  nodes = rec_chain(nodes, max_depth-1);

  vector<pair<int,int> > ans;
  long long ans_score = 0;

  nodes = chain_score_filter(nodes, setting.WIDTH_CHAIN_SEARCH);

  for(int i=0; i<nodes.size(); ++i){
    for(int j=0; j<nodes[i].size(); ++j){
      simulator.step(nodes[i][j].first, nodes[i][j].second);
      if(j<nodes[i].size()*2/3)continue;
      for(int x=-T; x<W; ++x)for(int r=0; r<4; ++r)if(simulator.step(x,r)){
        long long score = simulator.score;
        if(ans_score < score){
          DEBUG(ans_score);
          ans_score = score;
          ans.clear();
          for(int k=0; k<=j; ++k){
            ans.push_back(nodes[i][k]);
          }
          ans.push_back(make_pair(x,r));
        }
        simulator.restep();
      }
    }
    for(int j=0; j<nodes[i].size(); ++j){
      simulator.restep();
    }
  }

  if(ans.empty()){
    ans.push_back(make_pair(-100,-1));
  }
  return ans;
}


template<int W, int H, int T, int S, int N, int P, int Th>
long long Solver<W,H,T,S,N,P,Th>::calc_chain_score(){
  long long ret = 0;
  return ret;
}

template<int W, int H, int T, int S, int N, int P, int Th>
vector<vector<pair<int,int> > > Solver<W,H,T,S,N,P,Th>::chain_score_filter(const vector<vector<pair<int,int> > >& nodes, int n){

  LOG("Filtering..");
  if(nodes.size() < n){
    return nodes;
  }

  set<unsigned int> exist;
  vector<pair<long long,int> > score_nodes(nodes.size());

  for(int i=0; i<nodes.size(); ++i){
    for(int j=0; j<nodes[i].size(); ++j){
      simulator.step(nodes[i][j].first, nodes[i][j].second);
    }

    score_nodes[i].second = i;
    score_nodes[i].first  = 0;

    unsigned int hash = simulator.hash();

    if(exist.find(hash) == exist.end()){
      exist.insert(hash);
      for(int n=1; n<=S/2; ++n){
        for(int x=0; x<3; ++x)if(simulator.checkstep(x,n)){
          score_nodes[i].first = max(score_nodes[i].first,
          simulator.chain * 10000000 + simulator.raw_score);
          simulator.restep();
        }
      }
    }

    for(int j=0; j<nodes[i].size(); ++j){
      simulator.restep();
    }
  }

  sort(score_nodes.rbegin(), score_nodes.rend());

  DEBUG(score_nodes[0].first);

  int N = min((int)score_nodes.size(), n);
  vector<vector<pair<int,int> > > ret(N);
  for(int i=0; i<N; ++i){
    ret[i] = nodes[ score_nodes[i].second ];
  }

  return ret;
}

template<int W, int H, int T, int S, int N, int P, int Th>
vector<vector<pair<int,int> > > Solver<W,H,T,S,N,P,Th>::rec_chain(const vector<vector<pair<int,int> > >& before_nodes, int depth){
  DEBUG(depth);
  DEBUG(before_nodes.size());

  if(depth <= 0){
    return before_nodes;
  }

  int cuted = -1;
  vector<vector<pair<int,int> > > nodes = chain_score_filter(before_nodes, setting.WIDTH_CHAIN_SEARCH);
  vector<vector<pair<int,int> > > next_nodes;
  next_nodes.reserve(100 * (W + T) * 4);

  for(int i=0; i<nodes.size(); ++i){
    for(int j=0; j<nodes[i].size(); ++j){
      simulator.step(nodes[i][j].first, nodes[i][j].second);
    }
    int before_bc = simulator.blockcount();

    for(int x=-T; x<W/2; ++x)for(int r=0; r<4; ++r){
      if(simulator.step(x,r)){
        int after_bc = simulator.blockcount();
        if(cuted == -1 || simulator.raw_score < 100000 && before_bc - after_bc < 30){
          cuted++;
          vector<pair<int,int> > node(nodes[i]);
          node.push_back(make_pair(x,r));
          next_nodes.push_back(node);
        }else{
          cuted ++;
        }
        simulator.restep();
      }
    }

    for(int j=0; j<nodes[i].size(); ++j){
      simulator.restep();
    }
  }
  DEBUG(cuted);
  return rec_chain(next_nodes, depth-1);
}

template<int W, int H, int T, int S, int N, int P, int Th>
vector<pair<int,int> > Solver<W,H,T,S,N,P,Th>::run(){
  setting.debugPrint();
	vector<pair<int,int> > ret;

	while(simulator.turn < N){
    fixed_turn = simulator.turn;
    int left_turn = (N-1) - fixed_turn;

    vector<pair<int, int> > ans;

    if(simulator.Fc >= setting.CHARGE_END_FIRE_COUNT){
      fire_charged = true;
    }

    if(fire_test){
      fire_charged = false;
    }
    if(bomb_test){
      fire_charged = true;
      bomb_created = false;
    }
    if(chain_test){
      fire_charged = true;
      bomb_created = true;
    }


    if(!fire_charged){
      LOG("charge search!");
      ans = charge(min(setting.DEPTH_CHARGE_SEARCH, left_turn));
    }
    else{
      LOG("bomb search!");
      ans = bomb(min(setting.DEPTH_BOMB_SEARCH, left_turn));
    }

    TURN_CHECK;
    for(int i=0; i<ans.size(); ++i){
      if(!simulator.step(ans[i].first, ans[i].second)){
        goto RET;
      }
      DOUT << "Score: " << simulator.score << endl;
      DOUT << "Chain: " << simulator.chain << endl;
      DOUT << "LastErase: " << simulator.last_erase_count << endl;
      ret.push_back(ans[i]);
    }

    DOUT << "==============================" << endl;
    DOUT << "Turn: " << simulator.turn << endl;
    DOUT << "Fc: " << simulator.Fc << endl;
    DOUT << "Chain: " << simulator.chain << endl;
    DOUT << "LastErase: " << simulator.last_erase_count << endl;
    DOUT << "MaxErase: " << simulator.max_erase_count << endl;
    DOUT << "Score: " << simulator.score << endl;
    simulator.dump();
    DOUT << "==============================" << endl;
  }

  RET:
  return ret;
}

template class Solver<10, 16+4, 4, 10, 1000, 25, 100>;
template class Solver<15, 23+4, 4, 20, 1000, 30, 1000>;
template class Solver<20, 36+5, 5, 30, 1000, 35, 10000>;