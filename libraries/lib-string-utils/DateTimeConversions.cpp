/*!********************************************************************

 Sneedacity: A Digital Audio Editor

 @file DateTimeConversions.cpp
 @brief Define functions to work with date and time string representations.

 Dmitry Vedenko
 **********************************************************************/

#include "DateTimeConversions.h"

#include <wx/datetime.h>

#include "CodeConversions.h"

namespace sneedacity
{

bool ParseRFC822Date (const std::string& dateString, SystemTime* time)
{
    wxDateTime dt;
    wxString::const_iterator end;

    if (!dt.ParseRfc822Date (dateString, &end))
        return false;

    if (time != nullptr)
        *time = std::chrono::system_clock::from_time_t (dt.GetTicks ());

    return true;
}

std::string SerializeRFC822Date (SystemTime timePoint)
{
    const wxDateTime dt (
            time_t (std::chrono::duration_cast<std::chrono::seconds> (
                    timePoint.time_since_epoch ()
            ).count ()));

    return ToUTF8 (dt.Format("%a, %d %b %Y %H:%M:%S %z"));
}

}
