#include <iostream>
#include <fstream>
#include <sstream>
#include "pdg.h"
#include "astwalker.h"
#include "clang/Tooling/Tooling.h"

int main(int argc, char *argv[])
{
  if(argc <= 2)
    {
      std::cout << "usage: ./slicer file fun" << std::endl;
      return 1;
    }
  else
    {
      std::ifstream file(argv[1]);
      if(file.is_open())
        {
          std::stringstream buffer;
          buffer << file.rdbuf();
         clang::tooling::runToolOnCode(new ASTWalkerAction,buffer.str());

        }
    }

  return 0;
}
