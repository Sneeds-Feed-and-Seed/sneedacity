//Copyright 2021 Sneedacity Project

#include <string>
#include <iostream>
#include "Debug.h"

template <typename T, typename... Types>
void dprintf(T var1, Types... var2)
{
   if(global_debug_prints_enabled) {
    std::cout << var1;
    dprintf(var2...) ;
  }
}

