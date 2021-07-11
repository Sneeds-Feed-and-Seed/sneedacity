/**********************************************************************

   Sneedacity - A Digital Audio Editor
   Copyright 1999-2009 Audacity Team, 2021 Sneedacity Team
   License: wxwidgets

   Dan Horgan
   Marty Goddard
******************************************************************//**

\file GetTrackInfoCommand.h
\brief Declarations of GetTrackInfoCommand and GetTrackInfoCommandType classes

*//*******************************************************************/

#ifndef __GETTRACKINFOCOMMAND__
#define __GETTRACKINFOCOMMAND__

#include "Command.h"
#include "CommandType.h"

class GetTrackInfoCommand final : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   GetTrackInfoCommand();
   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Gets track values as JSON.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;

   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Tools#get_track_info";}

   bool Apply(const CommandContext &context ) override;
public:
   int mInfoType;
};

#endif /* End of include guard: __GETTRACKINFOCOMMAND__ */
