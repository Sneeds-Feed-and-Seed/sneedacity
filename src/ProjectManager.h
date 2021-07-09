/**********************************************************************

Sneedacity: A Digital Audio Editor

ProjectManager.h

Paul Licameli split from SneedacityProject.h

**********************************************************************/

#ifndef __SNEEDACITY_PROJECT_MANAGER__
#define __SNEEDACITY_PROJECT_MANAGER__

#include <memory>

#include <wx/event.h> // to inherit
#include "ClientData.h" // to inherit
#include "Identifier.h"

class wxTimer;
class wxTimerEvent;

class SneedacityProject;
struct AudioIOStartStreamOptions;

///\brief Object associated with a project for high-level management of the
/// project's lifetime, including creation, destruction, opening from file,
/// importing, pushing undo states, and reverting to saved states
class SNEEDACITY_DLL_API ProjectManager final
   : public wxEvtHandler
   , public ClientData::Base
{
public:
   static ProjectManager &Get( SneedacityProject &project );
   static const ProjectManager &Get( const SneedacityProject &project );

   explicit ProjectManager( SneedacityProject &project );
   ProjectManager( const ProjectManager & ) PROHIBITED;
   ProjectManager &operator=( const ProjectManager & ) PROHIBITED;
   ~ProjectManager() override;

   // This is the factory for projects:
   static SneedacityProject *New();

   // The function that imports files can act as a factory too, and for that
   // reason remains in this class, not in ProjectFileManager
   static void OpenFiles(SneedacityProject *proj);

   //! False when it is unsafe to overwrite proj with contents of an .aup3 file
   static bool SafeToOpenProjectInto(SneedacityProject &proj);

   //! Callable object that supplies the `chooser` argument of ProjectFileManager::OpenFile
   /*!
    Its operator(), called lower down in ProjectFileManager, decides which project to put new file data into,
    using file type information deduced there.  It may have the side effect of creating a project.

    At the higher level where it is constructed, it provides conditional RAII.
    One indicates there that the file opening succeeded by calling Commit().  But if that is never
    called, creation of projects, or changes to a preexisting project, are undone.
    */
   class ProjectChooser {
   public:
       /*!
       @param pProject if not null, an existing project to reuse if possible
       @param reuseNonemptyProject if true, may reuse the given project when nonempty,
       but only if importing (not for a project file)
       */
      ProjectChooser( SneedacityProject *pProject, bool reuseNonemptyProject )
         : mpGivenProject{ pProject }
         , mReuseNonemptyProject{ reuseNonemptyProject }
      {}
      //! Don't copy.  Use std::ref to pass it to ProjectFileManager
      ProjectChooser( const ProjectChooser& ) PROHIBITED;
      //! Destroy any fresh project, or rollback the existing project, unless committed
      ~ProjectChooser();
      //! May create a fresh project
      SneedacityProject &operator() ( bool openingProjectFile );
      //! Commit the creation of any fresh project or changes to the existing project
      void Commit();

   private:
      SneedacityProject *mpGivenProject;
      SneedacityProject *mpUsedProject = nullptr;
      bool mReuseNonemptyProject;
   };

   //! Open a file into an SneedacityProject, returning the project, or nullptr for failure
   /*!
    If an exception escapes this function, no projects are created.
    @param pGivenProject if not null, a project that may be reused
    @param fileNameArg path to the file to open; not always an Sneedacity project file, may be an import
    @param addtohistory whether to add .aup3 files to the MRU list (but always done for imports)
    @param reuseNonemptyProject if true, may reuse the given project when nonempty,
       but only if importing (not for a project file)
    */
   static SneedacityProject *OpenProject(
      SneedacityProject *pGivenProject,
      const FilePath &fileNameArg, bool addtohistory, bool reuseNonemptyProject);

   void ResetProjectToEmpty();

   static void SaveWindowSize();

   // Routine to estimate how many minutes of recording time are left on disk
   int GetEstimatedRecordingMinsLeftOnDisk(long lCaptureChannels = 0);
   // Converts number of minutes to human readable format
   TranslatableString GetHoursMinsString(int iMinutes);

   void SetStatusText( const TranslatableString &text, int number );
   void SetSkipSavePrompt(bool bSkip) { sbSkipPromptingForSave = bSkip; };

private:
   void OnReconnectionFailure(wxCommandEvent & event);
   void OnCloseWindow(wxCloseEvent & event);
   void OnTimer(wxTimerEvent & event);
   void OnOpenAudioFile(wxCommandEvent & event);
   void OnStatusChange( wxCommandEvent& );

   void RestartTimer();

   // non-static data members
   SneedacityProject &mProject;

   std::unique_ptr<wxTimer> mTimer;

   DECLARE_EVENT_TABLE()

   static bool sbWindowRectAlreadySaved;
   static bool sbSkipPromptingForSave;
};

#endif
