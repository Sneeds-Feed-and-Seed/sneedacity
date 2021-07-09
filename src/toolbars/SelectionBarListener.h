/**********************************************************************

  Sneedacity: A Digital Audio Editor

  SelectionBarListener.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __SNEEDACITY_SELECTION_BAR_LISTENER__
#define __SNEEDACITY_SELECTION_BAR_LISTENER__

#include "sneedacity/Types.h"

class SelectedRegion;

class SNEEDACITY_DLL_API SelectionBarListener /* not final */ {

 public:

   SelectionBarListener(){};
   virtual ~SelectionBarListener(){};

   virtual double AS_GetRate() = 0;
   virtual void AS_SetRate(double rate) = 0;
   virtual int AS_GetSnapTo() = 0;
   virtual void AS_SetSnapTo(int snap) = 0;
   virtual const NumericFormatSymbol & AS_GetSelectionFormat() = 0;
   virtual void AS_SetSelectionFormat(const NumericFormatSymbol & format) = 0;
   virtual void AS_ModifySelection(double &start, double &end, bool done) = 0;
};

class SNEEDACITY_DLL_API TimeToolBarListener /* not final */ {

 public:

   TimeToolBarListener(){};
   virtual ~TimeToolBarListener(){};

   virtual const NumericFormatSymbol & TT_GetAudioTimeFormat() = 0;
   virtual void TT_SetAudioTimeFormat(const NumericFormatSymbol & format) = 0;
};

#endif
