/**********************************************************************

  Sneedacity: A Digital Audio Editor

  SneedacityApp.cpp

  Dominic Mazzoni

******************************************************************//**

\class SneedacityApp
\brief SneedacityApp is the 'main' class for Sneedacity

It handles initialization and termination by subclassing wxApp.

*//*******************************************************************/

#include "SneedacityApp.h"

#if 0
// This may be used to debug memory leaks.
// See: Visual Leak Detector @ http://vld.codeplex.com/
#include <vld.h>
#endif

#include <wx/setup.h> // for wxUSE_* macros
#include <wx/wxcrtvararg.h>
#include <wx/defs.h>
#include <wx/evtloop.h>
#include <wx/app.h>
#include <wx/bitmap.h>
#include <wx/docview.h>
#include <wx/event.h>
#include <wx/ipc.h>
#include <wx/window.h>
#include <wx/intl.h>
#include <wx/menu.h>
#include <wx/snglinst.h>
#include <wx/splash.h>
#include <wx/stdpaths.h>
#include <wx/sysopt.h>
#include <wx/fontmap.h>

#include <wx/fs_zip.h>
#include <wx/image.h>

#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>

#ifdef __WXGTK__
#include <unistd.h>
#ifdef HAVE_GTK
#include <gtk/gtk.h>
#endif
#endif

// chmod, lstat, geteuid
#ifdef __UNIX__
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#endif

#if defined(__WXMSW__)
#include <wx/msw/registry.h> // for wxRegKey
#endif

#include "SneedacityLogger.h"
#include "AboutDialog.h"
#include "AColor.h"
#include "SneedacityFileConfig.h"
#include "AudioIO.h"
#include "Benchmark.h"
#include "Clipboard.h"
#include "Catalog.h"
#include "CrashReport.h" // for HAS_CRASH_REPORT
#include "commands/CommandHandler.h"
#include "commands/AppCommandEvent.h"
#include "widgets/ASlider.h"
#include "FFmpeg.h"
//#include "LangChoice.h"
#include "Languages.h"
#include "Menus.h"
#include "PluginManager.h"
#include "Project.h"
#include "ProjectAudioIO.h"
#include "ProjectAudioManager.h"
#include "ProjectFileIO.h"
#include "ProjectFileManager.h"
#include "ProjectHistory.h"
#include "ProjectManager.h"
#include "ProjectSettings.h"
#include "ProjectWindow.h"
#include "Screenshot.h"
#include "Sequence.h"
#include "TempDirectory.h"
#include "Track.h"
#include "prefs/PrefsDialog.h"
#include "Theme.h"
#include "PlatformCompatibility.h"
#include "AutoRecoveryDialog.h"
#include "SplashDialog.h"
#include "FFT.h"
#include "widgets/SneedacityMessageBox.h"
#include "prefs/DirectoriesPrefs.h"
#include "prefs/GUIPrefs.h"
#include "tracks/ui/Scrubbing.h"
#include "widgets/FileConfig.h"
#include "widgets/FileHistory.h"
#include "Debug.h"


#ifdef EXPERIMENTAL_EASY_CHANGE_KEY_BINDINGS
#include "prefs/KeyConfigPrefs.h"
#endif

//temporarily commented out till it is added to all projects
//#include "Profiler.h"

#include "ModuleManager.h"

#include "import/Import.h"

#ifdef EXPERIMENTAL_SCOREALIGN
#include "effects/ScoreAlignDialog.h"
#endif

#if 0
#ifdef _DEBUG
    #ifdef _MSC_VER
        #undef THIS_FILE
        static char*THIS_FILE= __FILE__;
        #define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
    #endif
#endif
#endif

#include "../images/SneedacityLogoWithName.xpm"

#include <thread>


////////////////////////////////////////////////////////////
/// Custom events
////////////////////////////////////////////////////////////

#if 0
#ifdef __WXGTK__
static void wxOnAssert(const wxChar *fileName, int lineNumber, const wxChar *msg)
{
   if (msg)
      wxPrintf("ASSERTION FAILED: %s\n%s: %d\n", (const char *)wxString(msg).mb_str(), (const char *)wxString(fileName).mb_str(), lineNumber);
   else
      wxPrintf("ASSERTION FAILED!\n%s: %d\n", (const char *)wxString(fileName).mb_str(), lineNumber);

   // Force core dump
   int *i = 0;
   if (*i)
      exit(1);

   exit(0);
}
#endif
#endif

namespace {
#if _WIN32
#include <Windows.h>
    // In case it's not system theme is not defined, return 1 for light theme
    const char* getSystemTheme() {
        HKEY resultKey;
        LONG err = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &resultKey);
        if (err != ERROR_SUCCESS)
        {
            return "light";
        }
        DWORD dataType;
        WCHAR value[255];
        PVOID pvdata = value;
        DWORD size = sizeof(value);
        err = RegGetValue(resultKey, NULL, L"AppsUseLightTheme", RRF_RT_ANY, &dataType, pvdata, &size);
        if (err != ERROR_SUCCESS)
        {
            return "light";
        }

        if (*(DWORD*)pvdata == 0) {
            return "dark";
        }
        else {
            return "light";
        }
        RegCloseKey(resultKey);
    }
#endif // _WIN32
void PopulatePreferences()
{
   bool resetPrefs = false;
   wxString langCode = gPrefs->Read(wxT("/Locale/Language"), wxEmptyString);
   bool writeLang = false;

   const wxFileName fn(
      FileNames::ResourcesDir(), 
      wxT("FirstTime.ini"));
   if (fn.FileExists())   // it will exist if the (win) installer put it there
   {
      const wxString fullPath{fn.GetFullPath()};

      auto pIni =
         SneedacityFileConfig::Create({}, {}, fullPath, {},
            wxCONFIG_USE_LOCAL_FILE);
      auto &ini = *pIni;

      wxString lang;
      if (ini.Read(wxT("/FromInno/Language"), &lang) && !lang.empty())
      {
         // Only change "langCode" if the language was actually specified in the ini file.
         langCode = lang;
         writeLang = true;

         // Inno Setup doesn't allow special characters in the Name values, so "0" is used
         // to represent the "@" character.
         langCode.Replace(wxT("0"), wxT("@"));
      }

      ini.Read(wxT("/FromInno/ResetPrefs"), &resetPrefs, false);

      bool gone = wxRemoveFile(fullPath);  // remove FirstTime.ini
      if (!gone)
      {
         SneedacityMessageBox(
            XO("Failed to remove %s").Format(fullPath),
            XO("Failed!"));
      }
   }

   // Use the system default language if one wasn't specified or if the user selected System.
   if (langCode.empty())
      langCode =
         Languages::GetSystemLanguageCode(FileNames::SneedacityPathList());

   langCode = GUIPrefs::SetLang( langCode );

   // User requested that the preferences be completely reset
   if (resetPrefs)
   {
      // pop up a dialogue
      auto prompt = XO(
"Reset Preferences?\n\nThis is a one-time question, after an 'install' where you asked to have the Preferences reset.");
      int action = SneedacityMessageBox(
         prompt,
         XO("Reset Sneedacity Preferences"),
         wxYES_NO, NULL);
      if (action == wxYES)   // reset
      {
         gPrefs->DeleteAll();
         writeLang = true;
      }
   }

   // Save the specified language
   if (writeLang)
   {
      gPrefs->Write(wxT("/Locale/Language"), langCode);
   }

   // In AUdacity 2.1.0 support for the legacy 1.2.x preferences (depreciated since Sneedacity
   // 1.3.1) is dropped. As a result we can drop the import flag
   // first time this version of Sneedacity is run we try to migrate
   // old preferences.
   bool newPrefsInitialized = false;
   gPrefs->Read(wxT("/NewPrefsInitialized"), &newPrefsInitialized, false);
   if (newPrefsInitialized) {
      gPrefs->DeleteEntry(wxT("/NewPrefsInitialized"), true);  // take group as well if empty
   }

   // record the Prefs version for future checking (this has not been used for a very
   // long time).
   gPrefs->Write(wxT("/PrefsVersion"), wxString(wxT(SNEEDACITY_PREFS_VERSION_STRING)));

   // Check if some prefs updates need to happen based on sneedacity version.
   // Unfortunately we can't use the PrefsVersion prefs key because that resets things.
   // In the future we may want to integrate that better.
   // these are done on a case-by-case basis for now so they must be backwards compatible
   // (meaning the changes won't mess sneedacity up if the user goes back to an earlier version)
   int vMajor = gPrefs->Read(wxT("/Version/Major"), (long) 0);
   int vMinor = gPrefs->Read(wxT("/Version/Minor"), (long) 0);
   int vMicro = gPrefs->Read(wxT("/Version/Micro"), (long) 0);

   gPrefs->SetVersionKeysInit(vMajor, vMinor, vMicro);   // make a note of these initial values
                                                            // for use by ToolManager::ReadConfig()

   // These integer version keys were introduced april 4 2011 for 1.3.13
   // The device toolbar needs to be enabled due to removal of source selection features in
   // the mixer toolbar.
   if ((vMajor < 1) ||
       (vMajor == 1 && vMinor < 3) ||
       (vMajor == 1 && vMinor == 3 && vMicro < 13)) {


      // Do a full reset of the Device Toolbar to get it on the screen.
      if (gPrefs->Exists(wxT("/GUI/ToolBars/Device")))
         gPrefs->DeleteGroup(wxT("/GUI/ToolBars/Device"));

      // We keep the mixer toolbar prefs (shown/not shown)
      // the width of the mixer toolbar may have shrunk, the prefs will keep the larger value
      // if the user had a device that had more than one source.
      if (gPrefs->Exists(wxT("/GUI/ToolBars/Mixer"))) {
         // Use the default width
         gPrefs->Write(wxT("/GUI/ToolBars/Mixer/W"), -1);
      }
   }

   // In 2.1.0, the Meter toolbar was split and lengthened, but strange arrangements happen
   // if upgrading due to the extra length.  So, if a user is upgrading, use the pre-2.1.0
   // lengths, but still use the NEW split versions.
   if (gPrefs->Exists(wxT("/GUI/ToolBars/Meter")) &&
      !gPrefs->Exists(wxT("/GUI/ToolBars/CombinedMeter"))) {

      // Read in all of the existing values
      long dock, order, show, x, y, w, h;
      gPrefs->Read(wxT("/GUI/ToolBars/Meter/Dock"), &dock, -1);
      gPrefs->Read(wxT("/GUI/ToolBars/Meter/Order"), &order, -1);
      gPrefs->Read(wxT("/GUI/ToolBars/Meter/Show"), &show, -1);
      gPrefs->Read(wxT("/GUI/ToolBars/Meter/X"), &x, -1);
      gPrefs->Read(wxT("/GUI/ToolBars/Meter/Y"), &y, -1);
      gPrefs->Read(wxT("/GUI/ToolBars/Meter/W"), &w, -1);
      gPrefs->Read(wxT("/GUI/ToolBars/Meter/H"), &h, -1);

      // "Order" must be adjusted since we're inserting two NEW toolbars
      if (dock > 0) {
         wxString oldPath = gPrefs->GetPath();
         gPrefs->SetPath(wxT("/GUI/ToolBars"));

         wxString bar;
         long ndx = 0;
         bool cont = gPrefs->GetFirstGroup(bar, ndx);
         while (cont) {
            long o;
            if (gPrefs->Read(bar + wxT("/Order"), &o) && o >= order) {
               gPrefs->Write(bar + wxT("/Order"), o + 2);
            }
            cont = gPrefs->GetNextGroup(bar, ndx);
         }
         gPrefs->SetPath(oldPath);

         // And override the height
         h = 27;
      }

      // Write the split meter bar values
      gPrefs->Write(wxT("/GUI/ToolBars/RecordMeter/Dock"), dock);
      gPrefs->Write(wxT("/GUI/ToolBars/RecordMeter/Order"), order);
      gPrefs->Write(wxT("/GUI/ToolBars/RecordMeter/Show"), show);
      gPrefs->Write(wxT("/GUI/ToolBars/RecordMeter/X"), -1);
      gPrefs->Write(wxT("/GUI/ToolBars/RecordMeter/Y"), -1);
      gPrefs->Write(wxT("/GUI/ToolBars/RecordMeter/W"), w);
      gPrefs->Write(wxT("/GUI/ToolBars/RecordMeter/H"), h);
      gPrefs->Write(wxT("/GUI/ToolBars/PlayMeter/Dock"), dock);
      gPrefs->Write(wxT("/GUI/ToolBars/PlayMeter/Order"), order + 1);
      gPrefs->Write(wxT("/GUI/ToolBars/PlayMeter/Show"), show);
      gPrefs->Write(wxT("/GUI/ToolBars/PlayMeter/X"), -1);
      gPrefs->Write(wxT("/GUI/ToolBars/PlayMeter/Y"), -1);
      gPrefs->Write(wxT("/GUI/ToolBars/PlayMeter/W"), w);
      gPrefs->Write(wxT("/GUI/ToolBars/PlayMeter/H"), h);

      // And hide the old combined meter bar
      gPrefs->Write(wxT("/GUI/ToolBars/Meter/Dock"), -1);
   }

   // Upgrading pre 2.2.0 configs we assume extended set of defaults.
   if ((0<vMajor && vMajor < 2) ||
       (vMajor == 2 && vMinor < 2))
   {
      gPrefs->Write(wxT("/GUI/Shortcuts/FullDefaults"),1);
   }

   // Upgrading pre 2.4.0 configs, the selection toolbar is now split.
   if ((0<vMajor && vMajor < 2) ||
       (vMajor == 2 && vMinor < 4))
   {
      gPrefs->Write(wxT("/GUI/Toolbars/Selection/W"),"");
      gPrefs->Write(wxT("/GUI/Toolbars/SpectralSelection/W"),"");
      gPrefs->Write(wxT("/GUI/Toolbars/Time/X"),-1);
      gPrefs->Write(wxT("/GUI/Toolbars/Time/Y"),-1);
      gPrefs->Write(wxT("/GUI/Toolbars/Time/H"),55);
      gPrefs->Write(wxT("/GUI/Toolbars/Time/W"),251);
      gPrefs->Write(wxT("/GUI/Toolbars/Time/DockV2"),2);
      gPrefs->Write(wxT("/GUI/Toolbars/Time/Dock"),2);
      gPrefs->Write(wxT("/GUI/Toolbars/Time/Path"),"0,1");
      gPrefs->Write(wxT("/GUI/Toolbars/Time/Show"),1);
   }

#if _WIN32
   {
       auto theme = getSystemTheme();
       gPrefs->Write(wxT("/Gui/Theme"), theme);
   }
#endif // _WIN32

   // write out the version numbers to the prefs file for future checking
   gPrefs->Write(wxT("/Version/Major"), SNEEDACITY_VERSION);
   gPrefs->Write(wxT("/Version/Minor"), SNEEDACITY_RELEASE);
   gPrefs->Write(wxT("/Version/Micro"), SNEEDACITY_REVISION);

   gPrefs->Flush();
}
}

