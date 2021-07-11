/**********************************************************************

   Sneedacity - A Digital Audio Editor
   Copyright 1999-2018 Audacity Team, 2021 Sneedacity Team
   License: GPL v2 - see LICENSE.txt

   Dominic Mazzoni
   Dan Horgan
   James Crook

******************************************************************//**

\class ScreenshotCommand
\brief Implements a command for capturing various areas of the screen or
project window.  It's one big if-elseif switch statement with lots of 
small calculations of rectangles.

*//*******************************************************************/


#include "ScreenshotCommand.h"

#include <mutex>

#include "LoadCommands.h"
#include "../Project.h"
#include <wx/toplevel.h>
#include <wx/dcscreen.h>
#include <wx/dcmemory.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/valgen.h>

#include "../AdornedRulerPanel.h"
#include "../BatchCommands.h"
#include "../TrackPanel.h"
#include "../effects/Effect.h"
#include "../toolbars/ToolManager.h"
#include "../Prefs.h"
#include "../ProjectWindow.h"
#include "../Shuttle.h"
#include "../ShuttleGui.h"
#include "../Track.h"
#include "CommandContext.h"
#include "CommandManager.h"

const ComponentInterfaceSymbol ScreenshotCommand::Symbol
{ XO("Screenshot") };

namespace{ BuiltinCommandsModule::Registration< ScreenshotCommand > reg; }


static const EnumValueSymbol
kCaptureWhatStrings[ ScreenshotCommand::nCaptureWhats ] =
{
   { XO("Window") },
   { wxT("FullWindow"), XO("Full Window") },
   { wxT("WindowPlus"), XO("Window Plus") },
   { XO("Fullscreen") },
   { XO("Toolbars") },
   { XO("Effects") },
   { XO("Scriptables") },
   { XO("Preferences") },
   { XO("Selectionbar") },
   { wxT("SpectralSelection"), XO("Spectral Selection") },
   { XO("Timer") },
   { XO("Tools") },
   { XO("Transport") },
   { XO("Mixer") },
   { XO("Meter") },
   { wxT("PlayMeter"), XO("Play Meter") },
   { wxT("RecordMeter"), XO("Record Meter") },
   { XO("Edit") },
   { XO("Device") },
   { XO("Scrub") },
   { XO("Play-at-Speed") },
   { XO("Trackpanel") },
   { XO("Ruler") },
   { XO("Tracks") },
   { wxT("FirstTrack"),       XO("First Track") },
   { wxT("FirstTwoTracks"),   XO("First Two Tracks") },
   { wxT("FirstThreeTracks"), XO("First Three Tracks") },
   { wxT("FirstFourTracks"),  XO("First Four Tracks") },
   { wxT("SecondTrack"),      XO("Second Track") },
   { wxT("TracksPlus"),       XO("Tracks Plus") },
   { wxT("FirstTrackPlus"),   XO("First Track Plus") },
   { wxT("AllTracks"),        XO("All Tracks") },
   { wxT("AllTracksPlus"),    XO("All Tracks Plus") },
};


static const EnumValueSymbol
kBackgroundStrings[ ScreenshotCommand::nBackgrounds ] =
{
   // These are acceptable dual purpose internal/visible names
   { XO("Blue") },
   /* i18n-hint:  This really means the color, not as in "white noise" */
   { XC("White", "color") },
   { XO("None") },
};


ScreenshotCommand::ScreenshotCommand()
{
   mbBringToTop=true;
   mIgnore=NULL;
   
   static std::once_flag flag;
   std::call_once( flag, []{
      SneedacityCommand::SetVetoDialogHook( MayCapture );
      Effect::SetVetoDialogHook( MayCapture );
   });
}

bool ScreenshotCommand::DefineParams( ShuttleParams & S ){
   S.Define(                               mPath,        wxT("Path"),         wxT(""));
   S.DefineEnum(                           mWhat,        wxT("CaptureWhat"),  kwindow,kCaptureWhatStrings, nCaptureWhats );
   S.DefineEnum(                           mBack,        wxT("Background"),   kNone, kBackgroundStrings, nBackgrounds );
   S.Define(                               mbBringToTop, wxT("ToTop"), true );
   return true;
};

