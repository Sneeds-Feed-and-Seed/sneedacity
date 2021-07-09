/**********************************************************************

  Sneedacity: A Digital Audio Editor

  AutoDuck.cpp

  Markus Meyer

*******************************************************************//**

\class EffectAutoDuck
\brief Implements the Auto Ducking effect

\class AutoDuckRegion
\brief a struct that holds a start and end time.

*******************************************************************/


#include "AutoDuck.h"
#include "LoadEffects.h"

#include <math.h>
#include <float.h>

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/intl.h>

#include "../AColor.h"
#include "../AllThemeResources.h"
#include "../Prefs.h"
#include "../Shuttle.h"
#include "../ShuttleGui.h"
#include "../Theme.h"
#include "../widgets/valnum.h"

#include "../WaveTrack.h"
#include "../widgets/SneedacityMessageBox.h"

// Define keys, defaults, minimums, and maximums for the effect parameters
//
//     Name                Type     Key                     Def      Min      Max      Scale
Param( DuckAmountDb,       double,  wxT("DuckAmountDb"),     -12.0,   -24.0,   0.0,     1  );
Param( InnerFadeDownLen,   double,  wxT("InnerFadeDownLen"), 0.0,     0.0,     3.0,     1  );
Param( InnerFadeUpLen,     double,  wxT("InnerFadeUpLen"),   0.0,     0.0,     3.0,     1  );
Param( OuterFadeDownLen,   double,  wxT("OuterFadeDownLen"), 0.5,     0.0,     3.0,     1  );
Param( OuterFadeUpLen,     double,  wxT("OuterFadeUpLen"),   0.5,     0.0,     3.0,     1  );
Param( ThresholdDb,        double,  wxT("ThresholdDb"),      -30.0,   -100.0,  0.0,     1  );
Param( MaximumPause,       double,  wxT("MaximumPause"),     1.0,     0.0,     DBL_MAX, 1  );

/*
 * Common constants
 */

static const size_t kBufSize = 131072u;     // number of samples to process at once
static const size_t kRMSWindowSize = 100u;  // samples in circular RMS window buffer

/*
 * A auto duck region and an array of auto duck regions
 */

struct AutoDuckRegion
{
   AutoDuckRegion(double t0, double t1)
   {
      this->t0 = t0;
      this->t1 = t1;
   }

   double t0;
   double t1;
};

/*
 * Effect implementation
 */

const ComponentInterfaceSymbol EffectAutoDuck::Symbol
{ XO("Auto Duck") };

namespace{ BuiltinEffectsModule::Registration< EffectAutoDuck > reg; }

BEGIN_EVENT_TABLE(EffectAutoDuck, wxEvtHandler)
   EVT_TEXT(wxID_ANY, EffectAutoDuck::OnValueChanged)
END_EVENT_TABLE()

EffectAutoDuck::EffectAutoDuck()
{
   mDuckAmountDb = DEF_DuckAmountDb;
   mInnerFadeDownLen = DEF_InnerFadeDownLen;
   mInnerFadeUpLen = DEF_InnerFadeUpLen;
   mOuterFadeDownLen = DEF_OuterFadeDownLen;
   mOuterFadeUpLen = DEF_OuterFadeUpLen;
   mThresholdDb = DEF_ThresholdDb;
   mMaximumPause = DEF_MaximumPause;

   SetLinearEffectFlag(true);

   mControlTrack = NULL;

   mPanel = NULL;
}

EffectAutoDuck::~EffectAutoDuck()
{
}

// ComponentInterface implementation

ComponentInterfaceSymbol EffectAutoDuck::GetSymbol()
{
   return Symbol;
}

TranslatableString EffectAutoDuck::GetDescription()
{
   return XO("Reduces (ducks) the volume of one or more tracks whenever the volume of a specified \"control\" track reaches a particular level");
}

ManualPageID EffectAutoDuck::ManualPage()
{
   return L"Auto_Duck";
}

// EffectDefinitionInterface implementation

EffectType EffectAutoDuck::GetType()
{
   return EffectTypeProcess;
}

