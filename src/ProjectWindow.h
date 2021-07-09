/**********************************************************************

Sneedacity: A Digital Audio Editor

ProjectWindow.h

Paul Licameli split from SneedacityProject.h

**********************************************************************/

#ifndef __SNEEDACITY_PROJECT_WINDOW__
#define __SNEEDACITY_PROJECT_WINDOW__

#include <memory>
#include "ProjectWindowBase.h" // to inherit
#include "TrackPanelListener.h" // to inherit
#include "Prefs.h"

class Track;

class wxScrollBar;
class wxPanel;

class ProjectWindow;
void InitProjectWindow( ProjectWindow &window );

///\brief A top-level window associated with a project, and handling scrollbars
/// and zooming
class SNEEDACITY_DLL_API ProjectWindow final : public ProjectWindowBase
   , public TrackPanelListener
   , public PrefsListener
{
public:
   static ProjectWindow &Get( SneedacityProject &project );
   static const ProjectWindow &Get( const SneedacityProject &project );
   static ProjectWindow *Find( SneedacityProject *pProject );
   static const ProjectWindow *Find( const SneedacityProject *pProject );

   explicit ProjectWindow(
      wxWindow * parent, wxWindowID id,
      const wxPoint & pos, const wxSize &size,
      SneedacityProject &project );
   ~ProjectWindow() override;

   // Next available ID for sub-windows
   int NextWindowID();

   bool IsActive() override;
   bool IsIconized() const override;

   bool IsBeingDeleted() const { return mIsDeleting; }
   void SetIsBeingDeleted() { mIsDeleting = true; }

   wxWindow *GetMainPage() { return mMainPage; }
   wxPanel *GetMainPanel() { return mMainPanel; }
   wxPanel *GetTopPanel() { return mTopPanel; }

   void UpdateStatusWidths();

   class PlaybackScroller final : public wxEvtHandler
   {
   public:
      explicit PlaybackScroller(SneedacityProject *project);

      enum class Mode {
         Off,
         Refresh,
         Pinned,
         Right,
      };

      Mode GetMode() const { return mMode; }
      void Activate(Mode mode)
      {
         mMode = mode;
      }

      double GetRecentStreamTime() const { return mRecentStreamTime; }

   private:
      void OnTimer(wxCommandEvent &event);

      SneedacityProject *mProject;
      Mode mMode { Mode::Off };

      // During timer update, grab the volatile stream time just once, so that
      // various other drawing code can use the exact same value.
      double mRecentStreamTime{ -1.0 };
   };
   PlaybackScroller &GetPlaybackScroller() { return *mPlaybackScroller; }

   void SetNormalizedWindowState(wxRect pSizeAndLocation) {  mNormalizedWindowState = pSizeAndLocation;   }
   wxRect GetNormalizedWindowState() const { return mNormalizedWindowState;   }

   void RedrawProject(const bool bForceWaveTracks = false);

   void Zoom(double level);
   void ZoomInByFactor( double ZoomFactor );
   void ZoomOutByFactor( double ZoomFactor );
   void ZoomBy(double multiplier);
   void ZoomAfterImport(Track *pTrack);
   double GetZoomOfToFit() const;
   void DoZoomFit();

   void ApplyUpdatedTheme();

   // Scrollbars

   wxScrollBar &GetVerticalScrollBar() { return *mVsbar; }
   wxScrollBar &GetHorizontalScrollBar() { return *mHsbar; }

   void ScrollIntoView(double pos);
   void ScrollIntoView(int x);

   void OnScrollLeft();
   void OnScrollRight();

   void Rewind(bool shift);
   void SkipEnd(bool shift);

   void OnScrollLeftButton(wxScrollEvent & event);
   void OnScrollRightButton(wxScrollEvent & event);

   void FinishAutoScroll();
   void FixScrollbars();

   bool MayScrollBeyondZero() const;
   double ScrollingLowerBoundTime() const;
   // How many pixels are covered by the period from lowermost scrollable time, to the given time:
   // PRL: Bug1197: we seem to need to compute all in double, to avoid differing results on Mac
   double PixelWidthBeforeTime(double scrollto) const;
   void SetHorizontalThumb(double scrollto);

   // PRL:  old and incorrect comment below, these functions are used elsewhere than TrackPanel
   // TrackPanel access
   wxSize GetTPTracksUsableArea() /* not override */;
   void RefreshTPTrack(Track* pTrk, bool refreshbacking = true) /* not override */;

   void TP_RedrawScrollbars() override;
   void TP_ScrollLeft() override;
   void TP_ScrollRight() override;
   void TP_ScrollWindow(double scrollto) override;
   bool TP_ScrollUpDown(int delta) override;
   void TP_HandleResize() override;

 private:

   void OnThemeChange(wxCommandEvent & evt);

   // PrefsListener implementation
   void UpdatePrefs() override;

 public:
   // Message Handlers

   void OnMenu(wxCommandEvent & event);
   void OnUpdateUI(wxUpdateUIEvent & event);

   void MacShowUndockedToolbars(bool show);
   void OnActivate(wxActivateEvent & event);

   void OnMouseEvent(wxMouseEvent & event);
   void OnIconize(wxIconizeEvent &event);
   void OnSize(wxSizeEvent & event);
   void HandleResize();
   void UpdateLayout();
   void OnShow(wxShowEvent & event);
   void OnMove(wxMoveEvent & event);
   void DoScroll();
   void OnScroll(wxScrollEvent & event);
   void OnToolBarUpdate(wxCommandEvent & event);
   void OnUndoPushedModified( wxCommandEvent & );
   void OnUndoRedo( wxCommandEvent & );
   void OnUndoReset( wxCommandEvent & );

   bool mbInitializingScrollbar{ false };

private:
   wxRect mNormalizedWindowState;

   wxPanel *mTopPanel{};
   wxWindow * mMainPage{};
   wxPanel * mMainPanel{};
   wxScrollBar *mHsbar{};
   wxScrollBar *mVsbar{};

   int mNextWindowID{};

   bool mAutoScrolling{ false };
   bool mActive{ true };
   bool mIconized{ false };
   bool mShownOnce{ false };



   bool mIsDeleting{ false };

private:

   std::unique_ptr<PlaybackScroller> mPlaybackScroller;

   DECLARE_EVENT_TABLE()
};

void GetDefaultWindowRect(wxRect *defRect);
void GetNextWindowPlacement(wxRect *nextRect, bool *pMaximized, bool *pIconized);

#endif
