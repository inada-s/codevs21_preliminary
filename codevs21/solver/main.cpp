#include "../codevs21/Solver.h"
#include "../codevs21/Logger.h"
#include "../codevs21/Simulator.hpp"
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
using namespace std;

int main(){
  Logger::Initialize();
	int wid,hei,size,sum,step;
	cin >> wid >> hei >> size >> sum >> step;
  vector<vector<vector<int> > > packs(step, vector<vector<int> > (size, vector<int>(size, 0)));

	for(int turn=0; turn<step; ++turn){
		for(int y=0; y<size; ++y){
			for(int x=0; x<size; ++x){
				int num;
				cin >> num;
        packs[turn][y][x] = num;
			}
		}
		string END;
		cin >> END;
	}

	time_t t = time(NULL);
	stringstream ss;
	ss << t;
	ofstream ofs("codevs21/resource/input/" + ss.str() + ".txt");
	ofs << wid << " " << hei << " " << size << " " << sum << " " << step << endl;
	for(int i=0;i<packs.size();++i){
		for(int y=0;y<size;++y){
			for(int x=0;x<size;++x){
        ofs << packs[i][y][x];
			}
			ofs << endl;
		}
		ofs << "END" << endl;
	}
	ofs.close();

  vector<pair<int,int> > ans;
	if(wid == 10){
    ans = Solver<10, 16+4, 4, 10, 1000, 25, 100>(packs).run();
  }
	if(wid == 15){
    ans = Solver<15, 23+4, 4, 20, 1000, 30, 1000>(packs).run();
  }
	if(wid == 20){
    ans = Solver<20, 36+5, 5, 30, 1000, 35, 10000>(packs).run();
  }

  ofs.open("codevs21/resource/output/" + ss.str() + ".txt");
	for(int i=0;i<ans.size();++i){
		cout << ans[i].first << " " << ans[i].second << endl;
		ofs  << ans[i].first << " " << ans[i].second << endl;
  }

  cout << "999 999" << endl;
}
