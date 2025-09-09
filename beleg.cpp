#include "includes/common.h"
#include <iostream>

#define PROJECT_NAME "beleg"

int main(int argc, char **argv) {
  if (argc != 1) {
    std::cout << argv[0] << " takes no arguments.\n";
    return 1;
  }
  String project_name = PROJECT_NAME;
  std::cout << "This is project " << project_name << ".\n";
  return 0;
}
