/*!********************************************************************

Sneedacity: A Digital Audio Editor

@file ProjectRate.h
@brief an object holding per-project preferred sample rate

Paul Licameli split from ProjectSettings.h

**********************************************************************/

#ifndef __SNEEDACITY_PROJECT_RATE__
#define __SNEEDACITY_PROJECT_RATE__

class SneedacityProject;

#include "ClientData.h"
#include "Observer.h"

///\brief Holds project sample rate
class ProjectRate final
   : public ClientData::Base
   , public Observer::Publisher<double>
{
public:
   static ProjectRate &Get( SneedacityProject &project );
   static const ProjectRate &Get( const SneedacityProject &project );
   
   explicit ProjectRate(SneedacityProject &project);
   ProjectRate( const ProjectRate & ) PROHIBITED;
   ProjectRate &operator=( const ProjectRate & ) PROHIBITED;

   void SetRate(double rate);
   double GetRate() const;

private:
   double mRate;
};

#endif

