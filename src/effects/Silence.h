/**********************************************************************

  Sneedacity: A Digital Audio Editor

  Silence.h

  Dominic Mazzoni

  An effect to add silence.

**********************************************************************/

#ifndef __SNEEDACITY_EFFECT_SILENCE__
#define __SNEEDACITY_EFFECT_SILENCE__

#include "Generator.h"

class NumericTextCtrl;

class EffectSilence final : public Generator
{
public:
   static const ComponentInterfaceSymbol Symbol;

   EffectSilence();
   virtual ~EffectSilence();

   // ComponentInterface implementation

   ComponentInterfaceSymbol GetSymbol() override;
   TranslatableString GetDescription() override;
   ManualPageID ManualPage() override;

   // EffectDefinitionInterface implementation

   EffectType GetType() override;

   // Effect implementation

   void PopulateOrExchange(ShuttleGui & S) override;
   bool TransferDataToWindow() override;
   bool TransferDataFromWindow() override;

protected:
   // Generator implementation

   bool GenerateTrack(WaveTrack *tmp, const WaveTrack &track, int ntrack) override;

private:
   NumericTextCtrl *mDurationT;
};

#endif
