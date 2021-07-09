

#include "../CommonCommandFlags.h"
#include "../Prefs.h"
#include "../Project.h"
#include "../ProjectHistory.h"
#include "../ProjectWindow.h"
#include "../Track.h"
#include "../SelectionState.h"
#include "../TrackPanel.h"
#include "../TrackPanelAx.h"
#include "../commands/CommandContext.h"
#include "../commands/CommandManager.h"
#include "../toolbars/ToolManager.h"
#include "../widgets/AButton.h"
#include "../widgets/ASlider.h"
#include "../widgets/Meter.h"

// private helper classes and functions
namespace {

void NextOrPrevFrame(SneedacityProject &project, bool forward)
{
   // Focus won't take in a dock unless at least one descendant window
   // accepts focus.  Tell controls to take focus for the duration of this
   // function, only.  Outside of this, they won't steal the focus when
   // clicked.
   auto temp1 = AButton::TemporarilyAllowFocus();
   auto temp2 = ASlider::TemporarilyAllowFocus();
   auto temp3 = MeterPanel::TemporarilyAllowFocus();

   auto &toolManager = ToolManager::Get( project );
   auto botDock = toolManager.GetBotDock();


   // Define the set of windows we rotate among.
   static const unsigned rotationSize = 3u;

   wxWindow *const begin [rotationSize] = {
      ProjectWindow::Get( project ).GetTopPanel(),
      &TrackPanel::Get( project ),
      botDock,
   };

   const auto end = begin + rotationSize;

   // helper functions
   auto IndexOf = [&](wxWindow *pWindow) {
      return std::find(begin, end, pWindow) - begin;
   };

   auto FindAncestor = [&]() {
      wxWindow *pWindow = wxWindow::FindFocus();
      unsigned index = rotationSize;
      while ( pWindow &&
              (rotationSize == (index = IndexOf(pWindow) ) ) )
         pWindow = pWindow->GetParent();
      return index;
   };

   const auto idx = FindAncestor();
   if (idx == rotationSize)
      return;

   auto idx2 = idx;
   auto increment = (forward ? 1 : rotationSize - 1);

   while( idx != (idx2 = (idx2 + increment) % rotationSize) ) {
      wxWindow *toFocus = begin[idx2];
      bool bIsAnEmptyDock=false;
      if( idx2 != 1 )
         bIsAnEmptyDock = ((idx2==0) ? toolManager.GetTopDock() : botDock)->
         GetChildren().GetCount() < 1;

      // Skip docks that are empty (Bug 1564).
      if( !bIsAnEmptyDock ){
         toFocus->SetFocus();
         if ( FindAncestor() == idx2 )
            // The focus took!
            break;
      }
   }
}

/// \todo Merge related methods, OnPrevTrack and OnNextTrack.
void DoPrevTrack(
   SneedacityProject &project, bool shift, bool circularTrackNavigation )
{
   auto &projectHistory = ProjectHistory::Get( project );
   auto &trackFocus = TrackFocus::Get( project );
   auto &tracks = TrackList::Get( project );
   auto &selectionState = SelectionState::Get( project );

   auto t = trackFocus.Get();
   if( t == NULL )   // if there isn't one, focus on last
   {
      t = *tracks.Any().rbegin();
      trackFocus.Set( t );
      if (t)
         t->EnsureVisible( true );
      return;
   }

   Track* p = NULL;
   bool tSelected = false;
   bool pSelected = false;
   if( shift )
   {
      p = * -- tracks.FindLeader( t ); // Get previous track
      if( p == NULL )   // On first track
      {
         // JKC: wxBell() is probably for accessibility, so a blind
         // user knows they were at the top track.
         wxBell();
         if( circularTrackNavigation )
            p = *tracks.Any().rbegin();
         else
         {
            t->EnsureVisible();
            return;
         }
      }
      tSelected = t->GetSelected();
      if (p)
         pSelected = p->GetSelected();
      if( tSelected && pSelected )
      {
         selectionState.SelectTrack
            ( *t, false, false );
         trackFocus.Set( p );   // move focus to next track up
         if (p)
            p->EnsureVisible( true );
         return;
      }
      if( tSelected && !pSelected )
      {
         selectionState.SelectTrack
            ( *p, true, false );
         trackFocus.Set( p );   // move focus to next track up
         if (p)
            p->EnsureVisible( true );
         return;
      }
      if( !tSelected && pSelected )
      {
         selectionState.SelectTrack
            ( *p, false, false );
         trackFocus.Set( p );   // move focus to next track up
         if (p)
            p->EnsureVisible( true );
         return;
      }
      if( !tSelected && !pSelected )
      {
         selectionState.SelectTrack
            ( *t, true, false );
         trackFocus.Set( p );   // move focus to next track up
         if (p)
            p->EnsureVisible( true );
         return;
      }
   }
   else
   {
      p = * -- tracks.FindLeader( t ); // Get previous track
      if( p == NULL )   // On first track so stay there?
      {
         wxBell();
         if( circularTrackNavigation )
         {
            auto range = tracks.Leaders();
            p = * range.rbegin(); // null if range is empty
            trackFocus.Set( p );   // Wrap to the last track
            if (p)
               p->EnsureVisible( true );
            return;
         }
         else
         {
            t->EnsureVisible();
            return;
         }
      }
      else
      {
         trackFocus.Set( p );   // move focus to next track up
         p->EnsureVisible( true );
         return;
      }
   }
}

/// The following method moves to the next track,
/// selecting and unselecting depending if you are on the start of a
/// block or not.
void DoNextTrack(
   SneedacityProject &project, bool shift, bool circularTrackNavigation )
{
   auto &projectHistory = ProjectHistory::Get( project );
   auto &trackFocus = TrackFocus::Get( project );
   auto &tracks = TrackList::Get( project );
   auto &selectionState = SelectionState::Get( project );

   auto t = trackFocus.Get();   // Get currently focused track
   if( t == NULL )   // if there isn't one, focus on first
   {
      t = *tracks.Any().begin();
      trackFocus.Set( t );
      if (t)
         t->EnsureVisible( true );
      return;
   }

   if( shift )
   {
      auto n = * ++ tracks.FindLeader( t ); // Get next track
      if( n == NULL )   // On last track so stay there
      {
         wxBell();
         if( circularTrackNavigation )
            n = *tracks.Any().begin();
         else
         {
            t->EnsureVisible();
            return;
         }
      }
      auto tSelected = t->GetSelected();
      auto nSelected = n->GetSelected();
      if( tSelected && nSelected )
      {
         selectionState.SelectTrack
            ( *t, false, false );
         trackFocus.Set( n );   // move focus to next track down
         if (n)
            n->EnsureVisible( true );
         return;
      }
      if( tSelected && !nSelected )
      {
         selectionState.SelectTrack
            ( *n, true, false );
         trackFocus.Set( n );   // move focus to next track down
         if (n)
            n->EnsureVisible( true );
         return;
      }
      if( !tSelected && nSelected )
      {
         selectionState.SelectTrack
            ( *n, false, false );
         trackFocus.Set( n );   // move focus to next track down
         if (n)
            n->EnsureVisible( true );
         return;
      }
      if( !tSelected && !nSelected )
      {
         selectionState.SelectTrack
            ( *t, true, false );
         trackFocus.Set( n );   // move focus to next track down
         if (n)
            n->EnsureVisible( true );
         return;
      }
   }
   else
   {
      auto n = * ++ tracks.FindLeader( t ); // Get next track
      if( n == NULL )   // On last track so stay there
      {
         wxBell();
         if( circularTrackNavigation )
         {
            n = *tracks.Any().begin();
            trackFocus.Set( n );   // Wrap to the first track
            if (n)
               n->EnsureVisible( true );
            return;
         }
         else
         {
            t->EnsureVisible();
            return;
         }
      }
      else
      {
         trackFocus.Set( n );   // move focus to next track down
         n->EnsureVisible( true );
         return;
      }
   }
}

}

