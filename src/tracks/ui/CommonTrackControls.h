/**********************************************************************

Sneedacity: A Digital Audio Editor

CommonTrackControls.h

Paul Licameli split from TrackControls.h

**********************************************************************/

#ifndef __SNEEDACITY_COMMON_TRACK_CONTROLS__
#define __SNEEDACITY_COMMON_TRACK_CONTROLS__

#include "TrackControls.h" // to inherit

class CloseButtonHandle;
class MenuButtonHandle;
class PopupMenuTable;
class MinimizeButtonHandle;
class SelectButtonHandle;
class TrackSelectHandle;

namespace TrackInfo{ struct TCPLine; }
using TCPLines = std::vector< TrackInfo::TCPLine >;

class SNEEDACITY_DLL_API CommonTrackControls /* not final */ : public TrackControls
{
public:
   using TrackControls::TrackControls;

   // This is passed to the InitUserData() methods of the PopupMenuTable
   // objects returned by GetMenuExtension:
   struct InitMenuData
   {
   public:
      SneedacityProject &project;
      Track *pTrack;
      wxWindow *pParent;
      unsigned result;
   };

   // This decides what details to draw in the track control panel, besides
   // those at the bottom
   virtual const TCPLines& GetTCPLines() const;

   // To help subclasses define GetTCPLines
   static const TCPLines& StaticTCPLines();

protected:
   // An override is supplied for derived classes to call through but it is
   // still marked pure virtual
   virtual std::vector<UIHandlePtr> HitTest
      (const TrackPanelMouseState &state,
       const SneedacityProject *) override = 0;

   unsigned DoContextMenu
      (const wxRect &rect, wxWindow *pParent, wxPoint *pPosition,
       SneedacityProject *pProject) override;
   virtual PopupMenuTable *GetMenuExtension(Track *pTrack) = 0;

   // TrackPanelDrawable implementation
   void Draw(
      TrackPanelDrawingContext &context,
      const wxRect &rect, unsigned iPass ) override;

   wxRect DrawingArea(
      TrackPanelDrawingContext &,
      const wxRect &rect, const wxRect &panelRect, unsigned iPass ) override;

   std::weak_ptr<CloseButtonHandle> mCloseHandle;
   std::weak_ptr<MenuButtonHandle> mMenuHandle;
   std::weak_ptr<MinimizeButtonHandle> mMinimizeHandle;
   std::weak_ptr<SelectButtonHandle> mSelectButtonHandle;
   std::weak_ptr<TrackSelectHandle> mSelectHandle;
};

#endif
