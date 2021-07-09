/**********************************************************************

  Sneedacity: A Digital Audio Editor

  XMLWriter.h

  Leland Lucius

**********************************************************************/
#ifndef __SNEEDACITY_XML_XML_FILE_WRITER__
#define __SNEEDACITY_XML_XML_FILE_WRITER__

#include <vector>
#include <wx/ffile.h> // to inherit

#include "../FileException.h"

#include "Identifier.h"

///
/// XMLWriter
///
class SNEEDACITY_DLL_API XMLWriter /* not final */ {

 public:

   XMLWriter();
   virtual ~XMLWriter();

   virtual void StartTag(const wxString &name);
   virtual void EndTag(const wxString &name);

   // nonvirtual pass-through
   void WriteAttr(const wxString &name, const Identifier &value)
      // using GET once here, permitting Identifiers in XML,
      // so no need for it at each WriteAttr call
      { WriteAttr( name, value.GET() ); }

   virtual void WriteAttr(const wxString &name, const wxString &value);
   virtual void WriteAttr(const wxString &name, const wxChar *value);

   virtual void WriteAttr(const wxString &name, int value);
   virtual void WriteAttr(const wxString &name, bool value);
   virtual void WriteAttr(const wxString &name, long value);
   virtual void WriteAttr(const wxString &name, long long value);
   virtual void WriteAttr(const wxString &name, size_t value);
   virtual void WriteAttr(const wxString &name, float value, int digits = -1);
   virtual void WriteAttr(const wxString &name, double value, int digits = -1);

   virtual void WriteData(const wxString &value);

   virtual void WriteSubTree(const wxString &value);

   virtual void Write(const wxString &data) = 0;

   // Escape a string, replacing certain characters with their
   // XML encoding, i.e. '<' becomes '&lt;'
   wxString XMLEsc(const wxString & s);

 protected:

   bool mInTag;
   int mDepth;
   wxArrayString mTagstack;
   std::vector<int> mHasKids;

};

///
/// XMLFileWriter
///

/// This writes to a provisional file, and replaces the previously existing
/// contents by a file rename in Commit() only after all writes succeed.
/// The original contents may also be retained at a backup path name, as
/// directed by the optional constructor argument.
/// If it is destroyed before Commit(), then the provisional file is removed.
/// If the construction and all operations are inside a GuardedCall or event
/// handler, then the default delayed handler action in case of exceptions will
/// notify the user of problems.
class SNEEDACITY_DLL_API XMLFileWriter final : private wxFFile, public XMLWriter {

 public:

   /// The caption is for message boxes to show in case of errors.
   /// Might throw.
   XMLFileWriter(
      const FilePath &outputPath, const TranslatableString &caption,
      bool keepBackup = false );

   virtual ~XMLFileWriter();

   /// Close all tags and then close the file.
   /// Might throw.  If not, then create
   /// or modify the file at the output path.
   /// Composed of two steps, PreCommit() and PostCommit()
   void Commit();

   /// Does the part of Commit that might fail because of exhaustion of space
   void PreCommit();

   /// Does other parts of Commit that are not likely to fail for exhaustion
   /// of space, but might for other reasons
   void PostCommit();

   /// Write to file. Might throw.
   void Write(const wxString &data) override;

   FilePath GetBackupName() const { return mBackupName; }

 private:

   void ThrowException(
      const wxFileName &fileName, const TranslatableString &caption)
   {
      throw FileException{ FileException::Cause::Write, fileName, caption };
   }

   /// Close file without automatically ending tags.
   /// Might throw.
   void CloseWithoutEndingTags(); // for auto-save files

   const FilePath mOutputPath;
   const TranslatableString mCaption;
   FilePath mBackupName;
   const bool mKeepBackup;

   wxFFile mBackupFile;

   bool mCommitted{ false };
};

///
/// XMLStringWriter
///
class XMLStringWriter final : public wxString, public XMLWriter {

 public:

   XMLStringWriter(size_t initialSize = 0);
   virtual ~XMLStringWriter();

   void Write(const wxString &data) override;

 private:

};

#endif
