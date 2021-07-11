/**********************************************************************

   Sneedacity - A Digital Audio Editor
   Copyright 1999-2018 Audacity Team, 2021 Sneedacity Team
   License: GPL v2 - see LICENSE.txt

   Dan Horgan
   James Crook

******************************************************************//**

\file SelectCommand.h
\brief Declarations for SelectCommand and SelectCommandType classes

*//*******************************************************************/

#ifndef __SELECT_COMMAND__
#define __SELECT_COMMAND__



#include "CommandType.h"
#include "Command.h"

//#include "../commands/SneedacityCommand.h"

class SelectTimeCommand : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Selects a time range.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_I#select_time";}

   bool bHasT0;
   bool bHasT1;
   bool bHasFromEnd;
   bool bHasRelativeSpec;

   double mT0;
   double mT1;
   int mRelativeTo;
   bool mFromEnd;
};

class SelectFrequenciesCommand : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Selects a frequency range.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;

   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_I#select_frequencies";}

   bool bHasBottom;
   bool bHasTop;

   double mBottom;
   double mTop;
};


class SelectTracksCommand : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Selects a range of tracks.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool Apply(const CommandContext & context) override;
   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_I#select_tracks";}

   bool bHasFirstTrack;
   bool bHasNumTracks;
   bool bHasMode;

   double mFirstTrack;
   double mNumTracks;
   int mMode;
};


class SelectCommand : public SneedacityCommand
{
public:
   static const ComponentInterfaceSymbol Symbol;

   // ComponentInterface overrides
   ComponentInterfaceSymbol GetSymbol() override {return Symbol;};
   TranslatableString GetDescription() override {return XO("Selects Audio.");};
   bool DefineParams( ShuttleParams & S ) override { 
      return 
         mSelTime.DefineParams(S) &&  
         mSelFreq.DefineParams(S) &&
         mSelTracks.DefineParams(S);
   };
   void PopulateOrExchange(ShuttleGui & S) override {
      mSelTime.PopulateOrExchange(S);
      mSelFreq.PopulateOrExchange(S);
      mSelTracks.PopulateOrExchange(S);
   };
   bool Apply(const CommandContext & context) override {
      return 
         mSelTime.Apply(context) &&  
         mSelFreq.Apply( context )&&
         mSelTracks.Apply(context);
   }
   // SneedacityCommand overrides
   ManualPageID ManualPage() override {return L"Extra_Menu:_Scriptables_II#select";}
private:
   SelectTimeCommand mSelTime;
   SelectFrequenciesCommand mSelFreq;
   SelectTracksCommand mSelTracks;


};

#endif /* End of include guard: __SELECT_COMMAND__ */