void ScreenshotCommand::PopulateOrExchange(ShuttleGui & S)
{
   S.AddSpace(0, 5);

   S.StartMultiColumn(2, wxALIGN_CENTER);
   {
      S.TieTextBox(  XXO("Path:"), mPath);
      S.TieChoice(   XXO("Capture What:"),
         mWhat, Msgids(kCaptureWhatStrings, nCaptureWhats));
      S.TieChoice(   XXO("Background:"),
         mBack, Msgids(kBackgroundStrings, nBackgrounds));
      S.TieCheckBox( XXO("Bring To Top"), mbBringToTop);
   }
   S.EndMultiColumn();
}

// static member variable.
void (*ScreenshotCommand::mIdleHandler)(wxIdleEvent& event) = NULL;
static SneedacityProject *pIdleHandlerProject = nullptr;
// This static variable is used to get from an idle event to the screenshot
// command that caused the idle event interception to be set up.
ScreenshotCommand * ScreenshotCommand::mpShooter=NULL;

// IdleHandler is expected to be called from EVT_IDLE when a dialog has been
// fully created.  Usually the dialog will have been created by invoking
// an effects gui.
void IdleHandler(wxIdleEvent& event){
   event.Skip();
   wxWindow * pWin = dynamic_cast<wxWindow*>(event.GetEventObject());
   wxASSERT( pWin );
   pWin->Unbind(wxEVT_IDLE, IdleHandler);
   CommandContext context( *pIdleHandlerProject );
   // We have the relevant window, so go and capture it.
   if( ScreenshotCommand::mpShooter )
      ScreenshotCommand::mpShooter->CaptureWindowOnIdle( context, pWin );
}

void ScreenshotCommand::SetIdleHandler( SneedacityProject &project )
{
   mIdleHandler = IdleHandler;
   pIdleHandlerProject = &project;
}

wxTopLevelWindow *ScreenshotCommand::GetFrontWindow(SneedacityProject *project)
{
   wxWindow *front = NULL;
   wxWindow *proj = wxGetTopLevelParent( ProjectWindow::Find( project ) );

   for (auto & win : wxTopLevelWindows)
   {
      win = wxGetTopLevelParent(win);
      if (win != mIgnore && win != proj  && win->IsShown()) {
         front = win;
         break;
      }
   }

   if (!front || !front->IsTopLevel()) {
      return (wxTopLevelWindow *)proj;
   }

   return (wxTopLevelWindow *)front;
}

wxRect ScreenshotCommand::GetBackgroundRect()
{
   wxRect r;

   r.x = 16;
   r.y = 16;
   r.width = r.x * 2;
   r.height = r.y * 2;

   return r;
}

static void Yield()
{
   int cnt;
   for (cnt = 10; cnt && !wxTheApp->Yield(true); cnt--) {
      wxMilliSleep(10);
   }
   wxMilliSleep(200);
   for (cnt = 10; cnt && !wxTheApp->Yield(true); cnt--) {
      wxMilliSleep(10);
   }
}

bool ScreenshotCommand::Capture(
   const CommandContext & context,
   const wxString &filename,
                          wxWindow *window, wxRect r,
                          bool bg)
{
   int width = r.width;
   int height = r.height;
   if( r.width == 0 )
      return false;
   if (window ) {
      wxWindow * win = window;
      wxTopLevelWindow * top_win= nullptr;
      if( !window->IsTopLevel())
         win = wxGetTopLevelParent(window);
      top_win = dynamic_cast<wxTopLevelWindow*>( win );
      if( (!bHasBringToTop || mbBringToTop) && (!top_win || !top_win->IsActive()) ){
         win->Raise();
         Yield();
      }
   }


   int screenW, screenH;
   wxDisplaySize(&screenW, &screenH);
   // Bug 1378 workaround.
   // wx 3.0.2 has a bug in Blit from ScreenDC where in default mode 
   // much is drawn transparent - including for example black text!
   // Forcing 24 bit here is a workaround.
   wxBitmap full(screenW, screenH, 24);

   wxScreenDC screenDC;
   wxMemoryDC fullDC;

#if defined(__WXMAC__) && !wxCHECK_VERSION(3, 0, 0)
   full = DoGetAsBitmap(NULL);
#else
   // We grab the whole screen image since there seems to be a problem with
   // using non-zero source coordinates on OSX.  (as of wx2.8.9)
   fullDC.SelectObject(full);
   fullDC.Blit(0, 0, screenW, screenH, &screenDC, 0, 0);
   fullDC.SelectObject(wxNullBitmap);
#endif

   //wxRect r(x, y, width, height);


   // Convert to screen coordinates if needed
   if (window && window->GetParent() && !window->IsTopLevel()) {
      r.SetPosition(window->GetParent()->ClientToScreen(r.GetPosition()));
   }

   // Ensure within bounds (x/y are negative on Windows when maximized)
   r.Intersect(wxRect(0, 0, screenW, screenH));

   // Extract the actual image
   wxBitmap part = full.GetSubBitmap(r);

   // Add a background
   if (bg && mBackground) {
      wxRect b = GetBackgroundRect();

      wxBitmap back(width + b.width, height + b.height);
      fullDC.SelectObject(back);

      fullDC.SetBackground(wxBrush(mBackColor, wxBRUSHSTYLE_SOLID));
      fullDC.Clear();

      fullDC.DrawBitmap(part, b.x, b.y);
      fullDC.SelectObject(wxNullBitmap);

      part = back;
   }

   // Save the final image
   wxImage image = part.ConvertToImage();
   ::wxBell();

   if (image.SaveFile(filename)) {
      // flush
      context.Status( wxString::Format( _("Saved %s"), filename ), true );
   }
   else {
      context.Error(
         wxString::Format( _("Error trying to save file: %s"), filename ) );
      return false;
   }
   return true;
}

