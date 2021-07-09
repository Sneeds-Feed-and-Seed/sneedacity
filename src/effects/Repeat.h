/**********************************************************************

  Sneedacity: A Digital Audio Editor

  Repeat.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __SNEEDACITY_EFFECT_REPEAT__
#define __SNEEDACITY_EFFECT_REPEAT__

#include "Effect.h"

class wxTextCtrl;
class ShuttleGui;

class wxStaticText;

class EffectRepeat final : public Effect
{
public:
   static const ComponentInterfaceSymbol Symbol;

   EffectRepeat();
   virtual ~EffectRepeat();

   // ComponentInterface implementation

   ComponentInterfaceSymbol GetSymbol() override;
   TranslatableString GetDescription() override;
   ManualPageID ManualPage() override;

   // EffectDefinitionInterface implementation

   EffectType GetType() override;

   // EffectClientInterface implementation

   bool DefineParams( ShuttleParams & S ) override;
   bool GetAutomationParameters(CommandParameters & parms) override;
   bool SetAutomationParameters(CommandParameters & parms) override;

   // Effect implementation

   bool Process() override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool TransferDataToWindow() override;
   bool TransferDataFromWindow() override;

private:
   // EffectRepeat implementation

   void OnRepeatTextChange(wxCommandEvent & evt);
   void DisplayNewTime();

private:
   int repeatCount;

   wxTextCtrl   *mRepeatCount;
   wxStaticText *mCurrentTime;
   wxStaticText *mTotalTime;

   DECLARE_EVENT_TABLE()
};

#endif