static bool gInited = false;
static bool gIsQuitting = false;

static void QuitSneedacity(bool bForce)
{
   // guard against recursion
   if (gIsQuitting)
      return;

   gIsQuitting = true;

   wxTheApp->SetExitOnFrameDelete(true);

   // Try to close each open window.  If the user hits Cancel
   // in a Save Changes dialog, don't continue.
   // BG: unless force is true

   // BG: Are there any projects open?
   //-   if (!AllProjects{}.empty())
/*start+*/
   if (AllProjects{}.empty())
   {
#ifdef __WXMAC__
      Clipboard::Get().Clear();
#endif
   }
   else
/*end+*/
   {
      if (AllProjects{}.size())
         // PRL:  Always did at least once before close might be vetoed
         // though I don't know why that is important
         ProjectManager::SaveWindowSize();
      bool closedAll = AllProjects::Close( bForce );
      if ( !closedAll )
      {
         gIsQuitting = false;
         return;
      }
   }

   ModuleManager::Get().Dispatch(AppQuiting);

#ifdef EXPERIMENTAL_SCOREALIGN
   CloseScoreAlignDialog();
#endif
   CloseScreenshotTools();

   //print out profile if we have one by deleting it
   //temporarily commented out till it is added to all projects
   //DELETE Profiler::Instance();

   // Save last log for diagnosis
   auto logger = SneedacityLogger::Get();
   if (logger)
   {
      wxFileName logFile(FileNames::DataDir(), wxT("lastlog.txt"));
      logger->SaveLog(logFile.GetFullPath());
   }

   //remove our logger
   std::unique_ptr<wxLog>{ wxLog::SetActiveTarget(NULL) }; // DELETE

   if (bForce)
   {
      wxExit();
   }
}

static void QuitSneedacity()
{
   QuitSneedacity(false);
}

#if defined(__WXGTK__) && defined(HAVE_GTK)

///////////////////////////////////////////////////////////////////////////////
// Provide the ability to receive notification from the session manager
// when the user is logging out or shutting down.
//
// Most of this was taken from nsNativeAppSupportUnix.cpp from Mozilla.
///////////////////////////////////////////////////////////////////////////////

// TODO: May need updating.  Is this code too obsolete (relying on Gnome2 so's) to be
// worth keeping anymore?
// CB suggests we use libSM directly ref:
// http://www.x.org/archive/X11R7.7/doc/libSM/SMlib.html#The_Save_Yourself_Callback

#include <dlfcn.h>
/* There is a conflict between the type names used in Glib >= 2.21 and those in
 * wxGTK (http://trac.wxwidgets.org/ticket/10883)
 * Happily we can avoid the hack, as we only need some of the headers, not
 * the full GTK headers
 */
#include <glib-object.h>

typedef struct _GnomeProgram GnomeProgram;
typedef struct _GnomeModuleInfo GnomeModuleInfo;
typedef struct _GnomeClient GnomeClient;

typedef enum
{
  GNOME_SAVE_GLOBAL,
  GNOME_SAVE_LOCAL,
  GNOME_SAVE_BOTH
} GnomeSaveStyle;

typedef enum
{
  GNOME_INTERACT_NONE,
  GNOME_INTERACT_ERRORS,
  GNOME_INTERACT_ANY
} GnomeInteractStyle;

typedef enum
{
  GNOME_DIALOG_ERROR,
  GNOME_DIALOG_NORMAL
} GnomeDialogType;

typedef GnomeProgram * (*_gnome_program_init_fn)(const char *,
                                                 const char *,
                                                 const GnomeModuleInfo *,
                                                 int,
                                                 char **,
                                                 const char *,
                                                 ...);
typedef const GnomeModuleInfo * (*_libgnomeui_module_info_get_fn)();
typedef GnomeClient * (*_gnome_master_client_fn)(void);
typedef void (*GnomeInteractFunction)(GnomeClient *,
                                      gint,
                                      GnomeDialogType,
                                      gpointer);
typedef void (*_gnome_client_request_interaction_fn)(GnomeClient *,
                                                     GnomeDialogType,
                                                     GnomeInteractFunction,
                                                     gpointer);
typedef void (*_gnome_interaction_key_return_fn)(gint, gboolean);

static _gnome_client_request_interaction_fn gnome_client_request_interaction;
static _gnome_interaction_key_return_fn gnome_interaction_key_return;

static void interact_cb(GnomeClient * /* client */,
                        gint key,
                        GnomeDialogType /* type */,
                        gpointer /* data */)
{
   wxCloseEvent e(wxEVT_QUERY_END_SESSION, wxID_ANY);
   e.SetEventObject(&wxGetApp());
   e.SetCanVeto(true);

   wxGetApp().ProcessEvent(e);

   gnome_interaction_key_return(key, e.GetVeto());
}

static gboolean save_yourself_cb(GnomeClient *client,
                                 gint /* phase */,
                                 GnomeSaveStyle /* style */,
                                 gboolean shutdown,
                                 GnomeInteractStyle interact,
                                 gboolean /* fast */,
                                 gpointer /* user_data */)
{
   if (!shutdown || interact != GNOME_INTERACT_ANY) {
      return TRUE;
   }

   if (AllProjects{}.empty()) {
      return TRUE;
   }

   gnome_client_request_interaction(client,
                                    GNOME_DIALOG_NORMAL,
                                    interact_cb,
                                    NULL);

   return TRUE;
}