bool ScreenshotCommand::CaptureToolbar(
   const CommandContext & context,
   ToolManager *man, int type, const wxString &name)
{
   bool visible = man->IsVisible(type);
   if (!visible) {
      man->ShowHide(type);
      Yield();
   }

   wxWindow *w = man->GetToolBar(type);
   int x = 0, y = 0;
   int width, height;

   w->ClientToScreen(&x, &y);
   w->GetParent()->ScreenToClient(&x, &y);
   w->GetClientSize(&width, &height);
   
   bool result = Capture(context, name, w, wxRect(x, y, width, height));

   if (!visible) {
      man->ShowHide(type);
      if (mIgnore)
         mIgnore->Raise();
   }
   return result;
}

bool ScreenshotCommand::CaptureDock(
   const CommandContext & context,
   wxWindow *win, const wxString &FileName)
{
   int x = 0, y = 0;
   int width, height;

   win->ClientToScreen(&x, &y);
   win->GetParent()->ScreenToClient(&x, &y);
   win->GetClientSize(&width, &height);

   return Capture(context, FileName, win, wxRect(x, y, width, height));
}

// Handed a dialog, which it is given the option to capture.
bool ScreenshotCommand::MayCapture( wxDialog * pDlg )
{
   if( mIdleHandler == NULL )
      return false;
   pDlg->Bind( wxEVT_IDLE, mIdleHandler );
   mIdleHandler = NULL;
   pDlg->ShowModal();
   return true;
}

void ScreenshotCommand::CaptureWindowOnIdle( 
   const CommandContext & context,
   wxWindow * pWin )
{
   wxDialog * pDlg = dynamic_cast<wxDialog*>(pWin);
   if( !pDlg ){
      wxLogDebug("Event from bogus dlg" );
      return;
   }

   wxPoint Pos = pDlg->GetScreenPosition();
   wxSize Siz = pDlg->GetSize();
   wxString Title = pDlg->GetTitle();

   // Remove '/' from "Sliding Time Scale/Pitch Shift..."
   // and any other effects that have illegal filename characters.
   Title.Replace( "/", "" );
   Title.Replace( ":", "" );
   wxString Name = mDirToWriteTo + Title + ".png";

   wxLogDebug("Taking screenshot of window %s (%i,%i,%i,%i)", Name, 
         Pos.x, Pos.y, Siz.x, Siz.y );
   // This delay is needed, as dialogs take a moment or two to fade in.
   wxMilliSleep( 400 );
   // JKC: The border of 7 pixels was determined from a trial capture and then measuring
   // in the GIMP.  I'm unsure where the border comes from.
   Capture( context, Name, pDlg, wxRect((int)Pos.x+7, (int)Pos.y, (int)Siz.x-14, (int)Siz.y-7) );

   // We've captured the dialog, so now dismiss the dialog.
   wxCommandEvent Evt( wxEVT_BUTTON, wxID_CANCEL );
   pDlg->GetEventHandler()->AddPendingEvent( Evt );
}

