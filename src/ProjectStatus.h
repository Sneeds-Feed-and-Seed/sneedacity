/**********************************************************************

Sneedacity: A Digital Audio Editor

ProjectStatus.h

Paul Licameli

**********************************************************************/

#ifndef __SNEEDACITY_PROJECT_STATUS__
#define __SNEEDACITY_PROJECT_STATUS__
#endif

#include <utility>
#include <vector>
#include <wx/event.h> // to declare custom event type
#include "ClientData.h" // to inherit
#include "Prefs.h"

class SneedacityProject;
class wxWindow;

enum StatusBarField : int {
   stateStatusBarField = 1,
   mainStatusBarField = 2,
   rateStatusBarField = 3,
   
   nStatusBarFields = 3
};

// Type of event emitted by the project when its status message is set
// GetInt() identifies the intended field of the status bar
wxDECLARE_EXPORTED_EVENT(SNEEDACITY_DLL_API,
                         EVT_PROJECT_STATUS_UPDATE, wxCommandEvent);

class SNEEDACITY_DLL_API ProjectStatus final
   : public ClientData::Base
   , public PrefsListener
{
public:
   static ProjectStatus &Get( SneedacityProject &project );
   static const ProjectStatus &Get( const SneedacityProject &project );

   explicit ProjectStatus( SneedacityProject &project );
   ProjectStatus( const ProjectStatus & ) = delete;
   ProjectStatus &operator= ( const ProjectStatus & ) = delete;
   ~ProjectStatus() override;

   // Type of a function to report translatable strings, and also report an extra
   // margin, to request that the corresponding field of the status bar should
   // be wide enough to contain any of those strings plus the margin.
   using StatusWidthResult = std::pair< std::vector<TranslatableString>, unsigned >;
   using StatusWidthFunction = std::function<
      StatusWidthResult( const SneedacityProject &, StatusBarField )
   >;
   using StatusWidthFunctions = std::vector< StatusWidthFunction >;

   // Typically a static instance of this struct is used.
   struct SNEEDACITY_DLL_API RegisteredStatusWidthFunction
   {
      explicit
      RegisteredStatusWidthFunction( const StatusWidthFunction &function );
   };

   static const StatusWidthFunctions &GetStatusWidthFunctions();

   const TranslatableString &Get( StatusBarField field = mainStatusBarField ) const;
   void Set(const TranslatableString &msg,
      StatusBarField field = mainStatusBarField);

   // PrefsListener implementation
   void UpdatePrefs() override;

private:
   SneedacityProject &mProject;
   TranslatableString mLastStatusMessages[ nStatusBarFields ];
};
