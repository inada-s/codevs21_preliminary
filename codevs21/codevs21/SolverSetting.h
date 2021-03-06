#pragma once
#include<string>
using namespace std;

class SolverSetting{
public:
  int CHARGE_END_FIRE_COUNT;
  int BOMB_END_ERASE_COUNT;

  int WIDTH_CHARGE_SEARCH;
  int DEPTH_CHARGE_SEARCH;

  long long RATIO_CHARGE_BLOCK_COUNT;
  long long RATIO_CHARGE_STEP_DELTA_FIRE_COUNT;
  long long RATIO_CHARGE_FINISH_BLOCK_COUNT;
  long long RATIO_CHARGE_FINISH_DELTA_FIRE_COUNT;

  int WIDTH_BOMB_SEARCH;
  int DEPTH_BOMB_SEARCH;

  long long RATIO_BOMB_BLOCK_COUNT;
  long long RATIO_BOMB_CHECKSTEP_BLOCK_COUNT;
  long long RATIO_BOMB_CHECKSTEP_MAX_ERASE_COUNT;
  long long RATIO_BOMB_CHECKSTEP_MAX_LAST_ERASE_COUNT;
  long long RATIO_BOMB_CHECKSTEP_MAX_RAW_SCORE;

  long long RATIO_BOMB_FINISH_BLOCK_COUNT;
  long long RATIO_BOMB_FINISH_ERASE_COUNT;
  long long RATIO_BOMB_FINISH_LAST_ERASE_COUNT;
  long long RATIO_BOMB_FINISH_RAW_SCORE;

  int WIDTH_CHAIN_SEARCH;
  int DEPTH_CHAIN_SEARCH;

  int CHAIN_RETRY_COUNT;

  SolverSetting();
  void loadSetting(string file);
  void debugPrint();
};