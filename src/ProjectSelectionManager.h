/**********************************************************************

Sneedacity: A Digital Audio Editor

ProjectSelectionManager.cpp

Paul Licameli split from ProjectManager.cpp

**********************************************************************/

#ifndef __SNEEDACITY_PROJECT_SELECTION_MANAGER__
#define __SNEEDACITY_PROJECT_SELECTION_MANAGER__

#include "ClientData.h" // to inherit
#include "toolbars/SelectionBarListener.h" // to inherit
#include "toolbars/SpectralSelectionBarListener.h" // to inherit

class SneedacityProject;

class SNEEDACITY_DLL_API ProjectSelectionManager final
   : public ClientData::Base
   , public SelectionBarListener
   , public SpectralSelectionBarListener
   , public TimeToolBarListener
{
public:
   static ProjectSelectionManager &Get( SneedacityProject &project );
   static const ProjectSelectionManager &Get( const SneedacityProject &project );

   explicit ProjectSelectionManager( SneedacityProject &project );
   ProjectSelectionManager( const ProjectSelectionManager & ) PROHIBITED;
   ProjectSelectionManager &operator=(
      const ProjectSelectionManager & ) PROHIBITED;
   ~ProjectSelectionManager() override;

   // SelectionBarListener callback methods
   double AS_GetRate() override;
   void AS_SetRate(double rate) override;
   int AS_GetSnapTo() override;
   void AS_SetSnapTo(int snap) override;
   const NumericFormatSymbol & AS_GetSelectionFormat() override;
   void AS_SetSelectionFormat(const NumericFormatSymbol & format) override;
   const NumericFormatSymbol & TT_GetAudioTimeFormat() override;
   void TT_SetAudioTimeFormat(const NumericFormatSymbol & format) override;
   void AS_ModifySelection(double &start, double &end, bool done) override;

   // SpectralSelectionBarListener callback methods
   double SSBL_GetRate() const override;
   const NumericFormatSymbol & SSBL_GetFrequencySelectionFormatName() override;
   void SSBL_SetFrequencySelectionFormatName(
      const NumericFormatSymbol & formatName) override;
   const NumericFormatSymbol & SSBL_GetBandwidthSelectionFormatName() override;
   void SSBL_SetBandwidthSelectionFormatName(
      const NumericFormatSymbol & formatName) override;
   void SSBL_ModifySpectralSelection(
      double &bottom, double &top, bool done) override;

private:
   bool SnapSelection();

   SneedacityProject &mProject;
};

#endif