// EffectClientInterface implementation
bool EffectAutoDuck::DefineParams( ShuttleParams & S ){
   S.SHUTTLE_PARAM(  mDuckAmountDb, DuckAmountDb);
   S.SHUTTLE_PARAM(  mInnerFadeDownLen, InnerFadeDownLen);
   S.SHUTTLE_PARAM(  mInnerFadeUpLen, InnerFadeUpLen);
   S.SHUTTLE_PARAM(  mOuterFadeDownLen, OuterFadeDownLen);
   S.SHUTTLE_PARAM(  mOuterFadeUpLen, OuterFadeUpLen);
   S.SHUTTLE_PARAM(  mThresholdDb, ThresholdDb);
   S.SHUTTLE_PARAM(  mMaximumPause, MaximumPause);
   return true;
}

bool EffectAutoDuck::GetAutomationParameters(CommandParameters & parms)
{
   parms.Write(KEY_DuckAmountDb, mDuckAmountDb);
   parms.Write(KEY_InnerFadeDownLen, mInnerFadeDownLen);
   parms.Write(KEY_InnerFadeUpLen, mInnerFadeUpLen);
   parms.Write(KEY_OuterFadeDownLen, mOuterFadeDownLen);
   parms.Write(KEY_OuterFadeUpLen, mOuterFadeUpLen);
   parms.Write(KEY_ThresholdDb, mThresholdDb);
   parms.Write(KEY_MaximumPause, mMaximumPause);

   return true;
}

bool EffectAutoDuck::SetAutomationParameters(CommandParameters & parms)
{
   ReadAndVerifyDouble(DuckAmountDb);
   ReadAndVerifyDouble(InnerFadeDownLen);
   ReadAndVerifyDouble(InnerFadeUpLen);
   ReadAndVerifyDouble(OuterFadeDownLen);
   ReadAndVerifyDouble(OuterFadeUpLen);
   ReadAndVerifyDouble(ThresholdDb);
   ReadAndVerifyDouble(MaximumPause);

   mDuckAmountDb = DuckAmountDb;
   mInnerFadeDownLen = InnerFadeDownLen;
   mInnerFadeUpLen = InnerFadeUpLen;
   mOuterFadeDownLen = OuterFadeDownLen;
   mOuterFadeUpLen = OuterFadeUpLen;
   mThresholdDb = ThresholdDb;
   mMaximumPause = MaximumPause;

   return true;
}

// Effect implementation

bool EffectAutoDuck::Startup()
{
   wxString base = wxT("/Effects/AutoDuck/");

   // Migrate settings from 2.1.0 or before

   // Already migrated, so bail
   if (gPrefs->Exists(base + wxT("Migrated")))
   {
      return true;
   }

   // Load the old "current" settings
   if (gPrefs->Exists(base))
   {
      gPrefs->Read(base + wxT("DuckAmountDb"), &mDuckAmountDb, DEF_DuckAmountDb);
      gPrefs->Read(base + wxT("InnerFadeDownLen"), &mInnerFadeDownLen, DEF_InnerFadeDownLen);
      gPrefs->Read(base + wxT("InnerFadeUpLen"), &mInnerFadeUpLen, DEF_InnerFadeUpLen);
      gPrefs->Read(base + wxT("OuterFadeDownLen"), &mOuterFadeDownLen, DEF_OuterFadeDownLen);
      gPrefs->Read(base + wxT("OuterFadeUpLen"), &mOuterFadeUpLen, DEF_OuterFadeUpLen);
      gPrefs->Read(base + wxT("ThresholdDb"), &mThresholdDb, DEF_ThresholdDb);
      gPrefs->Read(base + wxT("MaximumPause"), &mMaximumPause, DEF_MaximumPause);

      SaveUserPreset(GetCurrentSettingsGroup());
      
      // Do not migrate again
      gPrefs->Write(base + wxT("Migrated"), true);
      gPrefs->Flush();
   }

   return true;
}

