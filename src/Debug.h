#ifndef __SNEEDACITY_DEBUGH__
#define __SNEEDACITY_DEBUGH__

#include <string>

using namespace std;

inline bool global_debug_prints_enabled;
template <typename T, typename... Types>
void dprintf(T var1, Types... var2);

#endif
