/**********************************************************************

Sneedacity: A Digital Audio Editor

NoteTrackView.h

Paul Licameli split from class NoteTrack

**********************************************************************/

#ifndef __SNEEDACITY_NOTE_TRACK_VIEW__
#define __SNEEDACITY_NOTE_TRACK_VIEW__

#include "../../../ui/CommonTrackView.h"

class NoteTrackView final : public CommonTrackView
{
   NoteTrackView( const NoteTrackView& ) = delete;
   NoteTrackView &operator=( const NoteTrackView& ) = delete;

public:
   explicit
   NoteTrackView( const std::shared_ptr<Track> &pTrack );
   ~NoteTrackView() override;

   std::shared_ptr<TrackVRulerControls> DoGetVRulerControls() override;

private:
   std::vector<UIHandlePtr> DetailedHitTest
      (const TrackPanelMouseState &state,
       const SneedacityProject *pProject, int currentTool, bool bMultiTool)
      override;

   // TrackPanelDrawable implementation
   void Draw(
      TrackPanelDrawingContext &context,
      const wxRect &rect, unsigned iPass ) override;
};
#endif