bool EffectAutoDuck::Init()
{
   mControlTrack = NULL;

   bool lastWasSelectedWaveTrack = false;
   const WaveTrack *controlTrackCandidate = NULL;

   for (auto t : inputTracks()->Any())
   {
      if (lastWasSelectedWaveTrack && !t->GetSelected()) {
         // This could be the control track, so remember it
         controlTrackCandidate = track_cast<const WaveTrack *>(t);
      }

      lastWasSelectedWaveTrack = false;

      if (t->GetSelected()) {
         bool ok = t->TypeSwitch<bool>(
            [&](const WaveTrack *) {
               lastWasSelectedWaveTrack = true;
               return true;
            },
            [&](const Track *) {
               Effect::MessageBox(
                  /* i18n-hint: Auto duck is the name of an effect that 'ducks' (reduces the volume)
                   * of the audio automatically when there is sound on another track.  Not as
                   * in 'Donald-Duck'!*/
                  XO("You selected a track which does not contain audio. AutoDuck can only process audio tracks."),
                  wxICON_ERROR );
               return false;
            }
         );
         if (!ok)
            return false;
      }
   }

   if (!controlTrackCandidate)
   {
      Effect::MessageBox(
         /* i18n-hint: Auto duck is the name of an effect that 'ducks' (reduces the volume)
          * of the audio automatically when there is sound on another track.  Not as
          * in 'Donald-Duck'!*/
         XO("Auto Duck needs a control track which must be placed below the selected track(s)."),
         wxICON_ERROR );
      return false;
   }

   mControlTrack = controlTrackCandidate;

   return true;
}

void EffectAutoDuck::End()
{
   mControlTrack = NULL;
}

bool EffectAutoDuck::Process()
{
   if (GetNumWaveTracks() == 0 || !mControlTrack)
      return false;

   bool cancel = false;

   auto start =
      mControlTrack->TimeToLongSamples(mT0 + mOuterFadeDownLen);
   auto end =
      mControlTrack->TimeToLongSamples(mT1 - mOuterFadeUpLen);

   if (end <= start)
      return false;

   // the minimum number of samples we have to wait until the maximum
   // pause has been exceeded
   double maxPause = mMaximumPause;

   // We don't fade in until we have time enough to actually fade out again
   if (maxPause < mOuterFadeDownLen + mOuterFadeUpLen)
      maxPause = mOuterFadeDownLen + mOuterFadeUpLen;

   auto minSamplesPause =
      mControlTrack->TimeToLongSamples(maxPause);

   double threshold = DB_TO_LINEAR(mThresholdDb);

   // adjust the threshold so we can compare it to the rmsSum value
   threshold = threshold * threshold * kRMSWindowSize;

   int rmsPos = 0;
   float rmsSum = 0;
   // to make the progress bar appear more natural, we first look for all
   // duck regions and apply them all at once afterwards
   std::vector<AutoDuckRegion> regions;
   bool inDuckRegion = false;
   {
      Floats rmsWindow{ kRMSWindowSize, true };

      Floats buf{ kBufSize };

      // initialize the following two variables to prevent compiler warning
      double duckRegionStart = 0;
      sampleCount curSamplesPause = 0;

      auto pos = start;

      while (pos < end)
      {
         const auto len = limitSampleBufferSize( kBufSize, end - pos );
         
         mControlTrack->GetFloats(buf.get(), pos, len);

         for (auto i = pos; i < pos + len; i++)
         {
            rmsSum -= rmsWindow[rmsPos];
            // i - pos is bounded by len:
            auto index = ( i - pos ).as_size_t();
            rmsWindow[rmsPos] = buf[ index ] * buf[ index ];
            rmsSum += rmsWindow[rmsPos];
            rmsPos = (rmsPos + 1) % kRMSWindowSize;

            bool thresholdExceeded = rmsSum > threshold;

            if (thresholdExceeded)
            {
               // everytime the threshold is exceeded, reset our count for
               // the number of pause samples
               curSamplesPause = 0;

               if (!inDuckRegion)
               {
                  // the threshold has been exceeded for the first time, so
                  // let the duck region begin here
                  inDuckRegion = true;
                  duckRegionStart = mControlTrack->LongSamplesToTime(i);
               }
            }

            if (!thresholdExceeded && inDuckRegion)
            {
               // the threshold has not been exceeded and we are in a duck
               // region, but only fade in if the maximum pause has been
               // exceeded
               curSamplesPause += 1;

               if (curSamplesPause >= minSamplesPause)
               {
                  // do the actual duck fade and reset all values
                  double duckRegionEnd =
                     mControlTrack->LongSamplesToTime(i - curSamplesPause);

                  regions.push_back(AutoDuckRegion(
                     duckRegionStart - mOuterFadeDownLen,
                     duckRegionEnd + mOuterFadeUpLen));

                  inDuckRegion = false;
               }
            }
         }

         pos += len;

         if (TotalProgress(
            (pos - start).as_double() /
            (end - start).as_double() /
            (GetNumWaveTracks() + 1)
         ))
         {
            cancel = true;
            break;
         }
      }

      // apply last duck fade, if any
      if (inDuckRegion)
      {
         double duckRegionEnd =
            mControlTrack->LongSamplesToTime(end - curSamplesPause);
         regions.push_back(AutoDuckRegion(
            duckRegionStart - mOuterFadeDownLen,
            duckRegionEnd + mOuterFadeUpLen));
      }
   }

   if (!cancel)
   {
      CopyInputTracks(); // Set up mOutputTracks.

      int trackNum = 0;

      for( auto iterTrack : mOutputTracks->Selected< WaveTrack >() )
      {
         for (size_t i = 0; i < regions.size(); i++)
         {
            const AutoDuckRegion& region = regions[i];
            if (ApplyDuckFade(trackNum, iterTrack, region.t0, region.t1))
            {
               cancel = true;
               break;
            }
         }

         if (cancel)
            break;

         trackNum++;
      }
   }

   ReplaceProcessedTracks(!cancel);
   return !cancel;
}