void ScreenshotCommand::CapturePreferences( 
   const CommandContext & context,
   SneedacityProject * pProject, const wxString &FileName ){
   (void)&FileName;//compiler food.
   (void)&context;
   CommandManager &commandManager = CommandManager::Get( *pProject );

   // Yucky static variables.  Is there a better way?  The problem is that we need the
   // idle callback to know more about what to do.
#ifdef __WXMSW__
   mDirToWriteTo = FileName.BeforeLast('\\') + "\\";
#else
   mDirToWriteTo = FileName.BeforeLast('/') + "/";
#endif
   mpShooter = this;
   const int nPrefsPages = 19;

   for( int i=0;i<nPrefsPages;i++){
      // The handler is cleared each time it is used.
      SetIdleHandler( context.project );
      gPrefs->Write(wxT("/Prefs/PrefsCategory"), (long)i);
      gPrefs->Flush();
      CommandID Command{ wxT("Preferences") };
      const CommandContext projectContext( *pProject );
      if( !MacroCommands::HandleTextualCommand( commandManager,
         Command, projectContext, AlwaysEnabledFlag, true ) )
      {
         // using GET in a log message for devs' eyes only
         wxLogDebug("Command %s not found", Command.GET() );
      }
      // This sleep is not needed, but gives user a chance to see the
      // dialogs as they whizz by.
      wxMilliSleep( 200 );
   }
}

void ScreenshotCommand::CaptureEffects(
   const CommandContext & context,
   SneedacityProject * pProject, const wxString &FileName )
{
   (void)pProject;
   (void)&FileName;//compiler food.
   (void)&context;
#define TRICKY_CAPTURE
#define CAPTURE_NYQUIST_TOO
   // Commented out the effects that don't have dialogs.
   // Also any problematic ones, 
   CaptureCommands( context, {
#ifdef TRICKY_CAPTURE
      //"Contrast...", // renamed
      "ContrastAnalyser",
      //"Plot Spectrum...", // renamed
      "PlotSpectrum",

      "Auto Duck...",  // needs a track below.
      //"Spectral edit multi tool",
      "Spectral edit parametric EQ...", // Needs a spectral selection.
      "Spectral edit shelves...",

      //"Noise Reduction...", // Exits twice...
      //"SC4...", //Has 'Close' rather than 'Cancel'.
#endif
      "Amplify...",
      "Bass and Treble...",
      "Change Pitch...",
      "Change Speed...",
      "Change Tempo...",
      "Click Removal...",
      "Compressor...",
      "Distortion...",
      "Echo...",
      //"Equalization...",
      "Filter Curve EQ...",
      "Graphic EQ...",
      //"Fade In",
      //"Fade Out",
      //"Invert",
      "Normalize...",
      "Paulstretch...",
      "Phaser...",
      //"Repair",
      "Repeat...",
      "Reverb...",
      //"Reverse",
      "Sliding Stretch...",
      "Truncate Silence...",
      "Wahwah...",
      // Sole LADSPA effect...
#ifdef CAPTURE_NYQUIST_TOO
      "Adjustable Fade...",
      "Clip Fix...",
      //"Crossfade Clips",
      "Crossfade Tracks...",
      "Delay...",
      "High Pass Filter...",
      "Limiter...",
      "Low Pass Filter...",
      "Notch Filter...",
      "Nyquist Effects Prompt...",
      //"Studio Fade Out",
      "Tremolo...",
      "Vocal Reduction and Isolation...",
      "Vocal Remover...",
      "Vocoder...",
#endif
      // Generators.....
      "Chirp...",
      "DTMF Tones...",
      "Noise...",
      "Silence...",
      "Tone...",
#ifdef CAPTURE_NYQUIST_TOO
      "Pluck...",
      "Rhythm Track...",
      "Risset Drum...",
      "Sample Data Import...",
#endif
      // Analyzers...
      "Find Clipping...",
#ifdef CAPTURE_NYQUIST_TOO
      "Beat Finder...",
      "Label Sounds...",
      "Regular Interval Labels...",
      "Sample Data Export...",
#endif
   } );
}

