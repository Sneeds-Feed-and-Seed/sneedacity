/**********************************************************************

Sneedacity: A Digital Audio Editor

SpectrumVRulerControls.h

Paul Licameli split from WaveTrackVRulerControls.h

**********************************************************************/

#ifndef __SNEEDACITY_SPECTRUM_VRULER_CONTROLS__
#define __SNEEDACITY_SPECTRUM_VRULER_CONTROLS__

#include "../../../ui/TrackVRulerControls.h" // to inherit

class WaveTrack;
class SpectrumVZoomHandle;

class SpectrumVRulerControls final : public TrackVRulerControls
{
   SpectrumVRulerControls(const SpectrumVRulerControls&) = delete;
   SpectrumVRulerControls &operator=(const SpectrumVRulerControls&) = delete;

public:
   explicit
   SpectrumVRulerControls( const std::shared_ptr<TrackView> &pTrackView )
      : TrackVRulerControls( pTrackView ) {}
   ~SpectrumVRulerControls() override;

   std::vector<UIHandlePtr> HitTest(
      const TrackPanelMouseState &state,
      const SneedacityProject *) override;

   unsigned HandleWheelRotation(
      const TrackPanelMouseEvent &event,
      SneedacityProject *pProject) override;
   static unsigned DoHandleWheelRotation(
      const TrackPanelMouseEvent &evt, SneedacityProject *pProject,
      WaveTrack *wt);

private:
   // TrackPanelDrawable implementation
   void Draw(
      TrackPanelDrawingContext &context,
      const wxRect &rect, unsigned iPass ) override;

   // TrackVRulerControls implementation
   void UpdateRuler( const wxRect &rect ) override;

   static void DoUpdateVRuler( const wxRect &rect, const WaveTrack *wt );

   std::weak_ptr<SpectrumVZoomHandle> mVZoomHandle;
};

#endif