void EffectAutoDuck::PopulateOrExchange(ShuttleGui & S)
{
   S.SetBorder(5);
   S.StartVerticalLay(true);
   {
      S.AddSpace(0, 5);

      mPanel = safenew EffectAutoDuckPanel(S.GetParent(), wxID_ANY, this);
      S.AddWindow(mPanel);

      S.AddSpace(0, 5);

      S.StartMultiColumn(6, wxCENTER);
      {
         mDuckAmountDbBox = S.Validator<FloatingPointValidator<double>>(
               1, &mDuckAmountDb, NumValidatorStyle::NO_TRAILING_ZEROES,
               MIN_DuckAmountDb, MAX_DuckAmountDb
            )
            .NameSuffix(XO("db"))
            .AddTextBox(XXO("Duck &amount:"), wxT(""), 10);
         S.AddUnits(XO("dB"));

         mMaximumPauseBox = S.Validator<FloatingPointValidator<double>>(
               2, &mMaximumPause, NumValidatorStyle::NO_TRAILING_ZEROES,
               MIN_MaximumPause, MAX_MaximumPause
            )
            .NameSuffix(XO("seconds"))
            .AddTextBox(XXO("Ma&ximum pause:"), wxT(""), 10);
         S.AddUnits(XO("seconds"));

         mOuterFadeDownLenBox = S.Validator<FloatingPointValidator<double>>(
               2, &mOuterFadeDownLen, NumValidatorStyle::NO_TRAILING_ZEROES,
               MIN_OuterFadeDownLen, MAX_OuterFadeDownLen
            )
            .NameSuffix(XO("seconds"))
            .AddTextBox(XXO("Outer fade &down length:"), wxT(""), 10);
         S.AddUnits(XO("seconds"));

         mOuterFadeUpLenBox = S.Validator<FloatingPointValidator<double>>(
               2, &mOuterFadeUpLen, NumValidatorStyle::NO_TRAILING_ZEROES,
               MIN_OuterFadeUpLen, MAX_OuterFadeUpLen
            )
            .NameSuffix(XO("seconds"))
            .AddTextBox(XXO("Outer fade &up length:"), wxT(""), 10);
         S.AddUnits(XO("seconds"));

         mInnerFadeDownLenBox = S.Validator<FloatingPointValidator<double>>(
               2, &mInnerFadeDownLen, NumValidatorStyle::NO_TRAILING_ZEROES,
               MIN_InnerFadeDownLen, MAX_InnerFadeDownLen
            )
            .NameSuffix(XO("seconds"))
            .AddTextBox(XXO("Inner fade d&own length:"), wxT(""), 10);
         S.AddUnits(XO("seconds"));

         mInnerFadeUpLenBox = S.Validator<FloatingPointValidator<double>>(
               2, &mInnerFadeUpLen, NumValidatorStyle::NO_TRAILING_ZEROES,
               MIN_InnerFadeUpLen, MAX_InnerFadeUpLen
            )
            .NameSuffix(XO("seconds"))
            .AddTextBox(XXO("Inner &fade up length:"), wxT(""), 10);
         S.AddUnits(XO("seconds"));
      }
      S.EndMultiColumn();

      S.StartMultiColumn(3, wxCENTER);
      {
         mThresholdDbBox = S.Validator<FloatingPointValidator<double>>(
               2, &mThresholdDb, NumValidatorStyle::NO_TRAILING_ZEROES,
               MIN_ThresholdDb, MAX_ThresholdDb
            )
            .NameSuffix(XO("db"))
            .AddTextBox(XXO("&Threshold:"), wxT(""), 10);
         S.AddUnits(XO("dB"));
      }
      S.EndMultiColumn();

   }
   S.EndVerticalLay();

   return;
}

