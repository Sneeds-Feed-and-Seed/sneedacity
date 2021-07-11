/**********************************************************************

   Sneedacity: A Digital Audio Editor
   Sneedacity is copyright (c) 1999-2010 Audacity Team, copyright (c) 2021 Sneedacity Team.
   File License: wxwidgets

   BatchEvalCommand.h
   Dan Horgan

******************************************************************//**

\class BatchEvalCommand
\brief Given a string representing a command, pass it to the MacroCommands
system.

The eventual aim is to move the code from MacroCommands out into separate
command classes, but until this happens, BatchEvalCommand can act as a 'bridge'
to that system.

*//*******************************************************************/

#ifndef __BATCHEVALCOMMAND__
#define __BATCHEVALCOMMAND__

#include "Command.h"
#include "CommandType.h"
#include "../BatchCommands.h"

class SneedacityProject;

class BatchEvalCommandType final : public OldStyleCommandType
{
public:
   ComponentInterfaceSymbol BuildName() override;
   void BuildSignature(CommandSignature &signature) override;
   OldStyleCommandPointer Create( SneedacityProject *project,
      std::unique_ptr<CommandOutputTargets> &&target) override;
};

class BatchEvalCommand final : public CommandImplementation
{
public:
   BatchEvalCommand(SneedacityProject &project, OldStyleCommandType &type)
      : CommandImplementation(project, type)
   { }

   virtual ~BatchEvalCommand();
   bool Apply(const CommandContext &context) override;
};

#endif /* End of include guard: __BATCHEVALCOMMAND__ */
