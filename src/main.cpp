#include <iostream>
#include <fstream>
#include <sstream>
#include "slicer.h"
#include "relations_builder.h"
#include "clang/Tooling/Tooling.h"

int main(int argc, char *argv[]){
  if(argc <= 5){
      std::cout
          << "usage: ./slicer source-file function column row"
          << std::endl;
      return 1;
    }

  std::ifstream file(argv[1]);
  std::vector<std::string> params{argv[2],argv[3],argv[4]};
  if(file.is_open()){
      std::stringstream buffer;
      buffer << file.rdbuf();
      clang::tooling::runToolOnCode(
            new RelationsBuilderAction(params),
            buffer.str());
    }
  return 0;
}