bool EffectAutoDuck::TransferDataToWindow()
{
   if (!mUIParent->TransferDataToWindow())
   {
      return false;
   }

   mPanel->Refresh(false);

   return true;
}

bool EffectAutoDuck::TransferDataFromWindow()
{
   if (!mUIParent->Validate() || !mUIParent->TransferDataFromWindow())
   {
      return false;
   }

   return true;
}

// EffectAutoDuck implementation

// this currently does an exponential fade
bool EffectAutoDuck::ApplyDuckFade(int trackNum, WaveTrack* t,
                                   double t0, double t1)
{
   bool cancel = false;

   auto start = t->TimeToLongSamples(t0);
   auto end = t->TimeToLongSamples(t1);

   Floats buf{ kBufSize };
   auto pos = start;

   auto fadeDownSamples = t->TimeToLongSamples(
      mOuterFadeDownLen + mInnerFadeDownLen);
   if (fadeDownSamples < 1)
      fadeDownSamples = 1;

   auto fadeUpSamples = t->TimeToLongSamples(
      mOuterFadeUpLen + mInnerFadeUpLen);
   if (fadeUpSamples < 1)
      fadeUpSamples = 1;

   float fadeDownStep = mDuckAmountDb / fadeDownSamples.as_double();
   float fadeUpStep = mDuckAmountDb / fadeUpSamples.as_double();

   while (pos < end)
   {
      const auto len = limitSampleBufferSize( kBufSize, end - pos );

      t->GetFloats(buf.get(), pos, len);

      for (auto i = pos; i < pos + len; i++)
      {
         float gainDown = fadeDownStep * (i - start).as_float();
         float gainUp = fadeUpStep * (end - i).as_float();

         float gain;
         if (gainDown > gainUp)
            gain = gainDown;
         else
            gain = gainUp;
         if (gain < mDuckAmountDb)
            gain = mDuckAmountDb;

         // i - pos is bounded by len:
         buf[ ( i - pos ).as_size_t() ] *= DB_TO_LINEAR(gain);
      }

      t->Set((samplePtr)buf.get(), floatSample, pos, len);

      pos += len;

      float curTime = t->LongSamplesToTime(pos);
      float fractionFinished = (curTime - mT0) / (mT1 - mT0);
      if (TotalProgress( (trackNum + 1 + fractionFinished) /
                         (GetNumWaveTracks() + 1) ))
      {
         cancel = true;
         break;
      }
   }

   return cancel;
}

void EffectAutoDuck::OnValueChanged(wxCommandEvent & WXUNUSED(evt))
{
   mPanel->Refresh(false);
}

/*
 * EffectAutoDuckPanel implementation
 */

#define CONTROL_POINT_REGION 10 // pixel distance to click on a control point
#define CONTROL_POINT_MIN_MOVE 5 // min mouse move until value is changed

#define TEXT_DISTANCE 15 // pixel distance text <-> center of control point

#define FADE_DOWN_START 150 // x coordinate
#define FADE_UP_START 450 // x coordinate
#define DUCK_AMOUNT_START 50 // y coordinate

#define FADE_SCALE 40 // scale factor for second -> pixel conversion
#define DUCK_AMOUNT_SCALE 8 // scale factor for db -> pixel conversion

static int GetDistance(const wxPoint& first, const wxPoint& second)
{
   int distanceX = abs(first.x - second.x);
   int distanceY = abs(first.y - second.y);
   if (distanceX > distanceY)
      return distanceX;
   else
      return distanceY;
}