class GnomeShutdown
{
 public:
   GnomeShutdown()
   {
      mArgv[0].reset(strdup("Sneedacity"));

      mGnomeui = dlopen("libgnomeui-2.so.0", RTLD_NOW);
      if (!mGnomeui) {
         return;
      }

      mGnome = dlopen("libgnome-2.so.0", RTLD_NOW);
      if (!mGnome) {
         return;
      }

      _gnome_program_init_fn gnome_program_init = (_gnome_program_init_fn)
         dlsym(mGnome, "gnome_program_init");
      _libgnomeui_module_info_get_fn libgnomeui_module_info_get = (_libgnomeui_module_info_get_fn)
         dlsym(mGnomeui, "libgnomeui_module_info_get");
      _gnome_master_client_fn gnome_master_client = (_gnome_master_client_fn)
         dlsym(mGnomeui, "gnome_master_client");

      gnome_client_request_interaction = (_gnome_client_request_interaction_fn)
         dlsym(mGnomeui, "gnome_client_request_interaction");
      gnome_interaction_key_return = (_gnome_interaction_key_return_fn)
         dlsym(mGnomeui, "gnome_interaction_key_return");


      if (!gnome_program_init || !libgnomeui_module_info_get) {
         return;
      }

      gnome_program_init(mArgv[0].get(),
                         "1.0",
                         libgnomeui_module_info_get(),
                         1,
                         reinterpret_cast<char**>(mArgv),
                         NULL);

      mClient = gnome_master_client();
      if (mClient == NULL) {
         return;
      }

      g_signal_connect(mClient, "save-yourself", G_CALLBACK(save_yourself_cb), NULL);
   }

   virtual ~GnomeShutdown()
   {
      // Do not dlclose() the libraries here lest you want segfaults...
   }

 private:

   MallocString<> mArgv[1];
   void *mGnomeui;
   void *mGnome;
   GnomeClient *mClient;
};

// This variable exists to call the constructor and
// connect a signal for the 'save-yourself' message.
GnomeShutdown GnomeShutdownInstance;

#endif

// Where drag/drop or "Open With" filenames get stored until
// the timer routine gets around to picking them up.
static wxArrayString ofqueue;

//
// DDE support for opening multiple files with one instance
// of Sneedacity.
//

#define IPC_APPL wxT("sneedacity")
#define IPC_TOPIC wxT("System")

class IPCConn final : public wxConnection
{
public:
   IPCConn()
   : wxConnection()
   {
   };

   ~IPCConn()
   {
   };

   bool OnExec(const wxString & WXUNUSED(topic),
               const wxString & data)
   {
      // Add the filename to the queue.  It will be opened by
      // the OnTimer() event when it is safe to do so.
      ofqueue.push_back(data);

      return true;
   }
};

class IPCServ final : public wxServer
{
public:
   IPCServ(const wxString & appl)
   : wxServer()
   {
      Create(appl);
   };

   ~IPCServ()
   {
   };

   wxConnectionBase *OnAcceptConnection(const wxString & topic) override
   {
      if (topic != IPC_TOPIC) {
         return NULL;
      }

      // Trust wxWidgets framework to DELETE it
      return safenew IPCConn();
   };
};

#if defined(__WXMAC__)

IMPLEMENT_APP_NO_MAIN(SneedacityApp)
IMPLEMENT_WX_THEME_SUPPORT

int main(int argc, char *argv[])
{
   dprintf("main()");

   global_debug_prints_enabled = false;

   wxDISABLE_DEBUG_SUPPORT();

   // Proceed to SneedacityApp::OnInit() and then to SneedacityApp::InitPart2()
   return wxEntry(argc, argv);
}

#elif defined(__WXGTK__) && defined(NDEBUG)

IMPLEMENT_APP_NO_MAIN(SneedacityApp)
IMPLEMENT_WX_THEME_SUPPORT

int main(int argc, char *argv[])
{
   wxDISABLE_DEBUG_SUPPORT();

   // Bug #1986 workaround - This doesn't actually reduce the number of 
   // messages, it simply hides them in Release builds. We'll probably
   // never be able to get rid of the messages entirely, but we should
   // look into what's causing them, so allow them to show in Debug
   // builds.
   stdout = freopen("/dev/null", "w", stdout);
   stderr = freopen("/dev/null", "w", stderr);

   return wxEntry(argc, argv);
}

#else
IMPLEMENT_APP(SneedacityApp)
#endif

#ifdef __WXMAC__

// in response of an open-document apple event
void SneedacityApp::MacOpenFile(const wxString &fileName)
{
   ofqueue.push_back(fileName);
}

// in response of a print-document apple event
void SneedacityApp::MacPrintFile(const wxString &fileName)
{
   ofqueue.push_back(fileName);
}

// in response of a open-application apple event
void SneedacityApp::MacNewFile()
{
   if (!gInited)
      return;

   // This method should only be used on the Mac platform
   // when no project windows are open.

   if (AllProjects{}.empty())
      (void) ProjectManager::New();
}

#endif //__WXMAC__

// IPC communication
#define ID_IPC_SERVER   6200
#define ID_IPC_SOCKET   6201

// we don't really care about the timer id, but set this value just in case we do in the future
#define kSneedacityAppTimerID 0

BEGIN_EVENT_TABLE(SneedacityApp, wxApp)
   EVT_QUERY_END_SESSION(SneedacityApp::OnQueryEndSession)
   EVT_END_SESSION(SneedacityApp::OnEndSession)

   EVT_TIMER(kSneedacityAppTimerID, SneedacityApp::OnTimer)
#ifdef __WXMAC__
   EVT_MENU(wxID_NEW, SneedacityApp::OnMenuNew)
   EVT_MENU(wxID_OPEN, SneedacityApp::OnMenuOpen)
   EVT_MENU(wxID_ABOUT, SneedacityApp::OnMenuAbout)
   EVT_MENU(wxID_PREFERENCES, SneedacityApp::OnMenuPreferences)
#endif

   // Associate the handler with the menu id on all operating systems, even
   // if they don't have an application menu bar like in macOS, so that
   // other parts of the program can send the application a shut-down
   // event
   EVT_MENU(wxID_EXIT, SneedacityApp::OnMenuExit)

#ifndef __WXMSW__
   EVT_SOCKET(ID_IPC_SERVER, SneedacityApp::OnServerEvent)
   EVT_SOCKET(ID_IPC_SOCKET, SneedacityApp::OnSocketEvent)
#endif

   // Recent file event handlers.
   EVT_MENU(FileHistory::ID_RECENT_CLEAR, SneedacityApp::OnMRUClear)
   EVT_MENU_RANGE(FileHistory::ID_RECENT_FIRST, FileHistory::ID_RECENT_LAST,
      SneedacityApp::OnMRUFile)

   // Handle AppCommandEvents (usually from a script)
   EVT_APP_COMMAND(wxID_ANY, SneedacityApp::OnReceiveCommand)

   // Global ESC key handling
   EVT_KEY_DOWN(SneedacityApp::OnKeyDown)
   EVT_KEY_DOWN(CatalogListener::CheckForSneed)
END_EVENT_TABLE()

// backend for OnMRUFile
// TODO: Would be nice to make this handle not opening a file with more panache.
//  - Inform the user if DefaultOpenPath not set.
//  - Switch focus to correct instance of project window, if already open.
bool SneedacityApp::MRUOpen(const FilePath &fullPathStr) {
   // Most of the checks below are copied from ProjectManager::OpenFiles.
   // - some rationalisation might be possible.

   SneedacityProject *proj = GetActiveProject();

   if (!fullPathStr.empty())
   {
      // verify that the file exists
      if (wxFile::Exists(fullPathStr))
      {
         FileNames::UpdateDefaultPath(FileNames::Operation::Open, ::wxPathOnly(fullPathStr));

         // Make sure it isn't already open.
         // Test here even though SneedacityProject::OpenFile() also now checks, because
         // that method does not return the bad result.
         // That itself may be a FIXME.
         if (ProjectFileManager::IsAlreadyOpen(fullPathStr))
            return false;

         ( void ) ProjectManager::OpenProject( proj, fullPathStr,
               true /* addtohistory */, false /* reuseNonemptyProject */ );
      }
      else {
         // File doesn't exist - remove file from history
         SneedacityMessageBox(
            XO(
"%s could not be found.\n\nIt has been removed from the list of recent files.")
               .Format(fullPathStr) );
         return(false);
      }
   }
   return(true);
}

bool SneedacityApp::SafeMRUOpen(const wxString &fullPathStr)
{
   return GuardedCall< bool >( [&]{ return MRUOpen( fullPathStr ); } );
}

void SneedacityApp::OnMRUClear(wxCommandEvent& WXUNUSED(event))
{
   FileHistory::Global().Clear();
}

//vvv Basically, anything from Recent Files is treated as a .aup3, until proven otherwise,
// then it tries to Import(). Very questionable handling, imo.
// Better, for example, to check the file type early on.
void SneedacityApp::OnMRUFile(wxCommandEvent& event) {
   int n = event.GetId() - FileHistory::ID_RECENT_FIRST;
   auto &history = FileHistory::Global();
   const auto &fullPathStr = history[ n ];

   // Try to open only if not already open.
   // Test IsAlreadyOpen() here even though SneedacityProject::MRUOpen() also now checks,
   // because we don't want to Remove() just because it already exists,
   // and SneedacityApp::OnMacOpenFile() calls MRUOpen() directly.
   // that method does not return the bad result.
   // PRL: Don't call SafeMRUOpen
   // -- if open fails for some exceptional reason of resource exhaustion that
   // the user can correct, leave the file in history.
   if (!ProjectFileManager::IsAlreadyOpen(fullPathStr) && !MRUOpen(fullPathStr))
      history.Remove(n);
}

void SneedacityApp::OnTimer(wxTimerEvent& WXUNUSED(event))
{
   // Filenames are queued when Sneedacity receives a few of the
   // AppleEvent messages (via wxWidgets).  So, open any that are
   // in the queue and clean the queue.
   if (gInited) {
      if (ofqueue.size()) {
         // Load each file on the queue
         while (ofqueue.size()) {
            wxString name;
            name.swap(ofqueue[0]);
            ofqueue.erase( ofqueue.begin() );

            // Get the user's attention if no file name was specified
            if (name.empty()) {
               // Get the users attention
               SneedacityProject *project = GetActiveProject();
               if (project) {
                  auto &window = GetProjectFrame( *project );
                  window.Maximize();
                  window.Raise();
                  window.RequestUserAttention();
               }
               continue;
            }

            // TODO: Handle failures better.
            // Some failures are OK, e.g. file not found, just would-be-nices to do better,
            // so FAIL_MSG is more a case of an enhancement request than an actual  problem.
            // LL:  In all but one case an appropriate message is already displayed.  The
            //      instance that a message is NOT displayed is when a failure to write
            //      to the config file has occurred.
            // PRL: Catch any exceptions, don't try this file again, continue to
            // other files.
            if (!SafeMRUOpen(name)) {
               // Just log it.  Assertion failure is not appropriate on valid
               // defensive path against bad file data.
               wxLogMessage(wxT("MRUOpen failed"));
            }
         }
      }
   }
}

#if defined(__WXMSW__)
#define WL(lang, sublang) (lang), (sublang),
#else
#define WL(lang,sublang)
#endif

