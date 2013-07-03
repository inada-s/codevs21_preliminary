#pragma once
#include <vector>
#include "../codevs21/Logger.h"
using namespace std;

template<int W, int H, int T, int S, int N, int P, int Th>
struct Simulator{
  int turn;
  int erase_count;
  int wall_erase_count;
  int last_erase_count;
  int max_erase_count;
  int Fc;
  int chain;
  long long raw_score;
  long long score;
  char field[H][W];
  char packs[N][T][T];
  char field_log[N][H][W];
  int  fc_log[N];
  long long score_log[N];

  Simulator( vector<vector<vector<int> > > _packs )
    : turn(0), erase_count(0), wall_erase_count(0),last_erase_count(0), max_erase_count(0), raw_score(0), Fc(0), chain(0), score(0) {
      for(int i=0; i<N; ++i)
        for(int y=0; y<T; ++y)
          for(int x=0; x<T; ++x)
            packs[i][y][x] = _packs[i][y][x];
      memset(field, 0, sizeof(field));
  }


  bool check (const int x, const int y) const{
    return (x>=0 && x<W && y>=0 && y<H);
  }

  bool next(const int pos, const char pack[T][T]){
    fc_log[turn] = Fc;
    score_log[turn] = score;
    memcpy(field_log[turn], field, sizeof(field));

    for(int y=0; y<T; ++y){
      for(int x=0; x<T; ++x){
        if(pack[y][x]){
          field[H-1-y][pos+x] = pack[y][x];
        }
      }
    }

    chain = 0;
    max_erase_count = 0;
    last_erase_count = 0;
    raw_score = 0;

    for(bool once = true;;){
      erase_count = 0;
      wall_erase_count = 0;
      bool erase_mark[H][W] = {0};

      int ux = once ? W-1 : 0;
      int lx = once ? 0   : W-1;
      int uy = once ? H-1 : 0;
      int ly = once ? H-T : H-1;
      once = false;

      for(int x=0; x<W; ++x){
        int e = 0;
        for(int y=0; y<H; ++y){
          if(!field[y][x])e++;
          else if(e){
            field[y-e][x] = field[y][x];
            field[y][x] = 0;
            ux = max(ux, x);
            uy = max(uy, y-e);
            lx = min(lx, x);
            ly = min(ly, y-e);
          }
        }
      }

      const int Ux = ux;
      const int Uy = uy;
      const int Lx = lx;
      const int Ly = ly;

      for(int x=Lx; x<=Ux; ++x){
        int l = 0;
        int s = 0;
        for(int y=0; y<H; ++y){
          if(!field[y][x])break;
          s += field[y][x];
          while(s>S){
            s -= field[l++][x];
          }
          if(s==S)for(int i=l; i<=y; ++i){
            erase_mark[i][x] = true;
            erase_count++;
          }
        }
      }

      for(int y=Ly; y<=Uy; ++y){
        int l = 0;
        int s = 0;
        for(int x=0; x<W; ++x){
          if(!field[y][x]){
            l = x+1;
            s = 0;
            continue;
          }
          s += field[y][x];
          while(s>S){
            s -= field[y][l++];
          }
          if(s==S)for(int i=l; i<=x; ++i){
            erase_mark[y][i] = true;
            erase_count++;
          }
        }
      }

      for(int k=H-1-Uy+Lx; k<=H-1-Ly+Ux; ++k){
        int rx = max(0, k-H+1);
        int ry = max(0, H-1-k);
        int lx = rx;
        int ly = ry;
        int s = 0;
        for(;rx<W && ry<H; rx++, ry++){
          if(!field[ry][rx]){
            lx = rx + 1;
            ly = ry + 1;
            s  = 0;
            continue;
          }
          s += field[ry][rx];
          while(s>S){
            s -= field[ly++][lx++];
          }
          if(s==S)for(int i=0; i<=rx-lx; ++i){
            erase_mark[ly+i][lx+i] = true;
            erase_count++;
          }
        }
      }

      for(int k=Ly+Lx; k<=Uy+Ux; ++k){
        int rx = max(0, k-H+1);
        int ry = min(k, H-1);
        int lx = rx;
        int ly = ry;
        int s = 0;
        for(;rx<W && ry>=0; rx++, ry--){
          if(!field[ry][rx]){
            lx = rx + 1;
            ly = ry - 1;
            s  = 0;
            continue;
          }
          s += field[ry][rx];
          while(s>S){
            s -= field[ly--][lx++];
          }
          if(s==S)for(int i=0; i<=rx-lx; ++i){
            erase_mark[ly-i][lx+i] = true;
            erase_count++;
          }
        }
      }

      if(!erase_count)break;

      static const int dx[] = {1,1,1,0,-1,-1,-1,0};
      static const int dy[] = {1,0,-1,-1,-1,0,1,1};
      for(int y=0; y<H; ++y){
        for(int x=0; x<W; ++x){
          if(erase_mark[y][x]){
            field[y][x] = 0;
            for(int i=0; i<8; ++i){
              const int xi = x+dx[i];
              const int yi = y+dy[i];
              if(check(xi, yi) && (field[yi][xi] > S)){
                wall_erase_count++;
                field[yi][xi] = 0;
              }
            }
          }
        }
      }

      chain++;
      last_erase_count = erase_count + wall_erase_count;
      max_erase_count  = max(max_erase_count, last_erase_count);
      raw_score += (1LL<<min(last_erase_count/3, P)) * max(1, last_erase_count/3 - P + 1) * chain;
    }

    for(int x=0; x<W; ++x){
      if(field[H-T][x]){
        memcpy(field, field_log[turn], sizeof(field));
        return false;
      }
    }

    score += raw_score * (Fc+1);
    if(raw_score >= Th)Fc++;
    turn++;
    return true;
  }