BEGIN_EVENT_TABLE(EffectAutoDuckPanel, wxPanelWrapper)
   EVT_PAINT(EffectAutoDuckPanel::OnPaint)
   EVT_MOUSE_CAPTURE_CHANGED(EffectAutoDuckPanel::OnMouseCaptureChanged)
   EVT_MOUSE_CAPTURE_LOST(EffectAutoDuckPanel::OnMouseCaptureLost)
   EVT_LEFT_DOWN(EffectAutoDuckPanel::OnLeftDown)
   EVT_LEFT_UP(EffectAutoDuckPanel::OnLeftUp)
   EVT_MOTION(EffectAutoDuckPanel::OnMotion)
END_EVENT_TABLE()

EffectAutoDuckPanel::EffectAutoDuckPanel(
   wxWindow *parent, wxWindowID winid, EffectAutoDuck *effect)
:  wxPanelWrapper(parent, winid, wxDefaultPosition, wxSize(600, 300))
{
   mParent = parent;
   mEffect = effect;
   mCurrentControlPoint = none;
   mBackgroundBitmap = NULL;

   ResetControlPoints();
}

EffectAutoDuckPanel::~EffectAutoDuckPanel()
{
   if(HasCapture())
      ReleaseMouse();
}

void EffectAutoDuckPanel::ResetControlPoints()
{
   mControlPoints[innerFadeDown] = wxPoint(-100,-100);
   mControlPoints[innerFadeUp] = wxPoint(-100,-100);
   mControlPoints[outerFadeDown] = wxPoint(-100,-100);
   mControlPoints[outerFadeUp] = wxPoint(-100,-100);
   mControlPoints[duckAmount] = wxPoint(-100,-100);
}