#if wxCHECK_VERSION(3, 0, 1)
wxLanguageInfo userLangs[] =
{
   // Bosnian is defined in wxWidgets already
//   { wxLANGUAGE_USER_DEFINED, wxT("bs"), WL(0, SUBLANG_DEFAULT) wxT("Bosnian"), wxLayout_LeftToRight },

   { wxLANGUAGE_USER_DEFINED, wxT("eu"), WL(0, SUBLANG_DEFAULT) wxT("Basque"), wxLayout_LeftToRight },
};
#endif

void SneedacityApp::OnFatalException()
{
#if defined(HAS_CRASH_REPORT)
   CrashReport::Generate(wxDebugReport::Context_Exception);
#endif

   exit(-1);
}


#ifdef _MSC_VER
// If this is compiled with MSVC (Visual Studio)
#pragma warning( push )
#pragma warning( disable : 4702) // unreachable code warning.
#endif //_MSC_VER

bool SneedacityApp::OnExceptionInMainLoop()
{
   // This function is invoked from catch blocks in the wxWidgets framework,
   // and throw; without argument re-throws the exception being handled,
   // letting us dispatch according to its type.

   try { throw; }
   catch ( SneedacityException &e ) {
      (void)e;// Compiler food
      // Here is the catch-all for our own exceptions

      // Use CallAfter to delay this to the next pass of the event loop,
      // rather than risk doing it inside stack unwinding.
      auto pProject = ::GetActiveProject();
      auto pException = std::current_exception();
      CallAfter( [=]      // Capture pException by value!
      {

         // Restore the state of the project to what it was before the
         // failed operation
         if (pProject) {
            ProjectHistory::Get( *pProject ).RollbackState();

            // Forget pending changes in the TrackList
            TrackList::Get( *pProject ).ClearPendingTracks();

            ProjectWindow::Get( *pProject ).RedrawProject();
         }

         // Give the user an alert
         try { std::rethrow_exception( pException ); }
         catch( SneedacityException &e )
            { e.DelayedHandlerAction(); }

      } );

      // Don't quit the program
      return true;
   }
   catch ( ... ) {
      // There was some other type of exception we don't know.
      // Let the inherited function do throw; again and whatever else it does.
      return wxApp::OnExceptionInMainLoop();
   }
   // Shouldn't ever reach this line
   return false;
}
#ifdef _MSC_VER
#pragma warning( pop )
#endif //_MSC_VER

SneedacityApp::SneedacityApp()
{
#if !defined(_DEBUG)
#if defined(HAS_CRASH_REPORT)
#if defined(wxUSE_ON_FATAL_EXCEPTION) && wxUSE_ON_FATAL_EXCEPTION
   wxHandleFatalExceptions();
#endif
#endif
#endif
}

SneedacityApp::~SneedacityApp()
{
}

// The `main program' equivalent, creating the windows and returning the
// main frame
bool SneedacityApp::OnInit()
{
   dprintf("SneedacityApp::OnInit()");
   // JKC: ANSWER-ME: Who actually added the event loop guarantor?
   // Although 'blame' says Leland, I think it came from a donated patch.

   // PRL:  It was added by LL at 54676a72285ba7ee3a69920e91fa390a71ef10c9 :
   // "   Ensure OnInit() has an event loop
   //     And allow events to flow so the splash window updates under GTK"
   // then mistakenly lost in the merge at
   // 37168ebbf67ae869ab71a3b5cbbf1d2a48e824aa
   // then restored at 7687972aa4b2199f0717165235f3ef68ade71e08

   // Ensure we have an event loop during initialization
   wxEventLoopGuarantor eventLoop;

   // Fire up SQLite
   if ( !ProjectFileIO::InitializeSQL() )
      this->CallAfter([]{
         ::SneedacityMessageBox(
            XO("SQLite library failed to initialize.  Sneedacity cannot continue.") );
         QuitSneedacity( true );
      });


   // cause initialization of wxWidgets' global logger target
   (void) SneedacityLogger::Get();

#if defined(__WXMAC__)
   // Disable window animation
   wxSystemOptions::SetOption(wxMAC_WINDOW_PLAIN_TRANSITION, 1);
#endif

   // Some GTK themes produce larger combo boxes that make them taller
   // than our single toolbar height restriction.  This will remove some
   // of the extra space themes add.
#if defined(__WXGTK3__) && defined(HAVE_GTK)
   GtkWidget *combo = gtk_combo_box_new();
   GtkCssProvider *provider = gtk_css_provider_new();
   gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider),
                                   ".linked entry,\n"
                                   ".linked button,\n"
                                   ".linked combobox box.linked button,\n"
                                   ".horizontal.linked entry,\n"
                                   ".horizontal.linked button,\n"
                                   ".horizontal.linked combobox box.linked button,\n"
                                   "combobox {\n"
                                   "   padding-top: 0px;\n"
                                   "   padding-bottom: 0px;\n"
                                   "   padding-left: 4px;\n"
                                   "   padding-right: 4px;\n"
                                   "   margin: 0px;\n"
                                   "   font-size: 95%;\n"
                                   "}", -1, NULL);
   gtk_style_context_add_provider_for_screen(gtk_widget_get_screen(combo),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   g_object_unref(provider);
   g_object_unref(combo);
#elif defined(__WXGTK__) && defined(HAVE_GTK)
   gtk_rc_parse_string("style \"sneedacity\" {\n"
                       " GtkButton::inner_border = { 0, 0, 0, 0 }\n"
                       " GtkEntry::inner_border = { 0, 0, 0, 0 }\n"
                       " xthickness = 4\n"
                       " ythickness = 0\n"
                       "}\n"
                       "widget_class \"*GtkCombo*\" style \"sneedacity\"");
#endif

   // Don't use SNEEDACITY_NAME here.
   // We want Sneedacity with a capital 'A'

   wxString appName = wxT("Sneedacity");

   wxTheApp->SetAppName(appName);
   // Explicitly set since OSX will use it for the "Quit" menu item
   wxTheApp->SetAppDisplayName(appName);
   wxTheApp->SetVendorName(appName);

   ::wxInitAllImageHandlers();

   // AddHandler takes ownership
   wxFileSystem::AddHandler(safenew wxZipFSHandler);
   
   // by wxwidgets standard 
   wxStandardPaths::Get().SetFileLayout(wxStandardPaths::FileLayout::FileLayout_XDG);
   //
   // Paths: set search path and temp dir path
   //
   FilePaths sneedacityPathList;

#ifdef __WXGTK__
   // Make sure install prefix is set so wxStandardPath resolves paths properly
   wxStandardPaths::Get().SetInstallPrefix(wxT(INSTALL_PREFIX));

   /* Search path (for plug-ins, translations etc) is (in this order):
      * The SNEEDACITY_PATH environment variable
      * The current directory
      * The user's "~/.sneedacity-data" or "Portable Settings" directory
      * The user's "~/.sneedacity-files" directory
      * The "share" and "share/doc" directories in their install path */
   wxString home = wxGetHomeDir();

   wxString envTempDir = wxGetenv(wxT("TMPDIR"));
   if (!envTempDir.empty()) {
      /* On Unix systems, the environment variable TMPDIR may point to
         an unusual path when /tmp and /var/tmp are not desirable. */
      TempDirectory::SetDefaultTempDir( wxString::Format(
         wxT("%s/sneedacity-%s"), envTempDir, wxGetUserId() ) );
   } else {
      /* On Unix systems, the default temp dir is in /var/tmp. */
      TempDirectory::SetDefaultTempDir( wxString::Format(
         wxT("/var/tmp/sneedacity-%s"), wxGetUserId() ) );
   }

   wxString pathVar = wxGetenv(wxT("SNEEDACITY_PATH"));
   if (!pathVar.empty())
      FileNames::AddMultiPathsToPathList(pathVar, sneedacityPathList);
   FileNames::AddUniquePathToPathList(::wxGetCwd(), sneedacityPathList);

   wxString progPath = wxPathOnly(argv[0]);
   FileNames::AddUniquePathToPathList(progPath, sneedacityPathList);
   // Add the path to modules:
   FileNames::AddUniquePathToPathList(progPath + L"/lib/sneedacity", sneedacityPathList);

   FileNames::AddUniquePathToPathList(FileNames::DataDir(), sneedacityPathList);

#ifdef SNEEDACITY_NAME
   FileNames::AddUniquePathToPathList(wxString::Format(wxT("%s/.%s-files"),
      home, wxT(SNEEDACITY_NAME)),
      sneedacityPathList);
   FileNames::AddUniquePathToPathList(FileNames::ModulesDir(),
      sneedacityPathList);
   FileNames::AddUniquePathToPathList(wxString::Format(wxT("%s/share/%s"),
      wxT(INSTALL_PREFIX), wxT(SNEEDACITY_NAME)),
      sneedacityPathList);
   FileNames::AddUniquePathToPathList(wxString::Format(wxT("%s/share/doc/%s"),
      wxT(INSTALL_PREFIX), wxT(SNEEDACITY_NAME)),
      sneedacityPathList);
#else //SNEEDACITY_NAME
   FileNames::AddUniquePathToPathList(wxString::Format(wxT("%s/.sneedacity-files"),
      home),
      sneedacityPathList)
   FileNames::AddUniquePathToPathList(FileNames::ModulesDir(),
      sneedacityPathList);
   FileNames::AddUniquePathToPathList(wxString::Format(wxT("%s/share/sneedacity"),
      wxT(INSTALL_PREFIX)),
      sneedacityPathList);
   FileNames::AddUniquePathToPathList(wxString::Format(wxT("%s/share/doc/sneedacity"),
      wxT(INSTALL_PREFIX)),
      sneedacityPathList);
#endif //SNEEDACITY_NAME

   FileNames::AddUniquePathToPathList(wxString::Format(wxT("%s/share/locale"),
      wxT(INSTALL_PREFIX)),
      sneedacityPathList);

   FileNames::AddUniquePathToPathList(wxString::Format(wxT("./locale")),
      sneedacityPathList);

#endif //__WXGTK__

// JKC Bug 1220: Use path based on home directory on WXMAC
#ifdef __WXMAC__
   wxFileName tmpFile;
   tmpFile.AssignHomeDir();
   wxString tmpDirLoc = tmpFile.GetPath(wxPATH_GET_VOLUME);
#else
   wxFileName tmpFile;
   tmpFile.AssignTempFileName(wxT("nn"));
   wxString tmpDirLoc = tmpFile.GetPath(wxPATH_GET_VOLUME);
   ::wxRemoveFile(tmpFile.GetFullPath());
#endif



   // On Mac and Windows systems, use the directory which contains Sneedacity.
#ifdef __WXMSW__
   // On Windows, the path to the Sneedacity program is in argv[0]
   wxString progPath = wxPathOnly(argv[0]);
   FileNames::AddUniquePathToPathList(progPath, sneedacityPathList);
   FileNames::AddUniquePathToPathList(progPath + wxT("\\Languages"), sneedacityPathList);

   // See bug #1271 for explanation of location
   tmpDirLoc = FileNames::MkDir(wxStandardPaths::Get().GetUserLocalDataDir());
   TempDirectory::SetDefaultTempDir( wxString::Format(
      wxT("%s\\SessionData"), tmpDirLoc ) );
#endif //__WXWSW__

#ifdef __WXMAC__
   // On Mac OS X, the path to the Sneedacity program is in argv[0]
   wxString progPath = wxPathOnly(argv[0]);

   FileNames::AddUniquePathToPathList(progPath, sneedacityPathList);
   // If Sneedacity is a "bundle" package, then the root directory is
   // the great-great-grandparent of the directory containing the executable.
   //FileNames::AddUniquePathToPathList(progPath + wxT("/../../../"), sneedacityPathList);

   // These allow for searching the "bundle"
   FileNames::AddUniquePathToPathList(
      progPath + wxT("/../"), sneedacityPathList);
   FileNames::AddUniquePathToPathList(
      progPath + wxT("/../Resources"), sneedacityPathList);

   // JKC Bug 1220: Using an actual temp directory for session data on Mac was
   // wrong because it would get cleared out on a reboot.
   TempDirectory::SetDefaultTempDir( wxString::Format(
      wxT("%s/Library/Application Support/sneedacity/SessionData"), tmpDirLoc) );

   //TempDirectory::SetDefaultTempDir( wxString::Format(
   //   wxT("%s/sneedacity-%s"),
   //   tmpDirLoc,
   //   wxGetUserId() ) );
#endif //__WXMAC__

   FileNames::SetSneedacityPathList( std::move( sneedacityPathList ) );

   // Define languages for which we have translations, but that are not yet
   // supported by wxWidgets.
   //
   // TODO:  The whole Language initialization really need to be reworked.
   //        It's all over the place.
#if wxCHECK_VERSION(3, 0, 1)
   for (size_t i = 0, cnt = WXSIZEOF(userLangs); i < cnt; i++)
   {
      wxLocale::AddLanguage(userLangs[i]);
   }
#endif

   // Initialize preferences and language
   {
      dprintf("Initialize preferences and language");
      wxFileName configFileName(FileNames::ConfigDir(), wxT("sneedacity.cfg"));
      auto appName = wxTheApp->GetAppName();
      InitPreferences( SneedacityFileConfig::Create(
         appName, wxEmptyString,
         configFileName.GetFullPath(),
         wxEmptyString, wxCONFIG_USE_LOCAL_FILE) );
      PopulatePreferences();
      dprintf("Initialize preferences and language: done");
   }

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__) && !defined(__CYGWIN__)
   this->AssociateFileTypes();
