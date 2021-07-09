/**********************************************************************

Sneedacity: A Digital Audio Editor

LabelGlyphHandle.h

Paul Licameli split from TrackPanel.cpp

**********************************************************************/

#ifndef __SNEEDACITY_LABEL_GLYPH_HANDLE__
#define __SNEEDACITY_LABEL_GLYPH_HANDLE__

#include "LabelDefaultClickHandle.h"

class wxMouseState;
class LabelTrack;
struct LabelTrackEvent;
class NotifyingSelectedRegion;
class ZoomInfo;

/// mEdge:
/// 0 if not over a glyph,
/// else a bitwise or of :
/// 1 if over the left-hand glyph,
/// 2 if over the right-hand glyph on a label,
/// 4 if over center.
///
///   mMouseLabelLeft - index of any left label hit
///   mMouseLabelRight - index of any right label hit
///
struct LabelTrackHit
{
   LabelTrackHit( const std::shared_ptr<LabelTrack> &pLT );
   ~LabelTrackHit();

   int mEdge{};
   int mMouseOverLabelLeft{ -1 };    /// Keeps track of which left label the mouse is currently over.
   int mMouseOverLabelRight{ -1 };   /// Keeps track of which right label the mouse is currently over.
   bool mbIsMoving {};
   bool mIsAdjustingLabel {};

   std::shared_ptr<LabelTrack> mpLT {};

   void OnLabelPermuted( LabelTrackEvent &e );
};

class LabelGlyphHandle final : public LabelDefaultClickHandle
{
   static HitTestPreview HitPreview(bool hitCenter);

public:
   explicit LabelGlyphHandle
      (const std::shared_ptr<LabelTrack> &pLT,
       const wxRect &rect, const std::shared_ptr<LabelTrackHit> &pHit);

   LabelGlyphHandle &operator=(const LabelGlyphHandle&) = default;
   
   static UIHandlePtr HitTest
      (std::weak_ptr<LabelGlyphHandle> &holder,
       const wxMouseState &state,
       const std::shared_ptr<LabelTrack> &pLT, const wxRect &rect);

   virtual ~LabelGlyphHandle();

   void Enter(bool forward, SneedacityProject *) override;

   Result Click
      (const TrackPanelMouseEvent &event, SneedacityProject *pProject) override;

   Result Drag
      (const TrackPanelMouseEvent &event, SneedacityProject *pProject) override;

   HitTestPreview Preview
      (const TrackPanelMouseState &state, SneedacityProject *pProject)
      override;

   Result Release
      (const TrackPanelMouseEvent &event, SneedacityProject *pProject,
       wxWindow *pParent) override;

   Result Cancel(SneedacityProject *pProject) override;

   bool StopsOnKeystroke() override { return true; }

   std::shared_ptr<LabelTrackHit> mpHit{};

   static UIHandle::Result NeedChangeHighlight
      (const LabelGlyphHandle &oldState, const LabelGlyphHandle &newState);

private:
   void HandleGlyphClick
      (LabelTrackHit &hit,
       const wxMouseEvent & evt, const wxRect & r, const ZoomInfo &zoomInfo,
       NotifyingSelectedRegion &newSel);
   bool HandleGlyphDragRelease
      (SneedacityProject &project,
       LabelTrackHit &hit,
       const wxMouseEvent & evt, wxRect & r, const ZoomInfo &zoomInfo,
       NotifyingSelectedRegion &newSel);

   void MayAdjustLabel
      ( LabelTrackHit &hit,
        int iLabel, int iEdge, bool bAllowSwapping, double fNewTime);
   void MayMoveLabel( int iLabel, int iEdge, double fNewTime);

   std::shared_ptr<LabelTrack> mpLT {};
   wxRect mRect {};

   /// Displacement of mouse cursor from the centre being dragged.
   int mxMouseDisplacement;
};

#endif
