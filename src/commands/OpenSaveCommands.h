/**********************************************************************

   Sneedacity: A Digital Audio Editor
   Copyright 1999-2018 Audacity Team, 2021 Sneedacity Team.
   File License: wxwidgets

   OpenSaveCommands.h
   Stephen Parry
   James Crook

******************************************************************//**

\class OpenProjectCommand
\brief Command for opening an Sneedacity project

\class SaveProjectCommand
\brief Command for saving an Sneedacity project

\class SaveCopyCommand
\brief Command for saving a copy of currently project

\class SaveLogCommand
\brief Command for saving the log contents

*//*******************************************************************/

#include "Command.h"
#include "CommandType.h"

class OpenProjectCommand : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Opens a project.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#open_project";}
public:
   wxString mFileName;
   bool mbAddToHistory;
   bool bHasAddToHistory;
};

class SaveProjectCommand : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Saves a project.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#save_project";}
public:
   wxString mFileName;
   bool mbAddToHistory;
   bool bHasAddToHistory;
};

class SaveCopyCommand : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Saves a copy of current project.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#save_copy";}
public:
   wxString mFileName;
};

class SaveLogCommand : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Saves the log contents.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#save_log";}
public:
   wxString mFileName;
};

class ClearLogCommand : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Clears the log contents.");};
   bool DefineParams( ShuttleParams & S ) override;
   bool PromptUser(wxWindow *parent) override;
   bool Apply(const CommandContext & context) override;

   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#Clear_log";}
public:
   wxString mFileName;
};
