#ifndef __SNEEDACITY_DEBUGH__
#define __SNEEDACITY_DEBUGH__

#include <string>
#include <iostream>
#include <chrono>

inline bool global_debug_prints_enabled;

template <class... Ts>
void dprintf(Ts&&... ts)
{
   if (global_debug_prints_enabled)
   {
      int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      std::cout << now << " ";
      ((std::cout << std::forward<Ts>(ts)), ...);
      std::cout << std::endl;
   }
}

#endif
