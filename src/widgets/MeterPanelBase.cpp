/**********************************************************************

Sneedacity: A Digital Audio Editor

MeterPanelBase.cpp

Paul Licameli split from Meter.cpp

**********************************************************************/

#include "MeterPanelBase.h"

MeterPanelBase::~MeterPanelBase()
{
}

bool MeterPanelBase::s_AcceptsFocus{ false };

auto MeterPanelBase::TemporarilyAllowFocus() -> TempAllowFocus {
   s_AcceptsFocus = true;
   return TempAllowFocus{ &s_AcceptsFocus };
}