#endif

   theTheme.EnsureInitialised();

   // AColor depends on theTheme.
   AColor::Init();

   // If this fails, we must exit the program.
   if (!InitTempDir()) {
      FinishPreferences();
      return false;
   }

#ifdef __WXMAC__
   // Bug2437:  When files are opened from Finder and another instance of
   // Sneedacity is running, we must return from OnInit() to wxWidgets before
   // MacOpenFile is called, informing us of the paths that need to be
   // opened.  So use CallAfter() to delay the rest of initialization.
   // See CreateSingleInstanceChecker() where we send those paths over a
   // socket to the prior instance.

   // This call is what probably makes the sleep unnecessary:
   MacFinishLaunching();

   using namespace std::chrono;
   // This sleep may be unnecessary, but it is harmless.  It less NS framework
   // events arrive on another thread, but it might not always be long enough.
   std::this_thread::sleep_for(100ms);
   CallAfter([this]{
      if (!InitPart2())
         exit(-1);
   });
   return true;
#else
   return InitPart2();
#endif
}

bool SneedacityApp::InitPart2()
{
   dprintf("SneedacityApp::InitPart2()");
#if defined(__WXMAC__)
   SetExitOnFrameDelete(false);
#endif

   // Make sure the temp dir isn't locked by another process.
   {
      auto key =
         PreferenceKey(FileNames::Operation::Temp, FileNames::PathType::_None);
      auto temp = gPrefs->Read(key);
      if (temp.empty() || !CreateSingleInstanceChecker(temp)) {
         FinishPreferences();
         return false;
      }
   }

   //<<<< Try to avoid dialogs before this point.
   // The reason is that InitTempDir starts the single instance checker.
   // If we're waiitng in a dialog before then we can very easily
   // start multiple instances, defeating the single instance checker.

   // Initialize the CommandHandler
   dprintf("Initialize the CommandHandler");
   InitCommandHandler();

   // Initialize the ModuleManager, including loading found modules
   dprintf("Initialize the ModuleManager, including loading found modules");
   ModuleManager::Get().Initialize();

   // Initialize the PluginManager
   dprintf("Initialize the PluginManager");
   PluginManager::Get().Initialize();

   // Parse command line and handle options that might require
   // immediate exit...no need to initialize all of the audio
   // stuff to display the version string.
   std::shared_ptr< wxCmdLineParser > parser{ ParseCommandLine().release() };
   if (!parser)
   {
      // Either user requested help or a parsing error occurred
      exit(1);
   }

   if (parser->Found(wxT("v")))
   {
      wxPrintf("Sneedacity v%s\n", SNEEDACITY_VERSION_STRING);
      exit(0);
   }

   if (parser->Found(wxT("d")))
   {
      global_debug_prints_enabled = true;
   }
   dprintf("InitPart2: global_debug_prints_enabled =", global_debug_prints_enabled);


   long lval;
   if (parser->Found(wxT("b"), &lval))
   {
      if (lval < 256 || lval > 100000000)
      {
         wxPrintf(_("Block size must be within 256 to 100000000\n"));
         exit(1);
      }

      Sequence::SetMaxDiskBlockSize(lval);
   }

   // BG: Create a temporary window to set as the top window
   wxImage logoimage((const char **)SneedacityLogoWithName_xpm);
   logoimage.Rescale(logoimage.GetWidth() / 2, logoimage.GetHeight() / 2);
   if( GetLayoutDirection() == wxLayout_RightToLeft)
      logoimage = logoimage.Mirror();
   wxBitmap logo(logoimage);

   SneedacityProject *project;
   {
      dprintf("SneedacityProject *project;");
      // Bug 718: Position splash screen on same screen 
      // as where Sneedacity project will appear.
      wxRect wndRect;
      bool bMaximized = false;
      bool bIconized = false;
      GetNextWindowPlacement(&wndRect, &bMaximized, &bIconized);

      wxSplashScreen temporarywindow(
         logo,
         wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT,
         0,
         NULL,
         wxID_ANY,
         wndRect.GetTopLeft(),
         wxDefaultSize,
         wxSTAY_ON_TOP);

      // Unfortunately with the Windows 10 Creators update, the splash screen 
      // now appears before setting its position.
      // On a dual monitor screen it will appear on one screen and then 
      // possibly jump to the second.
      // We could fix this by writing our own splash screen and using Hide() 
      // until the splash scren was correctly positioned, then Show()

      // Possibly move it on to the second screen...
      temporarywindow.SetPosition( wndRect.GetTopLeft() );
      // Centered on whichever screen it is on.
      temporarywindow.Center();
      temporarywindow.SetTitle(_("Sneedacity is starting up..."));
      SetTopWindow(&temporarywindow);
      temporarywindow.Raise();

      // ANSWER-ME: Why is YieldFor needed at all?
      //wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_UI|wxEVT_CATEGORY_USER_INPUT|wxEVT_CATEGORY_UNKNOWN);
      wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_UI);

      //JKC: Would like to put module loading here.

      #ifdef USE_FFMPEG
      dprintf("SneedacityApp.cpp: Calling FFmpegStartup()");
      FFmpegStartup();
      dprintf("SneedacityApp.cpp: FFmpegStartup() returned");
      #endif

      // More initialization
      dprintf("More initialization");
      InitDitherers();
      dprintf("AudioIO::Init();");
      AudioIO::Init();
      dprintf("AudioIO::Init(); done");

#ifdef __WXMAC__

      // On the Mac, users don't expect a program to quit when you close the last window.
      // Create a menubar that will show when all project windows are closed.

      auto fileMenu = std::make_unique<wxMenu>();
      auto urecentMenu = std::make_unique<wxMenu>();
      auto recentMenu = urecentMenu.get();
      fileMenu->Append(wxID_NEW, wxString(_("&New")) + wxT("\tCtrl+N"));
      fileMenu->Append(wxID_OPEN, wxString(_("&Open...")) + wxT("\tCtrl+O"));
      fileMenu->AppendSubMenu(urecentMenu.release(), _("Open &Recent..."));
      fileMenu->Append(wxID_ABOUT, _("&About Sneedacity..."));
      fileMenu->Append(wxID_PREFERENCES, wxString(_("&Preferences...")) + wxT("\tCtrl+,"));

      {
         auto menuBar = std::make_unique<wxMenuBar>();
         menuBar->Append(fileMenu.release(), _("&File"));

         // PRL:  Are we sure wxWindows will not leak this menuBar?
         // The online documentation is not explicit.
         wxMenuBar::MacSetCommonMenuBar(menuBar.release());
      }

      auto &recentFiles = FileHistory::Global();
      recentFiles.UseMenu(recentMenu);

#endif //__WXMAC__
      temporarywindow.Show(false);
   }

   // Workaround Bug 1377 - Crash after Sneedacity starts and low disk space warning appears
   // The temporary splash window is closed AND cleaned up, before attempting to create
   // a project and possibly creating a modal warning dialog by doing so.
   // Also fixes problem of warning being obscured.
   // Downside is that we have no splash screen for the (brief) time that we spend
   // creating the project.
   // Root cause is problem with wxSplashScreen and other dialogs co-existing, that
   // seemed to arrive with wx3.
   {
      project = ProjectManager::New();
   }

   if( ProjectSettings::Get( *project ).GetShowSplashScreen() ){
      // This may do a check-for-updates at every start up.
      // Mainly this is to tell users of ALPHAS who don't know that they have an ALPHA.
      // Disabled for now, after discussion.
      // project->MayCheckForUpdates();
      SplashDialog::DoHelpWelcome(*project);
   }

   Importer::Get().Initialize();

   // Bug1561: delay the recovery dialog, to avoid crashes.
   CallAfter( [=] () mutable {
      // Remove duplicate shortcuts when there's a change of version
      int vMajorInit, vMinorInit, vMicroInit;
      gPrefs->GetVersionKeysInit(vMajorInit, vMinorInit, vMicroInit);
      if (vMajorInit != SNEEDACITY_VERSION || vMinorInit != SNEEDACITY_RELEASE
         || vMicroInit != SNEEDACITY_REVISION) {
         CommandManager::Get(*project).RemoveDuplicateShortcuts();
      }
      //
      // Auto-recovery
      //
      bool didRecoverAnything = false;
      // This call may reassign project (passed by reference)
      if (!ShowAutoRecoveryDialogIfNeeded(project, &didRecoverAnything))
      {
         QuitSneedacity(true);
      }

      //
      // Remainder of command line parsing, but only if we didn't recover
      //
      if (project && !didRecoverAnything)
      {
         if (parser->Found(wxT("t")))
         {
            RunBenchmark( nullptr, *project);
            QuitSneedacity(true);
         }

         for (size_t i = 0, cnt = parser->GetParamCount(); i < cnt; i++)
         {
            // PRL: Catch any exceptions, don't try this file again, continue to
            // other files.
            SafeMRUOpen(parser->GetParam(i));
         }
      }
   } );

   gInited = true;

   ModuleManager::Get().Dispatch(AppInitialized);

   mTimer.SetOwner(this, kSneedacityAppTimerID);
   mTimer.Start(200);

