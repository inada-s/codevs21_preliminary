#pragma once
#include "../codevs21/Simulator.hpp"
#include "SolverSetting.h"
#include <vector>
#include <map>
using namespace std;

template<int W, int H, int T, int S, int N, int P, int Th>
class Solver{
public:
  SolverSetting setting;
  Simulator<W,H,T,S,N,P,Th> simulator;

  int fixed_turn;
  bool fire_charged;
  bool bomb_created;
  bool chain_created;

  bool fire_test;
  bool bomb_test;
  bool chain_test;


  Solver(vector<vector<vector<int> > > packs);

public:
  vector<pair<int,int> > run();
private:
  vector<pair<int,int> > charge(int max_depth);
  long long calc_charge_score();
  vector<vector<pair<int,int> > > charge_score_filter(const vector<vector<pair<int,int> > >& nodes, int n);
  vector<vector<pair<int,int> > > rec_charge(const vector<vector<pair<int,int> > >& nodes, int depth);

public:
  vector<pair<int,int> > bomb(int max_depth);
private:
  long long calc_bomb_score();
  vector<vector<pair<int,int> > > bomb_score_filter(const vector<vector<pair<int,int> > >& nodes, int n);
  vector<vector<pair<int,int> > > rec_bomb(const vector<vector<pair<int,int> > >& nodes, int depth);


public:
  vector<pair<int,int> > chain(int max_depth);
private:
  long long calc_chain_score();
  vector<vector<pair<int,int> > > chain_score_filter(const vector<vector<pair<int,int> > >& nodes, int n);
  vector<vector<pair<int,int> > > rec_chain(const vector<vector<pair<int,int> > >& nodes, int depth);
};
