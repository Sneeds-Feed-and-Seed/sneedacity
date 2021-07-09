/**********************************************************************

Sneedacity: A Digital Audio Editor

WavelTrackButtonHandles.h

Paul Licameli split from TrackPanel.cpp

**********************************************************************/

#ifndef __SNEEDACITY_TRACK_BUTTON_HANDLES__
#define __SNEEDACITY_TRACK_BUTTON_HANDLES__

#include "../ui/ButtonHandle.h"

class wxMouseState;

class MinimizeButtonHandle final : public ButtonHandle
{
   MinimizeButtonHandle(const MinimizeButtonHandle&) = delete;

protected:
   Result CommitChanges
      (const wxMouseEvent &event, SneedacityProject *pProject, wxWindow *pParent)
      override;

   TranslatableString Tip(
      const wxMouseState &state, SneedacityProject &) const override;

public:
   explicit MinimizeButtonHandle
      ( const std::shared_ptr<Track> &pTrack, const wxRect &rect );

   MinimizeButtonHandle &operator=(const MinimizeButtonHandle&) = default;

   virtual ~MinimizeButtonHandle();

   static UIHandlePtr HitTest
      (std::weak_ptr<MinimizeButtonHandle> &holder,
       const wxMouseState &state, const wxRect &rect, TrackPanelCell *pCell);
};

////////////////////////////////////////////////////////////////////////////////
class SelectButtonHandle final : public ButtonHandle
{
   SelectButtonHandle(const SelectButtonHandle&) = delete;

protected:
   Result CommitChanges
      (const wxMouseEvent &event, SneedacityProject *pProject, wxWindow *pParent)
      override;

   TranslatableString Tip(
      const wxMouseState &state, SneedacityProject &) const override;

public:
   explicit SelectButtonHandle
      ( const std::shared_ptr<Track> &pTrack, const wxRect &rect );

   SelectButtonHandle &operator=(const SelectButtonHandle&) = default;

   virtual ~SelectButtonHandle();

   static UIHandlePtr HitTest
      (std::weak_ptr<SelectButtonHandle> &holder,
       const wxMouseState &state, const wxRect &rect, TrackPanelCell *pCell);
};

////////////////////////////////////////////////////////////////////////////////
class CloseButtonHandle final : public ButtonHandle
{
   CloseButtonHandle(const CloseButtonHandle&) = delete;

protected:
   Result CommitChanges
      (const wxMouseEvent &event, SneedacityProject *pProject, wxWindow *pParent)
      override;

   TranslatableString Tip(
      const wxMouseState &state, SneedacityProject &project) const override;

   bool StopsOnKeystroke () override { return true; }
   
public:
   explicit CloseButtonHandle
      ( const std::shared_ptr<Track> &pTrack, const wxRect &rect );

   CloseButtonHandle &operator=(const CloseButtonHandle&) = default;

   virtual ~CloseButtonHandle();

   static UIHandlePtr HitTest
      (std::weak_ptr<CloseButtonHandle> &holder,
       const wxMouseState &state, const wxRect &rect, TrackPanelCell *pCell);
};

////////////////////////////////////////////////////////////////////////////////

class MenuButtonHandle final : public ButtonHandle
{
   MenuButtonHandle(const MenuButtonHandle&) = delete;

protected:
   Result CommitChanges
      (const wxMouseEvent &event, SneedacityProject *pProject, wxWindow *pParent)
      override;

   TranslatableString Tip(
      const wxMouseState &state, SneedacityProject &) const override;

public:
   explicit MenuButtonHandle
      ( const std::shared_ptr<TrackPanelCell> &pCell,
        const std::shared_ptr<Track> &pTrack, const wxRect &rect );

   MenuButtonHandle &operator=(const MenuButtonHandle&) = default;

   virtual ~MenuButtonHandle();

   static UIHandlePtr HitTest
      (std::weak_ptr<MenuButtonHandle> &holder,
       const wxMouseState &state, const wxRect &rect,
       const std::shared_ptr<TrackPanelCell> &pCell);

private:
   std::weak_ptr<TrackPanelCell> mpCell;
};

#endif
