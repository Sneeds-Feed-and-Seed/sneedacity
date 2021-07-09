/*!
  @file InconsistencyException.cpp
  @brief Implements InconsistencyException
  

  Created by Paul Licameli on 11/27/16.

*/


#include "InconsistencyException.h"

InconsistencyException::~InconsistencyException()
{
}

TranslatableString InconsistencyException::ErrorMessage() const
{
   // Shorten the path
   wxString path { file };
   auto sub = wxString{ wxFILE_SEP_PATH } + "src" + wxFILE_SEP_PATH;
   auto index = path.Find(sub);
   if (index != wxNOT_FOUND)
      path = path.Mid(index + sub.size());

#ifdef __func__
   return
XO("Internal error in %s at %s line %d.\nPlease inform the Sneedacity team at https://github.com/Sneeds-Feed-and-Seed/sneedacity/issues.")
      .Format( func, path, line );
#else
   return
XO("Internal error at %s line %d.\nPlease inform the Sneedacity team at https://github.com/Sneeds-Feed-and-Seed/sneedacity/issues.")
      .Format( path, line );
#endif
}
