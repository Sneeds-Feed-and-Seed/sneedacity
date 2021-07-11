/**********************************************************************

  WrappedType.cpp

  James Crook
  (C) Sneedacity Developers, 2021
  (C) Audacity Developers, 2007

  wxWidgets license. See Licensing.txt

**********************************************************************//*!

\class WrappedType
\brief
  Used in type conversions, this wrapper for ints, strings, doubles and
  enums provides conversions between all the types.  Functions that
  work on wrapped types can quickly be reused to work on any of
  these types.  This cuts out a lot of repetitive code.

  JKC: This class grows in size with the square of the number of
  types it supports.  It has to do all conversions between all pairs,
  so try to re-use existing types if you can.

  It's probable that not all the cases are actually used in practice.
  The most heavily used will be conversions to <-> from string.

*//**********************************************************************/


#include "WrappedType.h"

#include <wx/wxprec.h>
#include "Internat.h"



wxString WrappedType::ReadAsString() const
{
   struct {
      wxString ret;
      void operator()(wxString* str)
      {
         ret = *str;
      }
      void operator()(int mpInt)
      {
         ret = wxString::Format(wxT("%i"), mpInt);
      }
      void operator()(double mpDouble)
      {
         ret = wxString::Format(wxT("%.8g"), mpDouble);
      }
      void operator()(bool mpBool)
      {
         ret = mpBool ? wxT("true") : wxT("false");
      }
   } to_visit{};
   std::visit(to_visit, mData);
   return to_visit.ret;
}

int WrappedType::ReadAsInt() const
{
   struct {
      void operator()(wxString* str) {
         long l;
         str->ToLong(&l);
         ret = l;
      }
      void operator()(int t) { ret = t; }
      void operator()(double t) { ret = t; }
      void operator()(bool t) { ret = t; }
      int ret;
   } to_visit{};
   std::visit(to_visit, mData);
   return to_visit.ret;
}

double WrappedType::ReadAsDouble() const
{
   struct {
      void operator()(wxString* str) {
         ret = Internat::CompatibleToDouble( *str );
      }
      void operator()(int i) { ret = i; }
      void operator()(double d) { ret = d; }
      void operator()(bool b) { ret = b; }
      double ret;
   } to_visit{};
   std::visit(to_visit, mData);
   return to_visit.ret;
}

bool WrappedType::ReadAsBool() const
{
   struct {
      void operator()(wxString* str) {
         ret = str->IsSameAs( wxT("true"), false );
      }
      void operator()(int i) { ret = i; }
      void operator()(double d) { ret = d; }
      void operator()(bool b) { ret = b; }
      double ret;
   } to_visit{};
   std::visit(to_visit, mData);
   return to_visit.ret;
}


void WrappedType::WriteToAsString( const wxString & inStr)
{
   struct {
      void operator()(wxString* str) {
         *str = inStr;
      }
      void operator()(int& i) {
         long l;
         inStr.ToLong(&l);
         i = l;
      }
      void operator()(double& d) {
         d = Internat::CompatibleToDouble(inStr);
      }
      void operator()(bool& b) {
         b = inStr.IsSameAs(wxT("true"), false);
      }
      const wxString & inStr;
   } to_visit{ inStr };
   std::visit(to_visit, mData);
}

void WrappedType::WriteToAsInt( const int inInt)
{
   struct {
      void operator()(wxString* str) {
         *str = wxString::Format( wxT("%i"), inInt);
      }
      void operator()(int& i) { i = inInt; }
      void operator()(double& d) { d = inInt; }
      void operator()(bool& b) { b = inInt; }
      const int & inInt;
   } to_visit{ inInt };
   std::visit(to_visit, mData);
}

void WrappedType::WriteToAsDouble( const double inDouble)
{
   struct {
      void operator()(wxString* str) {
         *str = wxString::Format( wxT("%.8g"), inDouble );
      }
      void operator()(int& i) { i = inDouble; }
      void operator()(double& d) { d = inDouble; }
      void operator()(bool& b) { b = inDouble; }
      const double & inDouble;
   } to_visit{ inDouble };
   std::visit(to_visit, mData);
}


void WrappedType::WriteToAsBool( const bool inBool)
{
   struct {
      void operator()(wxString* str) {
         *str = inBool ? wxT("true") : wxT("false");
      }
      void operator()(int& i) { i = inBool; }
      void operator()(double& d) { d = inBool; }
      void operator()(bool& b) { b = inBool; }
      const bool & inBool;
   } to_visit{ inBool };
   std::visit(to_visit, mData);
}
