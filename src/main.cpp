#include <iostream>
#include <fstream>
#include <sstream>
#include "slicer.h"
#include "relations_builder.h"
#include "clang/Tooling/Tooling.h"

int main(int argc, char *argv[]){
  if(argc < 5){
      std::cout
          << "usage: ./slicer source-file function row column"
          << std::endl;
      return 1;
    }
  std::string filename(argv[1]);
  std::ifstream file(filename);
  std::vector<std::string> params{argv[2],argv[3],argv[4]};
  if (!file.is_open()) {
	  std::cout
		  << "could not open file: " << filename << std::endl;
	  return 1;
  }else
  {
      std::stringstream buffer;
	  buffer << file.rdbuf();
	  std::cout << buffer.str() << std::endl;
      clang::tooling::runToolOnCode(
            new RelationsBuilderAction(params),
            buffer.str());
  }
  file.close();
  return 0;
}
