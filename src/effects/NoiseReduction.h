/**********************************************************************

  Sneedacity: A Digital Audio Editor

  NoiseReduction.h

  Dominic Mazzoni
  Vaughan Johnson (Preview)
  Paul Licameli

**********************************************************************/

#ifndef __SNEEDACITY_EFFECT_NOISE_REDUCTION__
#define __SNEEDACITY_EFFECT_NOISE_REDUCTION__

#include "Effect.h"

class EffectNoiseReduction final : public Effect {
public:
   static const ComponentInterfaceSymbol Symbol;

   EffectNoiseReduction();
   virtual ~EffectNoiseReduction();

   using Effect::TrackProgress;

   // ComponentInterface implementation

   ComponentInterfaceSymbol GetSymbol() override;
   TranslatableString GetDescription() override;

   // EffectDefinitionInterface implementation

   EffectType GetType() override;

   // Effect implementation

//   using Effect::TrackProgress;

   bool ShowInterface( wxWindow &parent,
      const EffectDialogFactory &factory, bool forceModal = false) override;

   bool Init() override;
   bool CheckWhetherSkipEffect() override;
   bool Process() override;

   class Settings;
   class Statistics;
   class Dialog;

private:
   class Worker;
   friend class Dialog;

   std::unique_ptr<Settings> mSettings;
   std::unique_ptr<Statistics> mStatistics;
};

#endif