void ScreenshotCommand::CaptureScriptables( 
   const CommandContext & context,
   SneedacityProject * pProject, const wxString &FileName )
{
   (void)pProject;
   (void)&FileName;//compiler food.
   (void)&context;

   CaptureCommands( context, {
      "SelectTime",
      "SelectFrequencies",
      "SelectTracks",
      "SetTrackStatus",
      "SetTrackAudio",
      "SetTrackVisuals",
      "GetPreference",
      "SetPreference",
      "SetClip",
      "SetEnvelope",
      "SetLabel",
      "SetProject",

      "Select",
      "SetTrack",
      "GetInfo",
      "Message",
      "Help", // Help on individual commands
      "Import2",
      "Export2",
      "OpenProject2",
      "SaveProject2",
      "Drag",
      "CompareAudio",
      "Screenshot",
   } );

}


void ScreenshotCommand::CaptureCommands( 
   const CommandContext & context, const wxArrayStringEx & Commands ){
   SneedacityProject * pProject = &context.project;
   CommandManager &manager = CommandManager::Get( *pProject );
   wxString Str;
   // Yucky static variables.  Is there a better way?  The problem is that we need the
   // idle callback to know more about what to do.
#ifdef __WXMSW__
   mDirToWriteTo = mFileName.BeforeLast('\\') + "\\";
#else
   mDirToWriteTo = mFileName.BeforeLast('/') + "/";
#endif
   mpShooter = this;

   for( size_t i=0;i<Commands.size();i++){
      // The handler is cleared each time it is used.
      SetIdleHandler( context.project );
      Str = Commands[i];
      const CommandContext projectContext( *pProject );
      if( !manager.HandleTextualCommand( Str, projectContext, AlwaysEnabledFlag, true ) )
      {
         wxLogDebug("Command %s not found", Str);
      }
      // This particular sleep is not needed, but gives user a chance to see the
      // dialogs as they whizz by.
      wxMilliSleep( 200 );
   }
}

wxString ScreenshotCommand::MakeFileName(const wxString &path, const wxString &basename)
{
   // If the path is a full file name, then use it.
   if( path.EndsWith( ".png" ) )
      return path;

   // Otherwise make up a file name that has not been used already.
   wxFileName prefixPath;
   prefixPath.AssignDir(path);
   wxString prefix = prefixPath.GetPath
      (wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);

   wxString filename;
   int i = 0;
   do {
      filename.Printf(wxT("%s%s%03d.png"),
                      prefix, basename, i);
      i++;
   } while (::wxFileExists(filename));

   return filename;
}

void ScreenshotCommand::GetDerivedParams()
{
      // Read the parameters that were passed in
   mFilePath    = mPath;
   mCaptureMode = mWhat;

   // Build a suitable filename
   mFileName = MakeFileName(mFilePath,
      kCaptureWhatStrings[ mCaptureMode ].Translation() );

   if (mBack == kBlue)
   {
      mBackground = true;
      mBackColor = wxColour(51, 102, 153);
   }
   else if (mBack == kWhite)
   {
      mBackground = true;
      mBackColor = wxColour(255, 255, 255);
   }
   else
   {
      mBackground = false;
   }

}

wxRect ScreenshotCommand::GetWindowRect(wxTopLevelWindow *w){
   int x = 0, y = 0;
   int width, height;

   w->ClientToScreen(&x, &y);
   w->GetClientSize(&width, &height);
   return wxRect( x,y,width,height);
}

wxRect ScreenshotCommand::GetFullWindowRect(wxTopLevelWindow *w){

   wxRect r = w->GetRect();
   r.SetPosition(w->GetScreenPosition());
   r = w->GetScreenRect();

#if defined(__WXGTK__)
   // In wxGTK, we need to include decoration sizes
   r.width += (wxSystemSettings::GetMetric(wxSYS_BORDER_X, w) * 2);
   r.height += wxSystemSettings::GetMetric(wxSYS_CAPTION_Y, w) +
      wxSystemSettings::GetMetric(wxSYS_BORDER_Y, w);
#endif
   if (!mBackground && mCaptureMode == kwindowplus )
   {
      // background colour not selected but we want a background
      wxRect b = GetBackgroundRect();
      r.x = (r.x - b.x) >= 0 ? (r.x - b.x): 0;
      r.y = (r.y - b.y) >= 0 ? (r.y - b.y): 0;
      r.width += b.width;
      r.height += b.height;
   }

   return r;
}

wxRect ScreenshotCommand::GetScreenRect(){
   int width, height;
   wxDisplaySize(&width, &height);

   return wxRect( 0,0,width,height);
}

