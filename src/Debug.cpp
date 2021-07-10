//Copyright 2021 Sneedacity Project

#include <string>
#include <iostream>
#include "Debug.h"

using namespace std;

void dprintf(const string message) {
   if(global_debug_prints_enabled) {
      std::cout << message << "\n";
   }
}


