#pragma once
#include<fstream>
#include<string>
using namespace std;

class Logger
{
public:
  static bool enable_stdio;
  static ofstream ofs;
  static void Initialize();
};

#ifndef SOLVER_MODE
#define DEBUG(a)  cout << "[DEBUG]" << #a << " = " << a << endl;
#define ERROR(a)  cout << "[ERROR]" << a << "(F:" << __FILE__ << " L:" << __LINE__ << ")" <<  endl;
#define LOG(a)    cout << "[ LOG ]" << a << endl;
#define DOUT      cout
#else
#define DEBUG(a)  Logger::ofs << "[DEBUG]" << #a << " = " << a << endl;
#define ERROR(a)  Logger::ofs << "[ERROR]" << a << "(F:" << __FILE__ << " L:" << __LINE__ << ")" <<  endl;
#define LOG(a)    Logger::ofs << "[ LOG ]" << a << endl;
#define DOUT      Logger::ofs
#endif

