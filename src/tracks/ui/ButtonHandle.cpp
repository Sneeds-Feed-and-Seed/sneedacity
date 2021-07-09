/**********************************************************************

Sneedacity: A Digital Audio Editor

ButtonHandle.cpp

Paul Licameli

**********************************************************************/


#include "ButtonHandle.h"

#include "../../HitTestResult.h"
#include "../../RefreshCode.h"
#include "../../Track.h"
#include "../../TrackPanelMouseEvent.h"

ButtonHandle::ButtonHandle
( const std::shared_ptr<Track> &pTrack, const wxRect &rect )
   : mpTrack{ pTrack }
   , mRect{ rect }
{
}

ButtonHandle::~ButtonHandle()
{
}

void ButtonHandle::Enter(bool, SneedacityProject *)
{
   mChangeHighlight = RefreshCode::RefreshCell;
}

UIHandle::Result ButtonHandle::Click
(const TrackPanelMouseEvent &evt, SneedacityProject *pProject)
{
   using namespace RefreshCode;
   auto pTrack = TrackList::Get( *pProject ).Lock(mpTrack);
   if ( !pTrack )
      return Cancelled;

   const wxMouseEvent &event = evt.event;
   if (!event.Button(wxMOUSE_BTN_LEFT))
      return Cancelled;

   // Come here for left click or double click
   if (mRect.Contains(event.m_x, event.m_y)) {
      mWasIn = true;
      mIsClicked = true;
      // Toggle visible button state
      return RefreshCell;
   }
   else
      return Cancelled;
}

UIHandle::Result ButtonHandle::Drag
(const TrackPanelMouseEvent &evt, SneedacityProject *pProject)
{
   const wxMouseEvent &event = evt.event;
   using namespace RefreshCode;
   auto pTrack = TrackList::Get( *pProject ).Lock(mpTrack);
   if (!pTrack)
      return Cancelled;

   auto isIn = mRect.Contains(event.m_x, event.m_y);
   auto result = (isIn == mWasIn) ? RefreshNone : RefreshCell;
   mWasIn = isIn;
   return result;
}

HitTestPreview ButtonHandle::Preview
(const TrackPanelMouseState &st, SneedacityProject *project)
{
   // No special cursor
   TranslatableString message;
   if (project)
      message = Tip(st.state, *project);
   return { message, {}, message };
}

UIHandle::Result ButtonHandle::Release
(const TrackPanelMouseEvent &evt, SneedacityProject *pProject,
 wxWindow *pParent)
{
   using namespace RefreshCode;
   auto pTrack = TrackList::Get( *pProject ).Lock(mpTrack);
   if (!pTrack)
      return Cancelled;

   Result result = RefreshNone;
   const wxMouseEvent &event = evt.event;
   if (pTrack && mRect.Contains(event.m_x, event.m_y))
      result |= RefreshCell | CommitChanges(event, pProject, pParent);
   return result;
}

UIHandle::Result ButtonHandle::Cancel(SneedacityProject *WXUNUSED(pProject))
{
   using namespace RefreshCode;
   return RefreshCell; // perhaps unnecessarily if pointer is out of the box
}