  bool step(const int pos, const int rot){
    if(rot >= 4 || rot < 0 || turn == N)return false;
    int pack_left  = T;
    int pack_right = 0;
    char pack[T][T] = {0};
    for(int y=0; y<T; ++y){
      for(int x=0; x<T; ++x){
        const int n = packs[turn][y][x];
        if(n){
          int ax;
          if(rot==0){
            pack[y][x] = n;
            ax = x;
          }
          else if(rot==1){
            pack[x][T-1-y] = n;
            ax = T-1-y;
          }
          else if(rot==2){
            pack[T-1-y][T-1-x] = n;
            ax = T-1-x;
          }
          else if(rot==3){
            pack[T-1-x][y] = n;
            ax = y;
          }
          pack_left = min(pack_left,  ax);
          pack_right= max(pack_right, ax);
        }
      }
    }
    if(!(pos + pack_left >= 0 && pos + pack_right < W))return false;
    return next(pos, pack);
  }

  bool checkstep(const int pos, const int num){
    if(!(pos >= 0 && pos < W))return false;
    char pack[T][T] = {0};
    pack[0][0] = num;
    return next(pos, pack);
  }

  void restep(){
    turn--;
    Fc = fc_log[turn];
    score = score_log[turn];
    memcpy(field, field_log[turn], sizeof(field));
  }

  int blockcount(){
    int ret = 0;
    for(int x=0; x<W; ++x){
      for(int y=0;y<H; ++y){
        if(field[y][x])ret++;
        else break;
      }
    }
    return ret;
  }

  unsigned int hash(){
    unsigned int ret=2166136261UL;
    for(int x=0; x<W; ++x)
    {
      for(int y=0;y<H; ++y)
      {
        if(field[y][x]){
          ret *= 16777619UL;
          ret ^= x;
          ret *= 16777619UL;
          ret ^= y;
          ret *= 16777619UL;
          ret ^= field[y][x];
        }
      }
    }
    return ret;
  }

  void dump(){
    for(int y = H-1; y>=0; y--){
      for(int x = 0; x < W; x++){
        DOUT << (int)field[y][x] << (field[y][x] >= 10 ? " " : "  ");
      }
      DOUT << endl;
    }
    DOUT << endl;
  }
};
