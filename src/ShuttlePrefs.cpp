/**********************************************************************

  Sneedacity: A Digital Audio Editor

  ShuttlePrefs.cpp

  Dominic Mazzoni
  James Crook

  Implements ShuttlePrefs

********************************************************************//*!

\class ShuttlePrefs

\brief
  A kind of Shuttle to exchange data with preferences e.g. the registry

  This class may be used by ShuttleGui to do the two step exchange,

\verbatim
     Gui -- Data -- Prefs
\endverbatim

*//*******************************************************************/


#include "ShuttlePrefs.h"

#include <variant>
#include <wx/defs.h>

#include "WrappedType.h"
#include "Prefs.h"

bool ShuttlePrefs::TransferBool( const wxString & Name, bool & bValue, const bool & bDefault )
{
   if( mbStoreInClient )
   {
      bValue = bDefault;
      gPrefs->Read( Name, &bValue );
   }
   else
   {
      return gPrefs->Write( Name, bValue );
   }
   return true;
}

bool ShuttlePrefs::TransferDouble( const wxString & Name, double & dValue, const double &dDefault )
{
   if( mbStoreInClient )
   {
      dValue = dDefault;
      gPrefs->Read( Name, &dValue );
   }
   else
   {
      return gPrefs->Write( Name, dValue );
   }
   return true;
}

bool ShuttlePrefs::TransferInt( const wxString & Name, int & iValue, const int &iDefault )
{
   if( mbStoreInClient )
   {
      iValue = iDefault;
      gPrefs->Read( Name, &iValue );
   }
   else
   {
      return gPrefs->Write( Name, iValue );
   }
   return true;
}

bool ShuttlePrefs::TransferString( const wxString & Name, wxString & strValue, const wxString &strDefault )
{
   if( mbStoreInClient )
   {
      strValue = strDefault;
      gPrefs->Read( Name, &strValue );
   }
   else
   {
      return gPrefs->Write( Name, strValue );
   }
   return true;
}

bool ShuttlePrefs::TransferWrappedType( const wxString & Name, WrappedType & W )
{
   struct {
      void operator()(wxString* string) {
         ret = that->TransferString(Name, *string, *string);
      }
      void operator()(int& i) {
         ret = that->TransferInt(Name, i, i);
      }
      void operator()(double& d) {
         ret = that->TransferDouble(Name, d, d);
      }
      void operator()(bool& b) {
         ret = that->TransferBool(Name, b, b);
      }
      const wxString & Name;
      bool ret;
      ShuttlePrefs * that;
   } to_visit{ Name, false, this };
   std::visit(to_visit, W.mData);
   return to_visit.ret;
}

bool ShuttlePrefs::ExchangeWithMaster(const wxString & WXUNUSED(Name))
{
   // ShuttlePrefs is unusual in that it overloads ALL the Transfer functions
   // which it supports.  It doesn't do any string conversion, because wxConv will
   // do so if it is required.
   // So, ExchangeWithMaster should never get called...  Hence the ASSERT here.
   wxASSERT( false );
   return false;
}