wxRect ScreenshotCommand::GetPanelRect(TrackPanel * panel){
   //AdornedRulerPanel *ruler = panel->mRuler;

   int h = panel->GetRuler()->GetRulerHeight();
   int x = 0, y = -h;
   int width, height;

   panel->ClientToScreen(&x, &y);
   panel->GetParent()->ScreenToClient(&x, &y);
   panel->GetClientSize(&width, &height);
   return wxRect(x, y, width, height + h);
}

wxRect ScreenshotCommand::GetRulerRect(AdornedRulerPanel *ruler){
   int x = 0, y = 0;
   int width, height;

   ruler->ClientToScreen(&x, &y);
   ruler->GetParent()->ScreenToClient(&x, &y);
   ruler->GetClientSize(&width, &height);
   height = ruler->GetRulerHeight();
   return wxRect( x, y, width, height);
}

wxRect ScreenshotCommand::GetTracksRect(TrackPanel * panel){
   int x = 0, y = 0;
   int width, height;

   panel->ClientToScreen(&x, &y);
   panel->GetParent()->ScreenToClient(&x, &y);
   panel->GetClientSize(&width, &height);

   return wxRect( x, y, width, height);
}

wxRect ScreenshotCommand::GetTrackRect( SneedacityProject * pProj, TrackPanel * panel, int n){
   auto FindRectangle = []( TrackPanel &panel, Track &t )
   {
      // This rectangle omits the focus ring about the track, and
      // also within that, a narrow black border with a "shadow" below and
      // to the right
      wxRect rect = panel.FindTrackRect( &t );

      // Enlarge horizontally.
      // PRL:  perhaps it's one pixel too much each side, including some gray
      // beyond the yellow?
      rect.x = 0;
      panel.GetClientSize(&rect.width, nullptr);

      // Enlarge vertically, enough to enclose the yellow focus border pixels
      // Omit the outermost ring of gray pixels

      // (Note that TrackPanel paints its focus over the "top margin" of the
      // rectangle allotted to the track, according to TrackView::GetY() and
      // TrackView::GetHeight(), but also over the margin of the next track.)

      rect.height += kBottomMargin;
      int dy = kTopMargin - 1;
      rect.Inflate( 0, dy );

      // Reposition it relative to parent of panel
      rect.SetPosition(
         panel.GetParent()->ScreenToClient(
            panel.ClientToScreen(
               rect.GetPosition())));

      return rect;
   };

   int count = 0;
   for (auto t : TrackList::Get( *pProj ).Leaders()) {
      count +=  1;
      if( count > n )
      {
         wxRect r =  FindRectangle( *panel, *t );
         return r;
      }
   }
   return wxRect( 0,0,0,0);
}

wxString ScreenshotCommand::WindowFileName(SneedacityProject * proj, wxTopLevelWindow *w){
   if (w != ProjectWindow::Find( proj ) && !w->GetTitle().empty()) {
      mFileName = MakeFileName(mFilePath,
         kCaptureWhatStrings[ mCaptureMode ].Translation() +
            (wxT("-") + w->GetTitle() + wxT("-")));
   }
   return mFileName;
}

