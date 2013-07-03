#include "Simulator.hpp"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <ctime>

using namespace std;

string tostring(int n)
{
	stringstream ss;
	ss << n;
	return ss.str();
}

int main(int argc, char *argv[])
{
  Logger::Initialize();
	string root = "../resource/";
	//string root = "resource/";
	string input_path = root + "input/" + argv[1];
	string output_path = root + "output/" + argv[1];
	string result_path = root + "result/" + argv[1];

	ifstream ifs(input_path);
	int wid,hei,size,sum,step;
	ifs >> wid >> hei >> size >> sum >> step;

  vector<vector<vector<int> > > packs(step, vector<vector<int> > (size, vector<int>(size, 0)));

	for(int turn=0; turn<step; ++turn){
		for(int y=0; y<size; ++y){
			for(int x=0; x<size; ++x){
				int num;
				ifs >> num;
        packs[turn][y][x] = num;
			}
		}
		string END;
		ifs >> END;
	}

  Simulator<10, 16+4, 4, 10, 1000, 25, 100> simulator(packs); // Small
  //Simulator<15, 23+4, 4, 20, 1000, 30, 1000> simulator(packs); // Medium
  //Simulator<20, 36+5, 5, 30, 1000, 35, 10000> simulator(packs); // Large

	ifs.open(output_path,ios::in);
	int x,pos;
	vector<pair<int,int> > ans;
	while(ifs >> x >> pos)
	{
		ans.push_back(make_pair(x,pos));
	}
	ifs.close();

	int max_turn = min(step,(int)ans.size());

	clock_t before_t = clock();
	for(int turn=0; turn<max_turn; ++turn)
	{
		if(!simulator.step(ans[turn].first,ans[turn].second)){
      break;
    }

    if(simulator.step(ans[turn].first,ans[turn].second))
      simulator.restep();

    if(simulator.step(0,0)){
      if(simulator.step(2,0)){
        if(simulator.step(1,0)){
          simulator.restep();
        }
        simulator.restep();
      }
      simulator.restep();
    }
	}
	clock_t delta_t = clock() - before_t;
	ifs.open(result_path,ios::in);
	int expected_chain;
	long long expected_score;
	ifs >> expected_score >> expected_chain;
	ifs.close();

  long long score = simulator.score;
	if(score == expected_score)
	{
		cout << "[AC]:" << argv[1] << " " << delta_t << "[ms]" << endl;
	}
	else
	{
		cout << "[WA]:" << argv[1] << endl;
		cout << ">>Expected:" << expected_chain << "," << expected_score << endl;
		cout << endl;
	}
	return 0;
}