#ifdef EXPERIMENTAL_EASY_CHANGE_KEY_BINDINGS
   CommandManager::SetMenuHook( [](const CommandID &id){
      if (::wxGetMouseState().ShiftDown()) {
         // Only want one page of the preferences
         PrefsPanel::Factories factories;
         factories.push_back(KeyConfigPrefsFactory( id ));
         const auto pProject = GetActiveProject();
         auto pWindow = FindProjectFrame( pProject );
         GlobalPrefsDialog dialog( pWindow, pProject, factories );
         dialog.ShowModal();
         MenuCreator::RebuildAllMenuBars();
         return true;
      }
      else
         return false;
   } );
#endif

#if defined(__WXMAC__)
   // The first time this version of Sneedacity is run or when the preferences
   // are reset, execute the "tccutil" command to reset the microphone permissions
   // currently assigned to Sneedacity.  The end result is that the user will be
   // prompted to approve/deny Sneedacity access (again).
   //
   // This should resolve confusion of why Sneedacity appears to record, but only
   // gets silence due to Sneedacity being denied microphone access previously.
   bool permsReset = false;
   gPrefs->Read(wxT("/MicrophonePermissionsReset"), &permsReset, false);
   if (!permsReset) {
      system("tccutil reset Microphone org.sneedacityteam.sneedacity");
      gPrefs->Write(wxT("/MicrophonePermissionsReset"), true);
   }
#endif

#if defined(__WXMAC__)
   // Bug 2709: Workaround CoreSVG locale issue
   Bind(wxEVT_MENU_OPEN, [=](wxMenuEvent &event)
   {
      wxSetlocale(LC_NUMERIC, wxString(wxT("C")));
      event.Skip();
   });

   Bind(wxEVT_MENU_CLOSE, [=](wxMenuEvent &event)
   {
      wxSetlocale(LC_NUMERIC, Languages::GetLocaleName());
      event.Skip();
   });
#endif

   return TRUE;
}

void SneedacityApp::InitCommandHandler()
{
   dprintf("SneedacityApp::InitCommandHandler()");
   mCmdHandler = std::make_unique<CommandHandler>();
   //SetNextHandler(mCmdHandler);
}

// AppCommandEvent callback - just pass the event on to the CommandHandler
void SneedacityApp::OnReceiveCommand(AppCommandEvent &event)
{
   dprintf("SneedacityApp::OnReceiveCommand()");
   wxASSERT(NULL != mCmdHandler);
   mCmdHandler->OnReceiveCommand(event);
}

void SneedacityApp::OnKeyDown(wxKeyEvent &event)
{
   dprintf("SneedacityApp::OnKeyDown()");
   if(event.GetKeyCode() == WXK_ESCAPE) {
      // Stop play, including scrub, but not record
      auto project = ::GetActiveProject();
      if ( project ) {
         auto token = ProjectAudioIO::Get( *project ).GetAudioIOToken();
         auto &scrubber = Scrubber::Get( *project );
         auto scrubbing = scrubber.HasMark();
         if (scrubbing)
            scrubber.Cancel();
         auto gAudioIO = AudioIO::Get();
         if((token > 0 &&
                  gAudioIO->IsAudioTokenActive(token) &&
                  gAudioIO->GetNumCaptureChannels() == 0) ||
            scrubbing)
            // ESC out of other play (but not record)
            ProjectAudioManager::Get( *project ).Stop();
         else
            event.Skip();
      }
   }

   event.Skip();
}

// Ensures directory is created and puts the name into result.
// result is unchanged if unsuccessful.
void SetToExtantDirectory( wxString & result, const wxString & dir ){
   // don't allow path of "".
   if( dir.empty() )
      return;
   if( wxDirExists( dir ) ){
      result = dir;
      return;
   }
   // Use '/' so that this works on Mac and Windows alike.
   wxFileName name( dir + "/junkname.cfg" );
   if( name.Mkdir( wxS_DIR_DEFAULT , wxPATH_MKDIR_FULL ) )
      result = dir;
}

bool SneedacityApp::InitTempDir()
{
   dprintf("SneedacityApp::InitTempDir()");
   // We need to find a temp directory location.
   auto tempFromPrefs = TempDirectory::TempDir();
   auto tempDefaultLoc = TempDirectory::DefaultTempDir();

   wxString temp;

   #ifdef __WXGTK__
   if (tempFromPrefs.length() > 0 && tempFromPrefs[0] != wxT('/'))
      tempFromPrefs = wxT("");
   #endif

   // Stop wxWidgets from printing its own error messages

   wxLogNull logNo;

   // Try temp dir that was stored in prefs first
   if( TempDirectory::IsTempDirectoryNameOK( tempFromPrefs ) )
      SetToExtantDirectory( temp, tempFromPrefs );

   // If that didn't work, try the default location

   if (temp.empty())
      SetToExtantDirectory( temp, tempDefaultLoc );

   // Check temp directory ownership on *nix systems only
   #ifdef __UNIX__
   struct stat tempStatBuf;
   if ( lstat(temp.mb_str(), &tempStatBuf) != 0 ) {
      temp.clear();
   }
   else {
      if ( geteuid() != tempStatBuf.st_uid ) {
         temp.clear();
      }
   }
   #endif

   if (temp.empty()) {
      // Failed
      if( !TempDirectory::IsTempDirectoryNameOK( tempFromPrefs ) ) {
         SneedacityMessageBox(XO(
"Sneedacity could not find a safe place to store temporary files.\nSneedacity needs a place where automatic cleanup programs won't delete the temporary files.\nPlease enter an appropriate directory in the preferences dialog."));
      } else {
         SneedacityMessageBox(XO(
"Sneedacity could not find a place to store temporary files.\nPlease enter an appropriate directory in the preferences dialog."));
      }

      // Only want one page of the preferences
      PrefsPanel::Factories factories;
      factories.push_back(DirectoriesPrefsFactory());
      GlobalPrefsDialog dialog(nullptr, nullptr, factories);
      dialog.ShowModal();

      SneedacityMessageBox(XO(
"Sneedacity is now going to exit. Please launch Sneedacity again to use the new temporary directory."));
      return false;
   }

   // The permissions don't always seem to be set on
   // some platforms.  Hopefully this fixes it...
   #ifdef __UNIX__
   chmod(OSFILENAME(temp), 0700);
   #endif

   TempDirectory::ResetTempDir();
   FileNames::UpdateDefaultPath(FileNames::Operation::Temp, temp);

   return true;
}

#if defined(__WXMSW__)

// Return true if there are no other instances of Sneedacity running,
// false otherwise.

bool SneedacityApp::CreateSingleInstanceChecker(const wxString &dir)
{
   dprintf("SneedacityApp::CreateSingleInstanceChecker()");
   wxString name = wxString::Format(wxT("sneedacity-lock-%s"), wxGetUserId());
   mChecker.reset();
   auto checker = std::make_unique<wxSingleInstanceChecker>();

   auto runningTwoCopiesStr = XO("Running two copies of Sneedacity simultaneously may cause\ndata loss or cause your system to crash.\n\n");

   if (!checker->Create(name, dir))
   {
      // Error initializing the wxSingleInstanceChecker.  We don't know
      // whether there is another instance running or not.

      auto prompt = XO(
"Sneedacity was not able to lock the temporary files directory.\nThis folder may be in use by another copy of Sneedacity.\n")
         + runningTwoCopiesStr
         + XO("Do you still want to start Sneedacity?");
      int action = SneedacityMessageBox(
         prompt,
         XO("Error Locking Temporary Folder"),
         wxYES_NO | wxICON_EXCLAMATION, NULL);
      if (action == wxNO)
         return false;
   }
   else if ( checker->IsAnotherRunning() ) {
      // Parse the command line to ensure correct syntax, but
      // ignore options other than -v, and only use the filenames, if any.
      auto parser = ParseCommandLine();
      if (!parser)
      {
         // Complaints have already been made
         return false;
      }

      if (parser->Found(wxT("v")))
      {
         wxPrintf("Sneedacity v%s\n", SNEEDACITY_VERSION_STRING);
         return false;
      }

      // Windows and Linux require absolute file names as command may
      // not come from current working directory.
      FilePaths filenames;
      for (size_t i = 0, cnt = parser->GetParamCount(); i < cnt; i++)
      {
         wxFileName filename(parser->GetParam(i));
         if (filename.MakeAbsolute())
         {
            filenames.push_back(filename.GetLongPath());
         }
      }

      // On Windows, we attempt to make a connection
      // to an already active Sneedacity.  If successful, we send
      // the first command line argument (the audio file name)
      // to that Sneedacity for processing.
      wxClient client;

      // We try up to 50 times since there's a small window
      // where the server may not have been fully initialized.
      for (int i = 0; i < 50; i++)
      {
         std::unique_ptr<wxConnectionBase> conn{ client.MakeConnection(wxEmptyString, IPC_APPL, IPC_TOPIC) };
         if (conn)
         {
            bool ok = false;
            if (filenames.size() > 0)
            {
               for (size_t i = 0, cnt = filenames.size(); i < cnt; i++)
               {
                  ok = conn->Execute(filenames[i]);
               }
            }
            else
            {
               // Send an empty string to force existing Sneedacity to front
               ok = conn->Execute(wxEmptyString);
            }

            if (ok)
               return false;
         }

         wxMilliSleep(10);
      }
      // There is another copy of Sneedacity running.  Force quit.

      auto prompt =  XO(
"The system has detected that another copy of Sneedacity is running.\n")
         + runningTwoCopiesStr
         + XO(
"Use the New or Open commands in the currently running Sneedacity\nprocess to open multiple projects simultaneously.\n");
      SneedacityMessageBox(
         prompt, XO("Sneedacity is already running"),
         wxOK | wxICON_ERROR);

      return false;
   }

   // Create the DDE IPC server
   mIPCServ = std::make_unique<IPCServ>(IPC_APPL);
   mChecker = std::move(checker);
   return true;
}
#endif