/// Namespace for functions for project navigation menu (part of Extra menu)
namespace NavigationActions {

// exported helper functions
// none

// Menu handler functions

struct Handler
   : CommandHandlerObject // MUST be the first base class!
   , ClientData::Base
   , PrefsListener
{

void OnPrevWindow(const CommandContext &context)
{
   auto &project = context.project;
   auto &window = GetProjectFrame( project );
   auto isEnabled = window.IsEnabled();

   wxWindow *w = wxGetTopLevelParent(wxWindow::FindFocus());
   const auto & list = window.GetChildren();
   auto iter = list.rbegin(), end = list.rend();

   // If the project window has the current focus, start the search with the
   // last child
   if (w == &window)
   {
   }
   // Otherwise start the search with the current window's previous sibling
   else
   {
      while (iter != end && *iter != w)
         ++iter;
      if (iter != end)
         ++iter;
   }

   // Search for the previous toplevel window
   for (; iter != end; ++iter)
   {
      // If it's a toplevel and is visible (we have come hidden windows), then
      // we're done
      w = *iter;
      if (w->IsTopLevel() && w->IsShown() && isEnabled)
      {
         break;
      }
   }

   // Ran out of siblings, so make the current project active
   if ((iter == end) && isEnabled)
   {
      w = &window;
   }

   // And make sure it's on top (only for floating windows...project window will
   // not raise)
   // (Really only works on Windows)
   w->Raise();


#if defined(__WXMAC__) || defined(__WXGTK__)
   // bug 868
   // Simulate a TAB key press before continuing, else the cycle of
   // navigation among top level windows stops because the keystrokes don't
   // go to the CommandManager.
   if (dynamic_cast<wxDialog*>(w)) {
      w->SetFocus();
   }
#endif
}

void OnNextWindow(const CommandContext &context)
{
   auto &project = context.project;
   auto &window = GetProjectFrame( project );
   auto isEnabled = window.IsEnabled();

   wxWindow *w = wxGetTopLevelParent(wxWindow::FindFocus());
   const auto & list = window.GetChildren();
   auto iter = list.begin(), end = list.end();

   // If the project window has the current focus, start the search with the
   // first child
   if (w == &window)
   {
   }
   // Otherwise start the search with the current window's next sibling
   else
   {
      // Find the window in this projects children.  If the window with the
      // focus isn't a child of this project (like when a dialog is created
      // without specifying a parent), then we'll get back NULL here.
      while (iter != end && *iter != w)
         ++iter;
      if (iter != end)
         ++iter;
   }

   // Search for the next toplevel window
   for (; iter != end; ++iter)
   {
      // If it's a toplevel, visible (we have hidden windows) and is enabled,
      // then we're done.  The IsEnabled() prevents us from moving away from
      // a modal dialog because all other toplevel windows will be disabled.
      w = *iter;
      if (w->IsTopLevel() && w->IsShown() && w->IsEnabled())
      {
         break;
      }
   }

   // Ran out of siblings, so make the current project active
   if ((iter == end) && isEnabled)
   {
      w = &window;
   }

   // And make sure it's on top (only for floating windows...project window will
   // not raise)
   // (Really only works on Windows)
   w->Raise();


#if defined(__WXMAC__) || defined(__WXGTK__)
   // bug 868
   // Simulate a TAB key press before continuing, else the cycle of
   // navigation among top level windows stops because the keystrokes don't
   // go to the CommandManager.
   if (dynamic_cast<wxDialog*>(w)) {
      w->SetFocus();
   }
#endif
}

void OnPrevFrame(const CommandContext &context)
{
   auto &project = context.project;
   NextOrPrevFrame(project, false);
}

void OnNextFrame(const CommandContext &context)
{
   auto &project = context.project;
   NextOrPrevFrame(project, true);
}

// Handler state:
bool mCircularTrackNavigation{};

void OnCursorUp(const CommandContext &context)
{
   auto &project = context.project;
   DoPrevTrack( project, false, mCircularTrackNavigation );
}

void OnCursorDown(const CommandContext &context)
{
   auto &project = context.project;
   DoNextTrack( project, false, mCircularTrackNavigation );
}

void OnFirstTrack(const CommandContext &context)
{
   auto &project = context.project;
   auto &trackFocus = TrackFocus::Get( project );
   auto &tracks = TrackList::Get( project );

   auto t = trackFocus.Get();
   if (!t)
      return;

   auto f = *tracks.Any().begin();
   if (t != f)
      trackFocus.Set(f);
   if (f)
      f->EnsureVisible( t != f );
}

void OnLastTrack(const CommandContext &context)
{
   auto &project = context.project;
   auto &trackFocus = TrackFocus::Get( project );
   auto &tracks = TrackList::Get( project );

   Track *t = trackFocus.Get();
   if (!t)
      return;

   auto l = *tracks.Any().rbegin();
   if (t != l)
      trackFocus.Set(l);
   if (l)
      l->EnsureVisible( t != l );
}

void OnShiftUp(const CommandContext &context)
{
   auto &project = context.project;
   DoPrevTrack( project, true, mCircularTrackNavigation );
}

void OnShiftDown(const CommandContext &context)
{
   auto &project = context.project;
   DoNextTrack( project, true, mCircularTrackNavigation );
}

void OnToggle(const CommandContext &context)
{
   auto &project = context.project;
   auto &trackFocus = TrackFocus::Get( project );
   auto &selectionState = SelectionState::Get( project );

   Track *t;

   t = trackFocus.Get();   // Get currently focused track
   if (!t)
      return;

   selectionState.SelectTrack
      ( *t, !t->GetSelected(), true );
   t->EnsureVisible( true );

   trackFocus.UpdateAccessibility();

   return;
}

void UpdatePrefs() override
{
   mCircularTrackNavigation =
      gPrefs->ReadBool(wxT("/GUI/CircularTrackNavigation"), false);
}
Handler()
{
   UpdatePrefs();
}
Handler( const Handler & ) PROHIBITED;
Handler &operator=( const Handler & ) PROHIBITED;

}; // struct Handler

} // namespace

