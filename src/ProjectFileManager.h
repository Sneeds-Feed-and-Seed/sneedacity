/**********************************************************************

Sneedacity: A Digital Audio Editor

ProjectFileManager.h

Paul Licameli split from SneedacityProject.h

**********************************************************************/

#ifndef __SNEEDACITY_PROJECT_FILE_MANAGER__
#define __SNEEDACITY_PROJECT_FILE_MANAGER__

#include <functional>
#include <memory>
#include <vector>

#include "ClientData.h" // to inherit
#include "FileNames.h" // for FileType

class wxString;
class wxFileName;
class SneedacityProject;
class Track;
class TrackList;
class WaveTrack;
class XMLTagHandler;

using WaveTrackArray = std::vector < std::shared_ptr < WaveTrack > >;
using TrackHolders = std::vector< WaveTrackArray >;

class SNEEDACITY_DLL_API ProjectFileManager final
   : public ClientData::Base
{
public:
   static ProjectFileManager &Get( SneedacityProject &project );
   static const ProjectFileManager &Get( const SneedacityProject &project );

   // Open and close a file, invisibly, removing its Autosave blob
   static void DiscardAutosave(const FilePath &filename);

   explicit ProjectFileManager( SneedacityProject &project );
   ProjectFileManager( const ProjectFileManager & ) PROHIBITED;
   ProjectFileManager &operator=( const ProjectFileManager & ) PROHIBITED;
   ~ProjectFileManager();

   bool OpenProject();
   void CloseProject();
   bool OpenNewProject();

   void CompactProjectOnClose();

   bool Save();
   bool SaveAs(bool allowOverwrite = false);
   bool SaveAs(const FilePath &newFileName, bool addToHistory = true);
   // strProjectPathName is full path for aup except extension
   bool SaveFromTimerRecording( wxFileName fnFile );
   bool SaveCopy(const FilePath &fileName = wxT(""));

   /** @brief Show an open dialogue for opening audio files, and possibly other
    * sorts of files.
    *
    * The file type filter will automatically contain:
    * - "All files" with any extension or none,
    * - "All supported files" based on the file formats supported in this
    *   build of Sneedacity,
    * - All of the individual formats specified by the importer plug-ins which
    *   are built into this build of Sneedacity, each with the relevant file
    *   extensions for that format.
    * The dialogue will start in the DefaultOpenPath directory read from the
    * preferences, failing that the working directory. The file format filter
    * will be set to the DefaultOpenType from the preferences, failing that
    * the first format specified in the dialogue. These two parameters will
    * be saved to the preferences once the user has chosen a file to open.
    * @param extraType Specify an additional format to allow opening in this
    * dialogue.
    * @return Array of file paths which the user selected to open (multiple
    * selections allowed).
    */
   static wxArrayString ShowOpenDialog(FileNames::Operation op,
      const FileNames::FileType &extraType = {});

   static bool IsAlreadyOpen(const FilePath &projPathName);

   //! A function that returns a project to use for opening a file; argument is true if opening a project file
   using ProjectChooserFn = std::function<SneedacityProject&(bool)>;

   /*!
    Opens files of many kinds.  In case of import (sound, MIDI, or .aup), the undo history is pushed.
    @param chooser told whether opening a project file; decides which project to open into
    @param fileName the name and contents are examined to decide a type and open appropriately
    @param addtohistory whether to add .aup3 files to the MRU list (but always done for imports)
    @return if something was successfully opened, the project containing it; else null
    */
   static SneedacityProject *OpenFile( const ProjectChooserFn &chooser,
      const FilePath &fileName, bool addtohistory = true);

   bool Import(const FilePath &fileName,
               bool addToHistory = true);

   void Compact();

   void AddImportedTracks(const FilePath &fileName,
                     TrackHolders &&newTracks);

   bool GetMenuClose() const { return mMenuClose; }
   void SetMenuClose(bool value) { mMenuClose = value; }

private:
   /*!
    @param fileName a path assumed to exist and contain an .aup3 project
    @param addtohistory whether to add the file to the MRU list
    @return if something was successfully opened, the project containing it; else null
    */
   SneedacityProject *OpenProjectFile(
      const FilePath &fileName, bool addtohistory);

   struct ReadProjectResults
   {
      bool parseSuccess;
      bool trackError;
      const TranslatableString errorString;
      wxString helpUrl;
   };
   ReadProjectResults ReadProjectFile(
      const FilePath &fileName, bool discardAutosave = false );

   bool DoSave(const FilePath & fileName, bool fromSaveAs);

   SneedacityProject &mProject;

   std::shared_ptr<TrackList> mLastSavedTracks;
   
   // Are we currently closing as the result of a menu command?
   bool mMenuClose{ false };
};

#endif
