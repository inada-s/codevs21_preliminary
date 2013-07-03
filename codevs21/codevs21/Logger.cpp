#include"Logger.h"
#include<iostream>

void Logger::Initialize()
{
  ofs = ofstream("logger.txt");
  if(ofs.fail()){
    std::cerr << "Cannot open logger.txt" << std::endl;
  }
}
ofstream Logger::ofs;