bool ScreenshotCommand::Apply(const CommandContext & context)
{
   GetDerivedParams();
   //Don't reset the toolbars to a known state.
   //We will be capturing variations of them.
   //ToolManager::Get( context.project ).Reset();

   wxTopLevelWindow *w = GetFrontWindow(&context.project);
   if (!w)
      return false;

   TrackPanel *panel = &TrackPanel::Get( context.project );
   AdornedRulerPanel *ruler = panel->GetRuler();

   int nTracks = TrackList::Get( context.project ).size();

   int x1,y1,x2,y2;
   w->ClientToScreen(&x1, &y1);
   panel->ClientToScreen(&x2, &y2);

   wxPoint p( x2-x1, y2-y1);

   auto &toolManager = ToolManager::Get( context.project );

   switch (mCaptureMode) {
   case kwindow:
      return Capture(context,  WindowFileName( &context.project, w ) , w, GetWindowRect(w));
   case kfullwindow:
   case kwindowplus:
      return Capture(context,  WindowFileName( &context.project, w ) , w, GetFullWindowRect(w));
   case kfullscreen:
      return Capture(context, mFileName, w,GetScreenRect());
   case ktoolbars:
      return CaptureDock(context, toolManager.GetTopDock(), mFileName);
   case kscriptables:
      CaptureScriptables(context, &context.project, mFileName);
      break;
   case keffects:
      CaptureEffects(context, &context.project, mFileName);
      break;
   case kpreferences:
      CapturePreferences(context, &context.project, mFileName);
      break;
   case kselectionbar:
      return CaptureToolbar(context, &toolManager, SelectionBarID, mFileName);
   case kspectralselection:
      return CaptureToolbar(context, &toolManager, SpectralSelectionBarID, mFileName);
   case ktimer:
      return CaptureToolbar(context, &toolManager, TimeBarID, mFileName);
   case ktools:
      return CaptureToolbar(context, &toolManager, ToolsBarID, mFileName);
   case ktransport:
      return CaptureToolbar(context, &toolManager, TransportBarID, mFileName);
   case kmixer:
      return CaptureToolbar(context, &toolManager, MixerBarID, mFileName);
   case kmeter:
      return CaptureToolbar(context, &toolManager, MeterBarID, mFileName);
   case krecordmeter:
      return CaptureToolbar(context, &toolManager, RecordMeterBarID, mFileName);
   case kplaymeter:
      return CaptureToolbar(context, &toolManager, PlayMeterBarID, mFileName);
   case kedit:
      return CaptureToolbar(context, &toolManager, EditBarID, mFileName);
   case kdevice:
      return CaptureToolbar(context, &toolManager, DeviceBarID, mFileName);
   case ktranscription:
      return CaptureToolbar(context, &toolManager, TranscriptionBarID, mFileName);
   case kscrub:
      return CaptureToolbar(context, &toolManager, ScrubbingBarID, mFileName);
   case ktrackpanel:
      return Capture(context, mFileName, panel, GetPanelRect(panel));
   case kruler:
      return Capture(context, mFileName, ruler, GetRulerRect(ruler) );
   case ktracks:
      return Capture(context, mFileName, panel, GetTracksRect(panel));
   case kfirsttrack:
      return Capture(context, mFileName, panel, GetTrackRect( &context.project, panel, 0 ) );
   case ksecondtrack:
      return Capture(context, mFileName, panel, GetTrackRect( &context.project, panel, 1 ) );
   case ktracksplus:
   {  wxRect r = GetTracksRect(panel);
      r.SetTop( r.GetTop() - ruler->GetRulerHeight() );
      r.SetHeight( r.GetHeight() + ruler->GetRulerHeight() );
      return Capture(context, mFileName, panel, r);
   }
   case kfirsttrackplus:
   {  wxRect r = GetTrackRect(&context.project, panel, 0 );
      r.SetTop( r.GetTop() - ruler->GetRulerHeight() );
      r.SetHeight( r.GetHeight() + ruler->GetRulerHeight() );
      return Capture(context, mFileName, panel, r );
   }
   case kfirsttwotracks:
   {  wxRect r = GetTrackRect( &context.project, panel, 0 );
      r = r.Union( GetTrackRect( &context.project, panel, 1 ));
      return Capture(context, mFileName, panel, r );
   }
   case kfirstthreetracks:
   {  wxRect r = GetTrackRect( &context.project, panel, 0 );
      r = r.Union( GetTrackRect( &context.project, panel, 2 ));
      return Capture(context, mFileName, panel, r );
   }
   case kfirstfourtracks:
   {  wxRect r = GetTrackRect( &context.project, panel, 0 );
      r = r.Union( GetTrackRect( &context.project, panel, 3 ));
      return Capture(context, mFileName, panel, r );
   }
   case kalltracks:
   {  wxRect r = GetTrackRect( &context.project, panel, 0 );
      r = r.Union( GetTrackRect( &context.project, panel, nTracks-1 ));
      return Capture(context, mFileName, panel, r );
   }
   case kalltracksplus:
   {  wxRect r = GetTrackRect( &context.project, panel, 0 );
      r.SetTop( r.GetTop() - ruler->GetRulerHeight() );
      r.SetHeight( r.GetHeight() + ruler->GetRulerHeight() );
      r = r.Union( GetTrackRect( &context.project, panel, nTracks-1 ));
      return Capture(context, mFileName, panel, r );
   }
   default:
      return false;
   }

   return true;
}
