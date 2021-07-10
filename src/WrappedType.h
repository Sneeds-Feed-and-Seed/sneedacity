/**********************************************************************

  WrappedType.h

  James Crook
  (C) Sneedacity Developers, 2021
  (C) Audacity Developers, 2007

  wxWidgets license. See Licensing.txt

*************************************************************************/

#ifndef __WRAPPED_TYPE__
#define __WRAPPED_TYPE__
#include <variant>

class wxString;
class ShuttlePrefs;

class SNEEDACITY_DLL_API WrappedType
{
public:
   explicit WrappedType()
   {}
   explicit WrappedType( wxString & InStr )
      : mData{ &InStr }
   {}
   explicit WrappedType( int & InInt )
      : mData{ InInt }
   {}
   explicit WrappedType( double & InDouble )
      : mData{ InDouble }
   {}
   explicit WrappedType( bool & InBool )
      : mData{ InBool }
   {}
   
   /// @return true if the wrapped type is a string.
   inline bool IsString() { return std::holds_alternative<wxString*>(mData); }

   wxString ReadAsString();
   int ReadAsInt();
   double ReadAsDouble();
   bool ReadAsBool();

   void WriteToAsString( const wxString & InStr);
   void WriteToAsInt( const int InInt);
   void WriteToAsDouble( const double InDouble);
   void WriteToAsBool( const bool InBool);

private:
   std::variant<wxString*, int, double, bool> mData;
};

#endif