void EffectAutoDuckPanel::OnPaint(wxPaintEvent & WXUNUSED(evt))
{
   int clientWidth, clientHeight;
   GetSize(&clientWidth, &clientHeight);

   if (!mBackgroundBitmap || mBackgroundBitmap->GetWidth() != clientWidth ||
       mBackgroundBitmap->GetHeight() != clientHeight)
   {
      mBackgroundBitmap = std::make_unique<wxBitmap>(clientWidth, clientHeight,24);
   }

   wxMemoryDC dc;
   dc.SelectObject(*mBackgroundBitmap);

   dc.SetBrush(*wxWHITE_BRUSH);
   dc.SetPen(*wxBLACK_PEN);
   dc.DrawRectangle(0, 0, clientWidth, clientHeight);

   dc.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
                     wxFONTWEIGHT_NORMAL));
   dc.SetTextForeground(*wxBLACK);
   dc.SetTextBackground(*wxWHITE);

   double duckAmountDb = 0;
   double innerFadeDownLen = 0;
   double innerFadeUpLen = 0;
   double outerFadeDownLen = 0;
   double outerFadeUpLen = 0;
   mEffect->mDuckAmountDbBox->GetValue().ToDouble(&duckAmountDb);
   mEffect->mInnerFadeDownLenBox->GetValue().ToDouble(&innerFadeDownLen);
   mEffect->mInnerFadeUpLenBox->GetValue().ToDouble(&innerFadeUpLen);
   mEffect->mOuterFadeDownLenBox->GetValue().ToDouble(&outerFadeDownLen);
   mEffect->mOuterFadeUpLenBox->GetValue().ToDouble(&outerFadeUpLen);

   if (innerFadeDownLen < MIN_InnerFadeDownLen || innerFadeDownLen > MAX_InnerFadeDownLen ||
       innerFadeUpLen < MIN_InnerFadeUpLen     || innerFadeUpLen > MAX_InnerFadeUpLen     ||
       outerFadeDownLen < MIN_OuterFadeDownLen || outerFadeDownLen > MAX_OuterFadeDownLen ||
       outerFadeUpLen < MIN_OuterFadeUpLen     || outerFadeUpLen > MAX_OuterFadeUpLen     ||
       duckAmountDb < MIN_DuckAmountDb         || duckAmountDb > MAX_DuckAmountDb)
   {
      // values are out of range, no preview available
      wxString message = _("Preview not available");
      int textWidth = 0, textHeight = 0;
      dc.GetTextExtent(message, &textWidth, &textHeight);
      dc.DrawText(message, (clientWidth - textWidth) / 2,
                           (clientHeight - textHeight) / 2);

      ResetControlPoints();
   } else
   {
      // draw preview
      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.SetPen(wxPen(theTheme.Colour(clrGraphLines), 3, wxPENSTYLE_SOLID));

      wxPoint points[6];

      points[0].x = 10;
      points[0].y = DUCK_AMOUNT_START;

      points[1].x = FADE_DOWN_START - (int)(outerFadeDownLen * FADE_SCALE);
      points[1].y = DUCK_AMOUNT_START;

      points[2].x = FADE_DOWN_START + (int)(innerFadeDownLen * FADE_SCALE);
      points[2].y = DUCK_AMOUNT_START -
         (int)(duckAmountDb * DUCK_AMOUNT_SCALE);

      points[3].x = FADE_UP_START - (int)(innerFadeUpLen * FADE_SCALE);
      points[3].y = DUCK_AMOUNT_START -
         (int)(duckAmountDb * DUCK_AMOUNT_SCALE);

      points[4].x = FADE_UP_START + (int)(outerFadeUpLen * FADE_SCALE);
      points[4].y = DUCK_AMOUNT_START;

      points[5].x = clientWidth - 10;
      points[5].y = DUCK_AMOUNT_START;

      AColor::Lines(dc, 6, points);

      dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_DOT));

      AColor::Line(dc, FADE_DOWN_START, 10, FADE_DOWN_START, clientHeight - 10);
      AColor::Line(dc, FADE_UP_START, 10, FADE_UP_START, clientHeight - 10);

      dc.SetPen(AColor::envelopePen);
      dc.SetBrush(*wxWHITE_BRUSH);

      mControlPoints[outerFadeDown] = points[1];
      mControlPoints[innerFadeDown] = points[2];
      mControlPoints[innerFadeUp] = points[3];
      mControlPoints[outerFadeUp] = points[4];
      mControlPoints[duckAmount] = wxPoint(
         (points[2].x + points[3].x) / 2, points[2].y);

      for (int i = 0; i < AUTO_DUCK_PANEL_NUM_CONTROL_POINTS; i++)
      {
         EControlPoint cp = (EControlPoint)i;
         int digits;
         float value;

         if (cp == innerFadeDown)
         {
            value = innerFadeDownLen;
            digits = 2;
         }
         else if (cp == innerFadeUp)
         {
            value = innerFadeUpLen;
            digits = 2;
         }
         else if (cp == outerFadeDown)
         {
            value = outerFadeDownLen;
            digits = 2;
         } else if (cp == outerFadeUp)
         {
            value = outerFadeUpLen;
            digits = 2;
         }
         else
         {
            value = duckAmountDb;
            digits = 1;
         }

         wxString valueStr = Internat::ToDisplayString(value, digits);
         valueStr += wxT(" ");

         if (cp == duckAmount)
            /* i18n-hint: short form of 'decibels'.*/
            valueStr += _("dB");
         else
            /* i18n-hint: short form of 'seconds'.*/
            valueStr += _("s");

         int textWidth = 0, textHeight = 0;
         GetTextExtent(valueStr, &textWidth, &textHeight);

         int textPosX = mControlPoints[i].x - textWidth / 2;
         int textPosY = mControlPoints[i].y;

         if (cp == duckAmount || cp == outerFadeDown || cp == outerFadeUp)
            textPosY -= TEXT_DISTANCE + textHeight;
         else
            textPosY += TEXT_DISTANCE;

         dc.DrawText(valueStr, textPosX, textPosY);

         dc.DrawEllipse(mControlPoints[i].x - 3,
                        mControlPoints[i].y - 3, 6, 6);
      }
   }

   // copy background buffer to paint dc
   wxPaintDC paintDC(this);
   paintDC.Blit(0, 0, clientWidth, clientHeight, &dc, 0, 0);

   // clean up: necessary to free resources on Windows
   dc.SetPen(wxNullPen);
   dc.SetBrush(wxNullBrush);
   dc.SetFont(wxNullFont);
   dc.SelectObject(wxNullBitmap);
}

void EffectAutoDuckPanel::OnMouseCaptureChanged(
   wxMouseCaptureChangedEvent & WXUNUSED(evt))
{
   SetCursor(wxNullCursor);
   mCurrentControlPoint = none;
}

void EffectAutoDuckPanel::OnMouseCaptureLost(
   wxMouseCaptureLostEvent & WXUNUSED(evt))
{
   mCurrentControlPoint = none;

   if (HasCapture())
   {
      ReleaseMouse();
   }
}

