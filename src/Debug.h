#ifndef __SNEEDACITY_DEBUGH__
#define __SNEEDACITY_DEBUGH__

#include <string>
#include <iostream>

using namespace std;

inline bool global_debug_prints_enabled;

template <class... Ts>
void dprintf(Ts&&... ts)
{
   if (global_debug_prints_enabled)
   {
      ((std::cout << std::forward<Ts>(ts)), ...) << '\n';
   }
}

#endif