// Handler is stateful.  Needs a factory registered with
// SneedacityProject.
static const SneedacityProject::AttachedObjects::RegisteredFactory key{
   [](SneedacityProject&) {
      return std::make_unique< NavigationActions::Handler >(); } };

static CommandHandlerObject &findCommandHandler(SneedacityProject &project) {
   return project.AttachedObjects::Get< NavigationActions::Handler >( key );
};

// Menu definitions

#define FN(X) (& NavigationActions::Handler :: X)

namespace {
using namespace MenuTable;
BaseItemSharedPtr ExtraGlobalCommands()
{
   // Ceci n'est pas un menu
   using Options = CommandManager::Options;

   static BaseItemSharedPtr items{
   ( FinderScope{ findCommandHandler },
   Items( wxT("Navigation"),
      Command( wxT("PrevWindow"), XXO("Move Backward Through Active Windows"),
         FN(OnPrevWindow), AlwaysEnabledFlag,
         Options{ wxT("Alt+Shift+F6") }.IsGlobal() ),
      Command( wxT("NextWindow"), XXO("Move Forward Through Active Windows"),
         FN(OnNextWindow), AlwaysEnabledFlag,
         Options{ wxT("Alt+F6") }.IsGlobal() )
   ) ) };
   return items;
}

AttachedItem sAttachment2{
   wxT("Optional/Extra/Part2"),
   Shared( ExtraGlobalCommands() )
};

BaseItemSharedPtr ExtraFocusMenu()
{
   static const auto FocusedTracksFlags = TracksExistFlag() | TrackPanelHasFocus();

   static BaseItemSharedPtr menu{
   ( FinderScope{ findCommandHandler },
   Menu( wxT("Focus"), XXO("Foc&us"),
      Command( wxT("PrevFrame"),
         XXO("Move &Backward from Toolbars to Tracks"), FN(OnPrevFrame),
         AlwaysEnabledFlag, wxT("Ctrl+Shift+F6") ),
      Command( wxT("NextFrame"),
         XXO("Move F&orward from Toolbars to Tracks"), FN(OnNextFrame),
         AlwaysEnabledFlag, wxT("Ctrl+F6") ),
      Command( wxT("PrevTrack"), XXO("Move Focus to &Previous Track"),
         FN(OnCursorUp), FocusedTracksFlags, wxT("Up") ),
      Command( wxT("NextTrack"), XXO("Move Focus to &Next Track"),
         FN(OnCursorDown), FocusedTracksFlags, wxT("Down") ),
      Command( wxT("FirstTrack"), XXO("Move Focus to &First Track"),
         FN(OnFirstTrack), FocusedTracksFlags, wxT("Ctrl+Home") ),
      Command( wxT("LastTrack"), XXO("Move Focus to &Last Track"),
         FN(OnLastTrack), FocusedTracksFlags, wxT("Ctrl+End") ),
      Command( wxT("ShiftUp"), XXO("Move Focus to P&revious and Select"),
         FN(OnShiftUp), FocusedTracksFlags, wxT("Shift+Up") ),
      Command( wxT("ShiftDown"), XXO("Move Focus to N&ext and Select"),
         FN(OnShiftDown), FocusedTracksFlags, wxT("Shift+Down") ),
      Command( wxT("Toggle"), XXO("&Toggle Focused Track"), FN(OnToggle),
         FocusedTracksFlags, wxT("Return") ),
      Command( wxT("ToggleAlt"), XXO("Toggle Focuse&d Track"), FN(OnToggle),
         FocusedTracksFlags, wxT("NUMPAD_ENTER") )
   ) ) };
   return menu;
}

AttachedItem sAttachment3{
   wxT("Optional/Extra/Part2"),
   Shared( ExtraFocusMenu() )
};

}

#undef FN