EffectAutoDuckPanel::EControlPoint
   EffectAutoDuckPanel::GetNearestControlPoint(const wxPoint & pt)
{
   int dist[AUTO_DUCK_PANEL_NUM_CONTROL_POINTS];
   int i;

   for (i = 0; i < AUTO_DUCK_PANEL_NUM_CONTROL_POINTS; i++)
      dist[i] = GetDistance(pt, mControlPoints[i]);

   int curMinimum = 0;
   for (i = 0; i < AUTO_DUCK_PANEL_NUM_CONTROL_POINTS; i++)
      if (dist[i] < dist[curMinimum])
         curMinimum = i;

   if (dist[curMinimum] <= CONTROL_POINT_REGION)
      return (EControlPoint)curMinimum;
   else
      return none;
}

void EffectAutoDuckPanel::OnLeftDown(wxMouseEvent & evt)
{
   EControlPoint nearest = GetNearestControlPoint(evt.GetPosition());

   if (nearest != none)
   {
      // this control point has been clicked
      mMouseDownPoint = evt.GetPosition();

      mCurrentControlPoint = nearest;
      mControlPointMoveActivated = false;

      for (int i = 0; i < AUTO_DUCK_PANEL_NUM_CONTROL_POINTS; i++)
         mMoveStartControlPoints[i] = mControlPoints[i];

      if( !HasCapture() )
         CaptureMouse();
   }
}

void EffectAutoDuckPanel::OnLeftUp(wxMouseEvent & WXUNUSED(evt))
{
   if (mCurrentControlPoint != none)
   {
      mCurrentControlPoint = none;
      ReleaseMouse();
   }
}

void EffectAutoDuckPanel::OnMotion(wxMouseEvent & evt)
{
   switch (GetNearestControlPoint(evt.GetPosition()))
   {
   case none:
      SetCursor(wxNullCursor);
      break;
   case innerFadeDown:
   case innerFadeUp:
   case outerFadeDown:
   case outerFadeUp:
      SetCursor(wxCursor(wxCURSOR_SIZEWE));
      break;
   case duckAmount:
      SetCursor(wxCursor(wxCURSOR_SIZENS));
      break;
   }

   if (mCurrentControlPoint != none)
   {
      if (!mControlPointMoveActivated)
      {
         int dist;

         if (mCurrentControlPoint == duckAmount)
            dist = abs(evt.GetY() - mMouseDownPoint.y);
         else
            dist = abs(evt.GetX() - mMouseDownPoint.x);

         if (dist >= CONTROL_POINT_MIN_MOVE)
            mControlPointMoveActivated = true;
      }

      if (mControlPointMoveActivated)
      {
         float newValue;

         switch (mCurrentControlPoint)
         {
         case outerFadeDown:
            newValue = ((double)(FADE_DOWN_START - evt.GetX())) / FADE_SCALE;
            mEffect->mOuterFadeDownLen = TrapDouble(newValue, MIN_OuterFadeDownLen, MAX_OuterFadeDownLen);
            break;
         case outerFadeUp:
            newValue = ((double)(evt.GetX() - FADE_UP_START)) / FADE_SCALE;
            mEffect->mOuterFadeUpLen = TrapDouble(newValue, MIN_OuterFadeUpLen, MAX_OuterFadeUpLen);
            break;
         case innerFadeDown:
            newValue = ((double)(evt.GetX() - FADE_DOWN_START)) / FADE_SCALE;
            mEffect->mInnerFadeDownLen = TrapDouble(newValue, MIN_InnerFadeDownLen, MAX_InnerFadeDownLen);
            break;
         case innerFadeUp:
            newValue = ((double)(FADE_UP_START - evt.GetX())) / FADE_SCALE;
            mEffect->mInnerFadeUpLen = TrapDouble(newValue, MIN_InnerFadeUpLen, MAX_InnerFadeUpLen);
            break;
         case duckAmount:
            newValue = ((double)(DUCK_AMOUNT_START - evt.GetY())) / DUCK_AMOUNT_SCALE;
            mEffect->mDuckAmountDb = TrapDouble(newValue, MIN_DuckAmountDb, MAX_DuckAmountDb);
            break;
         case none:
            wxASSERT(false); // should not happen
         }
         mEffect->TransferDataToWindow();
         Refresh(false);
      }
   }
}
