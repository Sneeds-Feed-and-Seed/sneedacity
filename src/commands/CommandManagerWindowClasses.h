/**********************************************************************

  Sneedacity: A Digital Audio Editor

  CommandManagerWindowClasses.h

  Paul Licameli

**********************************************************************/

#ifndef __SNEEDACITY_COMMAND_MANAGER_WINDOW_CLASSES__
#define __SNEEDACITY_COMMAND_MANAGER_WINDOW_CLASSES__

/*
 \brief By default, windows when focused override the association of the digits
 and certain navigation keys with commands, but certain windows do not, and
 those inherit this class.
 */
struct SNEEDACITY_DLL_API NonKeystrokeInterceptingWindow
{
   virtual ~NonKeystrokeInterceptingWindow();
};

/*
 \brief Top-level windows that do redirect keystrokes to the associated
 project's CommandManager inherit this class.
*/
struct SNEEDACITY_DLL_API TopLevelKeystrokeHandlingWindow
{
   virtual ~TopLevelKeystrokeHandlingWindow();
};

#endif
