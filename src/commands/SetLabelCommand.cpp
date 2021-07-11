/**********************************************************************

   Sneedacity - A Digital Audio Editor
   Copyright 1999-2018 Audacity Team, 2021 Sneedacity Team
   License: wxwidgets

   James Crook

******************************************************************//**

\file SetLabelCommand.cpp
\brief Definitions for SetLabelCommand

\class SetLabelCommand
\brief Command that sets label information

*//*******************************************************************/


#include "SetLabelCommand.h"

#include "LoadCommands.h"
#include "../ViewInfo.h"
#include "../WaveTrack.h"
#include "../LabelTrack.h"
#include "../ProjectHistory.h"
#include "../Shuttle.h"
#include "../ShuttleGui.h"
#include "CommandContext.h"
#include "../tracks/labeltrack/ui/LabelTrackView.h"

const ComponentInterfaceSymbol SetLabelCommand::Symbol
{ XO("Set Label") };

namespace{ BuiltinCommandsModule::Registration< SetLabelCommand > reg; }

SetLabelCommand::SetLabelCommand()
{
}


bool SetLabelCommand::DefineParams( ShuttleParams & S ){ 
   S.Define(    mLabelIndex,                            wxT("Label"), 0, 0, 100 );
   S.OptionalY( bHasText       ).Define(  mText,        wxT("Text"),       wxT("empty") );
   S.OptionalY( bHasT0         ).Define(  mT0,          wxT("Start"),      0.0, 0.0, 100000.0);
   S.OptionalY( bHasT1         ).Define(  mT1,          wxT("End"),        0.0, 0.0, 100000.0);
   S.OptionalN( bHasSelected   ).Define(  mbSelected,   wxT("Selected"),   false );
   return true;
};

void SetLabelCommand::PopulateOrExchange(ShuttleGui & S)
{
   S.AddSpace(0, 5);

   S.StartMultiColumn(2, wxALIGN_CENTER);
   {
      S.TieNumericTextBox( XXO("Label Index"), mLabelIndex );
   }
   S.EndMultiColumn();
   S.StartMultiColumn(3, wxALIGN_CENTER);
   {
      S.Optional( bHasText      ).TieTextBox(         XXO("Text:"),     mText );
      S.Optional( bHasT0        ).TieNumericTextBox(  XXO("Start:"),    mT0 );
      S.Optional( bHasT1        ).TieNumericTextBox(  XXO("End:"),      mT1 );
      S.Optional( bHasSelected  ).TieCheckBox(        XXO("Selected"),  mbSelected );
   }
   S.EndMultiColumn();
}

bool SetLabelCommand::Apply(const CommandContext & context)
{
   // \todo we have similar code for finding the nth Label, Clip, Track etc.
   // this code could be put in subroutines/reduced.

   //wxString mode = GetString(wxT("Type"));
   SneedacityProject * p = &context.project;
   auto &tracks = TrackList::Get( *p );
   auto &selectedRegion = ViewInfo::Get( *p ).selectedRegion;
   const LabelStruct * pLabel = nullptr;
   LabelTrack *labelTrack = nullptr;
   auto ii = mLabelIndex;
   if ( mLabelIndex >= 0 ) {
      for (auto lt : tracks.Any<LabelTrack>()) {
         const auto &labels = lt->GetLabels();
         const auto nLabels = labels.size();
         if( ii >= (int)nLabels )
            ii -= nLabels;
         else {
            labelTrack = lt;
            pLabel = &labels[ ii ];
            break;
         }
      }
   }

   if ( !pLabel )
   {
      context.Error(wxT("LabelIndex was invalid."));
      return false;
   }
   auto newLabel = *pLabel;
   if( bHasText )
      newLabel.title = mText;
   if( bHasT0 )
      newLabel.selectedRegion.setT0(mT0, false);
   if( bHasT1 )
      newLabel.selectedRegion.setT1(mT1, false);
   if( bHasT0 || bHasT1 )
      newLabel.selectedRegion.ensureOrdering();
   labelTrack->SetLabel( ii, newLabel );

   // Only one label can be selected.
   if( bHasSelected ) {
      auto &view = LabelTrackView::Get( *labelTrack );
      if( mbSelected )
      {
         view.SetSelectedIndex( ii );
         double t0 = pLabel->selectedRegion.t0();
         double t1 = pLabel->selectedRegion.t1();
         selectedRegion.setTimes( t0, t1);
      }
      else if( view.GetSelectedIndex( context.project ) == ii )
         view.SetSelectedIndex( -1 );
   }

   labelTrack->SortLabels();

   ProjectHistory::Get(context.project).PushState(
      XO("Edited Label"), XO("Label"));

   return true;
}