#if defined(__UNIX__)

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

// Return true if there are no other instances of Sneedacity running,
// false otherwise.

bool SneedacityApp::CreateSingleInstanceChecker(const wxString &dir)
{
   dprintf("SneedacityApp::CreateSingleInstanceChecker(const wxString &dir)");
   mIPCServ.reset();

   bool isServer = false;
   wxIPV4address addr;
   addr.LocalHost();

   struct sembuf op = {};

   // Generate the IPC key we'll use for both shared memory and semaphores.
   wxString datadir = FileNames::DataDir();
   key_t memkey = ftok(datadir.c_str(), 0);
   key_t servkey = ftok(datadir.c_str(), 1);
   key_t lockkey = ftok(datadir.c_str(), 2);

   // Create and map the shared memory segment where the port number
   // will be stored.
   int memid = shmget(memkey, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR);
   int *portnum = (int *) shmat(memid, nullptr, 0);

   // Create (or return) the SERVER semaphore ID
   int servid = semget(servkey, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

   // Create the LOCK semaphore only if it doesn't already exist.
   int lockid = semget(lockkey, 1, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

   // If the LOCK semaphore was successfully created, then this is the first
   // time Sneedacity has been run during this boot of the system. In this
   // case we know we'll become the "server" application, so set up the
   // semaphores to prepare for it.
   if (lockid != -1)
   {
      // Initialize value of each semaphore, 1 indicates released and 0
      // indicates acquired.
      //
      // Note that this action is NOT recorded in the semaphore's
      // UNDO buffer.
      semctl(servid, 0, SETVAL, 1);
      semctl(lockid, 0, SETVAL, 1);

      // Now acquire them so the semaphores will be set to the
      // released state when the process terminates.
      op.sem_num = 0;
      op.sem_op = -1;
      op.sem_flg = SEM_UNDO;
      if (semop(lockid, &op, 1) == -1 || semop(servid, &op, 1) == -1)
      {
         SneedacityMessageBox(
            XO("Unable to acquire semaphores.\n\n"
               "This is likely due to a resource shortage\n"
               "and a reboot may be required."),
            XO("Sneedacity Startup Failure"),
            wxOK | wxICON_ERROR);

         return false;
      }

      // We will be the server...
      isServer = true;
   }
   // Something catastrophic must have happened, so bail.
   else if (errno != EEXIST)
   {
      SneedacityMessageBox(
         XO("Unable to create semaphores.\n\n"
            "This is likely due to a resource shortage\n"
            "and a reboot may be required."),
         XO("Sneedacity Startup Failure"),
         wxOK | wxICON_ERROR);

      return false;
   }
   // Otherwise it's a normal startup and we need to determine whether
   // we'll be the server or the client.
   else
   {
      // Retrieve the LOCK semaphore since we wouldn't have gotten it above.
      lockid = semget(lockkey, 1, 0);

      // Acquire the LOCK semaphore. We may block here if another
      // process is currently setting up the server.
      op.sem_num = 0;
      op.sem_op = -1;
      op.sem_flg = SEM_UNDO;
      if (semop(lockid, &op, 1) == -1)
      {
         SneedacityMessageBox(
            XO("Unable to acquire lock semaphore.\n\n"
               "This is likely due to a resource shortage\n"
               "and a reboot may be required."),
            XO("Sneedacity Startup Failure"),
            wxOK | wxICON_ERROR);

         return false;
      }

      // Try to acquire the SERVER semaphore. If it's not currently active, then
      // we will become the server. Otherwise, this will fail and we'll know that
      // the server is already active and we will become the client.
      op.sem_num = 0;
      op.sem_op = -1;
      op.sem_flg = IPC_NOWAIT | SEM_UNDO;
      if (semop(servid, &op, 1) == 0)
      {
         isServer = true;
      }
      else if (errno != EAGAIN)
      {
         SneedacityMessageBox(
            XO("Unable to acquire server semaphore.\n\n"
               "This is likely due to a resource shortage\n"
               "and a reboot may be required."),
            XO("Sneedacity Startup Failure"),
            wxOK | wxICON_ERROR);

         return false;
      }
   }

   // Initialize the socket server if we're to be the server.
   if (isServer)
   {
      // The system will randomly assign a port
      addr.Service(0);

      // Create the socket and bind to it.
      auto serv = std::make_unique<wxSocketServer>(addr, wxSOCKET_NOWAIT);
      if (serv && serv->IsOk())
      {
         serv->SetEventHandler(*this, ID_IPC_SERVER);
         serv->SetNotify(wxSOCKET_CONNECTION_FLAG);
         serv->Notify(true);
         mIPCServ = std::move(serv);

         // Save the port number in shared memory so that clients
         // know where to connect.
         mIPCServ->GetLocal(addr);
         *portnum = addr.Service();
      }

      // Now that the server is active, we release the LOCK semaphore
      // to allow any waiters to continue. The SERVER semaphore will
      // remain locked for the duration of this processes execution
      // and will be cleaned up by the system.
      op.sem_num = 0;
      op.sem_op = 1;
      semop(lockid, &op, 1);

      // Bail if the server creation failed.
      if (mIPCServ == nullptr)
      {
         SneedacityMessageBox(
            XO("The Sneedacity IPC server failed to initialize.\n\n"
               "This is likely due to a resource shortage\n"
               "and a reboot may be required."),
            XO("Sneedacity Startup Failure"),
            wxOK | wxICON_ERROR);

         return false;
      }

      // We've successfully created the socket server and the app
      // should continue to initialize.
      return true;
   }

   // Retrieve the port number that the server is listening on.
   addr.Service(*portnum);

   // Now release the LOCK semaphore.
   op.sem_num = 0;
   op.sem_op = 1;
   semop(lockid, &op, 1);

   // If we get here, then Sneedacity is currently active. So, we connect
   // to it and we forward all filenames listed on the command line to
   // the active process.

   // Setup the socket
   //
   // A wxSocketClient must not be deleted by us, but rather, let the
   // framework do appropriate delayed deletion after Destroy()
   Destroy_ptr<wxSocketClient> sock { safenew wxSocketClient() };
   sock->SetFlags(wxSOCKET_WAITALL);

   // Attempt to connect to an active Sneedacity.
   sock->Connect(addr, true);
   if (!sock->IsConnected())
   {
      // All attempts to become the server or connect to one have failed.  Not
      // sure what we can say about the error, but it's probably not because
      // Sneedacity is already running.
      SneedacityMessageBox(
         XO("An unrecoverable error has occurred during startup"),
         XO("Sneedacity Startup Failure"),
         wxOK | wxICON_ERROR);

      return false;
   }

   // Parse the command line to ensure correct syntax, but ignore
   // options other than -v, and only use the filenames, if any.
   auto parser = ParseCommandLine();
   if (!parser)
   {
      // Complaints have already been made
      return false;
   }

   // Display Sneedacity's version if requested
   if (parser->Found(wxT("v")))
   {
      wxPrintf("Sneedacity v%s\n", SNEEDACITY_VERSION_STRING);

      return false;
   }

#if defined(__WXMAC__)
   // On macOS the client gets events from the wxWidgets framework that
   // go to SneedacityApp::MacOpenFile. Forward the file names to the prior
   // instance via the socket.
   for (const auto &filename: ofqueue)
   {
      auto str = filename.c_str().AsWChar();
      sock->WriteMsg(str, (filename.length() + 1) * sizeof(*str));
   }
#endif

   // On macOS and Linux, forward any file names found in the command
   // line arguments.
   for (size_t j = 0, cnt = parser->GetParamCount(); j < cnt; ++j)
   {
      wxFileName filename(parser->GetParam(j));
      if (filename.MakeAbsolute())
      {
         const wxString param = filename.GetLongPath();
         sock->WriteMsg((const wxChar *) param, (param.length() + 1) * sizeof(wxChar));
      }
   }

   // Send an empty string to force existing Sneedacity to front
   sock->WriteMsg(wxEmptyString, sizeof(wxChar));

   // We've forwarded all of the filenames, so let the caller know
   // to terminate.
   return false;
}

void SneedacityApp::OnServerEvent(wxSocketEvent & /* evt */)
{
   dprintf("SneedacityApp::OnServerEvent()");
   wxSocketBase *sock;

   // Accept all pending connection requests
   do
   {
      sock = mIPCServ->Accept(false);
      if (sock)
      {
         // Setup the socket
         sock->SetEventHandler(*this, ID_IPC_SOCKET);
         sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
         sock->Notify(true);
      }
   } while (sock);
}

void SneedacityApp::OnSocketEvent(wxSocketEvent & evt)
{
   dprintf("SneedacityApp::OnSocketEvent()");
   wxSocketBase *sock = evt.GetSocket();

   if (evt.GetSocketEvent() == wxSOCKET_LOST)
   {
      sock->Destroy();
      return;
   }

   // Read the length of the filename and bail if we have a short read
   wxChar name[PATH_MAX];
   sock->ReadMsg(&name, sizeof(name));
   if (!sock->Error())
   {
      // Add the filename to the queue.  It will be opened by
      // the OnTimer() event when it is safe to do so.
      ofqueue.push_back(name);
   }
}

#endif

std::unique_ptr<wxCmdLineParser> SneedacityApp::ParseCommandLine()
{
   auto parser = std::make_unique<wxCmdLineParser>(argc, argv);
   if (!parser)
   {
      return nullptr;
   }

   /*i18n-hint: This controls the number of bytes that Sneedacity will
    *           use when writing files to the disk */
   parser->AddOption(wxT("b"), wxT("blocksize"), _("set max disk block size in bytes"),
                     wxCMD_LINE_VAL_NUMBER);

   /*i18n-hint: This displays a list of available options */
   parser->AddSwitch(wxT("h"), wxT("help"), _("this help message"),
                     wxCMD_LINE_OPTION_HELP);

   /*i18n-hint: This runs a set of automatic tests on Sneedacity itself */
   parser->AddSwitch(wxT("t"), wxT("test"), _("run self diagnostics"));

   /*i18n-hint: This displays the Sneedacity version */
   parser->AddSwitch(wxT("v"), wxT("version"), _("display Sneedacity version"));

   /*i18n-hint: This is a list of one or more files that Sneedacity
    *           should open upon startup */
   parser->AddParam(_("audio or project file name"),
                    wxCMD_LINE_VAL_STRING,
                    wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL);
                    
   // Debug/trace printfs   
   parser->AddSwitch(wxT("d"), wxT("debug"), _("display debug and trace messages"));

   // Run the parser
   if (parser->Parse() == 0)
      return parser;

   return{};
}

void SneedacityApp::OnQueryEndSession(wxCloseEvent & event)
{
   bool mustVeto = false;

#ifdef __WXMAC__
   mustVeto = wxDialog::OSXHasModalDialogsOpen();
#endif

   if ( mustVeto )
      event.Veto(true);
   else
      OnEndSession(event);
}

void SneedacityApp::OnEndSession(wxCloseEvent & event)
{
   bool force = !event.CanVeto();

   // Try to close each open window.  If the user hits Cancel
   // in a Save Changes dialog, don't continue.
   gIsQuitting = true;
   if (AllProjects{}.size())
      // PRL:  Always did at least once before close might be vetoed
      // though I don't know why that is important
      ProjectManager::SaveWindowSize();
   bool closedAll = AllProjects::Close( force );
   if ( !closedAll )
   {
      gIsQuitting = false;
      event.Veto();
   }
}

int SneedacityApp::OnExit()
{
   gIsQuitting = true;
   while(Pending())
   {
      Dispatch();
   }

   Importer::Get().Terminate();

   if(gPrefs)
   {
      bool bFalse = false;
      //Should we change the commands.cfg location next startup?
      if(gPrefs->Read(wxT("/QDeleteCmdCfgLocation"), &bFalse))
      {
         gPrefs->DeleteEntry(wxT("/QDeleteCmdCfgLocation"));
         gPrefs->Write(wxT("/DeleteCmdCfgLocation"), true);
         gPrefs->Flush();
      }
   }

   FileHistory::Global().Save(*gPrefs);

   FinishPreferences();

#ifdef USE_FFMPEG
   DropFFmpegLibs();
#endif

   DeinitFFT();

   AudioIO::Deinit();

   MenuTable::DestroyRegistry();

   // Terminate the PluginManager (must be done before deleting the locale)
   PluginManager::Get().Terminate();

   return 0;
}

// The following five methods are currently only used on Mac OS,
// where it's possible to have a menu bar but no windows open.
// It doesn't hurt any other platforms, though.

// ...That is, as long as you check to see if no windows are open
// before executing the stuff.
// To fix this, check to see how many project windows are open,
// and skip the event unless none are open (which should only happen
// on the Mac, at least currently.)

void SneedacityApp::OnMenuAbout(wxCommandEvent & /*event*/)
{
   // This function shadows a similar function
   // in Menus.cpp, but should only be used on the Mac platform.
#ifdef __WXMAC__
   // Modeless dialog, consistent with other Mac applications
   // Not more than one at once!
   const auto instance = AboutDialog::ActiveIntance();
   if (instance)
      instance->Raise();
   else
      // This dialog deletes itself when dismissed
      (safenew AboutDialog{ nullptr })->Show(true);
#else
      wxASSERT(false);
#endif
}

void SneedacityApp::OnMenuNew(wxCommandEvent & event)
{
   // This function shadows a similar function
   // in Menus.cpp, but should only be used on the Mac platform
   // when no project windows are open. This check assures that
   // this happens, and enable the same code to be present on
   // all platforms.

   if(AllProjects{}.empty())
      (void) ProjectManager::New();
   else
      event.Skip();
}


void SneedacityApp::OnMenuOpen(wxCommandEvent & event)
{
   // This function shadows a similar function
   // in Menus.cpp, but should only be used on the Mac platform
   // when no project windows are open. This check assures that
   // this happens, and enable the same code to be present on
   // all platforms.


   if(AllProjects{}.empty())
      ProjectManager::OpenFiles(NULL);
   else
      event.Skip();


}

void SneedacityApp::OnMenuPreferences(wxCommandEvent & event)
{
   // This function shadows a similar function
   // in Menus.cpp, but should only be used on the Mac platform
   // when no project windows are open. This check assures that
   // this happens, and enable the same code to be present on
   // all platforms.

   if(AllProjects{}.empty()) {
      GlobalPrefsDialog dialog(nullptr /* parent */, nullptr );
      dialog.ShowModal();
   }
   else
      event.Skip();

}

void SneedacityApp::OnMenuExit(wxCommandEvent & event)
{
   // This function shadows a similar function
   // in Menus.cpp, but should only be used on the Mac platform
   // when no project windows are open. This check assures that
   // this happens, and enable the same code to be present on
   // all platforms.

   // LL:  Removed "if" to allow closing based on final project count.
   // if(AllProjects{}.empty())
      QuitSneedacity();

   // LL:  Veto quit if projects are still open.  This can happen
   //      if the user selected Cancel in a Save dialog.
   event.Skip(AllProjects{}.empty());

}

//BG: On Windows, associate the aup file type with Sneedacity
/* We do this in the Windows installer now,
   to avoid issues where user doesn't have admin privileges, but
   in case that didn't work, allow the user to decide at startup.

   //v Should encapsulate this & allow access from Prefs, too,
   //      if people want to manually change associations.
*/
#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__) && !defined(__CYGWIN__)
void SneedacityApp::AssociateFileTypes()
{
   // Check pref in case user has already decided against it.
   bool bWantAssociateFiles = true;
   if (gPrefs->Read(wxT("/WantAssociateFiles"), &bWantAssociateFiles) &&
         !bWantAssociateFiles)
   {
      // User has already decided against it
      return;
   }

   wxRegKey associateFileTypes;

   auto IsDefined = [&](const wxString &type)
   {
      associateFileTypes.SetName(wxString::Format(wxT("HKCR\\%s"), type));
      bool bKeyExists = associateFileTypes.Exists();
      if (!bKeyExists)
      {
         // Not at HKEY_CLASSES_ROOT. Try HKEY_CURRENT_USER.
         associateFileTypes.SetName(wxString::Format(wxT("HKCU\\Software\\Classes\\%s"), type));
         bKeyExists = associateFileTypes.Exists();
      }
      return bKeyExists;
   };

   auto DefineType = [&](const wxString &type)
   {
      wxString root_key = wxT("HKCU\\Software\\Classes\\");

      // Start with HKEY_CLASSES_CURRENT_USER.
      associateFileTypes.SetName(wxString::Format(wxT("%s%s"), root_key, type));
      if (!associateFileTypes.Create(true))
      {
         // Not at HKEY_CLASSES_CURRENT_USER. Try HKEY_CURRENT_ROOT.
         root_key = wxT("HKCR\\");
         associateFileTypes.SetName(wxString::Format(wxT("%s%s"), root_key, type));
         if (!associateFileTypes.Create(true))
         {
            // Actually, can't create keys. Empty root_key to flag failure.
            root_key.empty();
         }
      }

      if (!root_key.empty())
      {
         associateFileTypes = wxT("Sneedacity.Project"); // Finally set value for the key
      }

      return root_key;
   };

   // Check for legacy and UP types
   if (IsDefined(wxT(".aup3")) && IsDefined(wxT(".aup")) && IsDefined(wxT("Sneedacity.Project")))
   {
      // Already defined, so bail
      return;
   }

   // File types are not currently associated.
   int wantAssoc =
      SneedacityMessageBox(
         XO(
"Sneedacity project (.aup3) files are not currently \nassociated with Sneedacity. \n\nAssociate them, so they open on double-click?"),
         XO("Sneedacity Project Files"),
         wxYES_NO | wxICON_QUESTION);

   if (wantAssoc == wxNO)
   {
      // User said no. Set a pref so we don't keep asking.
      gPrefs->Write(wxT("/WantAssociateFiles"), false);
      gPrefs->Flush();
      return;
   }

   // Show that user wants associations
   gPrefs->Write(wxT("/WantAssociateFiles"), true);
   gPrefs->Flush();

   wxString root_key;

   root_key = DefineType(wxT(".aup3"));
   if (root_key.empty())
   {
      //v Warn that we can't set keys. Ask whether to set pref for no retry?
   }
   else
   {
      DefineType(wxT(".aup"));

      associateFileTypes = wxT("Sneedacity.Project"); // Finally set value for .AUP key
      associateFileTypes.SetName(root_key + wxT("Sneedacity.Project"));
      if (!associateFileTypes.Exists())
      {
         associateFileTypes.Create(true);
         associateFileTypes = wxT("Sneedacity Project File");
      }

      associateFileTypes.SetName(root_key + wxT("Sneedacity.Project\\shell"));
      if (!associateFileTypes.Exists())
      {
         associateFileTypes.Create(true);
         associateFileTypes = wxT("");
      }

      associateFileTypes.SetName(root_key + wxT("Sneedacity.Project\\shell\\open"));
      if (!associateFileTypes.Exists())
      {
         associateFileTypes.Create(true);
      }

      associateFileTypes.SetName(root_key + wxT("Sneedacity.Project\\shell\\open\\command"));
      wxString tmpRegAudPath;
      if(associateFileTypes.Exists())
      {
         tmpRegAudPath = associateFileTypes.QueryDefaultValue().Lower();
      }

      if (!associateFileTypes.Exists() ||
            (tmpRegAudPath.Find(wxT("sneedacity.exe")) >= 0))
      {
         associateFileTypes.Create(true);
         associateFileTypes = (wxString)argv[0] + (wxString)wxT(" \"%1\"");
      }

#if 0
      // These can be use later to support more startup messages
      // like maybe "Import into existing project" or some such.
      // Leaving here for an example...
      associateFileTypes.SetName(root_key + wxT("Sneedacity.Project\\shell\\open\\ddeexec"));
      if (!associateFileTypes.Exists())
      {
         associateFileTypes.Create(true);
         associateFileTypes = wxT("%1");
      }

      associateFileTypes.SetName(root_key + wxT("Sneedacity.Project\\shell\\open\\ddeexec\\Application"));
      if (!associateFileTypes.Exists())
      {
         associateFileTypes.Create(true);
         associateFileTypes = IPC_APPL;
      }

      associateFileTypes.SetName(root_key + wxT("Sneedacity.Project\\shell\\open\\ddeexec\\Topic"));
      if (!associateFileTypes.Exists())
      {
         associateFileTypes.Create(true);
         associateFileTypes = IPC_TOPIC;
      }
#endif
   }
}
#endif
