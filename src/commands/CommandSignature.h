/**********************************************************************

   Sneedacity - A Digital Audio Editor
   Copyright 1999-2009 Audacity Team, 2021 Sneedacity Team
   License: wxwidgets

   Dan Horgan

******************************************************************//**

\file CommandSignature.h
\brief Declaration of CommandSignature class

\class CommandSignature
\brief Class that maps parameter names to default values and validators

*//*******************************************************************/

#ifndef __COMMANDSIGNATURE__
#define __COMMANDSIGNATURE__

#include "CommandMisc.h"

class wxString;
class wxVariant;
class Validator;

class SNEEDACITY_DLL_API CommandSignature
{
private:
   ParamValueMap mDefaults;
   ValidatorMap mValidators;
   CommandSignature(const CommandSignature &) PROHIBITED;
   CommandSignature& operator=(const CommandSignature &) PROHIBITED;
public:
   explicit CommandSignature();
   ~CommandSignature();

   // Add a parameter to the signature.
   //    name:  the parameter name (case-sensitive)
   //    dft:   a default value
   //    valid: a suitable validator (caller doesn't need to DELETE it)
   void AddParameter(const wxString &name,
                     const wxVariant &dft,
                     std::unique_ptr<Validator> &&valid);

   // Methods for accessing the signature
   ParamValueMap GetDefaults() const;
   Validator &GetValidator(const wxString &paramName);
};

#endif /* End of include guard: __COMMANDSIGNATURE__ */
