/**********************************************************************

  Sneedacity: A Digital Audio Editor

  KeyboardCapture.h

  Paul Licameli split this from Project.h

**********************************************************************/

#ifndef __SNEEDACITY_KEYBOARD_CAPTURE__
#define __SNEEDACITY_KEYBOARD_CAPTURE__



#include <functional>
#include <wx/event.h>

////////////////////////////////////////////////////////////
/// Custom events
////////////////////////////////////////////////////////////

/// \brief Type of event that may be sent to a window while it is
/// installed as the handler with KeyboardCapture::Capture; if it does not skip
/// the event, it will receive further events of type wxEVT_KEY_DOWN,
/// and then wxEVT_CHAR (if key-down was not skipped; those further events
/// don't actually come from wxWidgets, but are simulated by Sneedacity, as
/// translations of the EVT_CHAR_HOOK event); or, wxEVT_KEY_UP (really from
/// wxWidgets).
DECLARE_EXPORTED_EVENT_TYPE(SNEEDACITY_DLL_API, EVT_CAPTURE_KEY, -1);

namespace KeyboardCapture
{
   SNEEDACITY_DLL_API bool IsHandler(const wxWindow *handler);
   SNEEDACITY_DLL_API wxWindow *GetHandler();
   SNEEDACITY_DLL_API void Capture(wxWindow *handler);
   SNEEDACITY_DLL_API void Release(wxWindow *handler);

   using FilterFunction = std::function< bool( wxKeyEvent& ) >;

   /// \brief Install a pre-filter, returning the previously installed one
   /// Pre-filter is called before passing the event to the captured window; if it
   /// returns false, then skip the event entirely
   SNEEDACITY_DLL_API
   FilterFunction SetPreFilter( const FilterFunction &function );
   
   /// \brief Install a post-filter, returning the previously installed one
   /// Post-filter is called if the captured window skips either the
   /// EVT_CAPTURE_KEY or the following wxKEY_DOWN event (but not if
   /// it skips only the wxEVT_CHAR or wxEVT_KEY_UP event); it is passed a
   /// wxKEY_DOWN or a wxKEY_UP event; if it returns false, then the event is
   /// skipped
   SNEEDACITY_DLL_API
   FilterFunction SetPostFilter( const FilterFunction &function );

   /// \brief a function useful to implement a focus event handler
   /// The window releases the keyboard if the event is for killing focus,
   /// otherwise the window captures the keyboard; then refresh the window
   /// and skip the event
   SNEEDACITY_DLL_API
   void OnFocus( wxWindow &window, wxFocusEvent &event );
}

#endif
