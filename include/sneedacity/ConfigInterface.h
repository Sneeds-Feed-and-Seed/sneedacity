/**********************************************************************

   Sneedacity: A Digital Audio Editor

   ConfigInterface.h

   Leland Lucius

   Copyright (c) 2014, Sneedacity Team 
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

**********************************************************************/

#ifndef __SNEEDACITY_CONFIGINTERFACE_H__
#define __SNEEDACITY_CONFIGINTERFACE_H__

#include "Identifier.h"

/*************************************************************************************//**

\class ConfigClientInterface

\brief ConfigClientInterface is an unholy get/set configuration class, which 
differentiates between private and shared config.  It should probably be replaced 
with a Shuttle.

*******************************************************************************************/
class SNEEDACITY_DLL_API ConfigClientInterface /* not final */
{
public:
   virtual ~ConfigClientInterface() {};

   virtual bool HasSharedConfigGroup(const RegistryPath & group) = 0;
   virtual bool GetSharedConfigSubgroups(const RegistryPath & group, RegistryPaths & subgroups) = 0;

   virtual bool GetSharedConfig(const RegistryPath & group, const RegistryPath & key, wxString & value, const wxString & defval) = 0;
   virtual bool GetSharedConfig(const RegistryPath & group, const RegistryPath & key, int & value, int defval) = 0;
   virtual bool GetSharedConfig(const RegistryPath & group, const RegistryPath & key, bool & value, bool defval) = 0;
   virtual bool GetSharedConfig(const RegistryPath & group, const RegistryPath & key, float & value, float defval) = 0;
   virtual bool GetSharedConfig(const RegistryPath & group, const RegistryPath & key, double & value, double defval) = 0;

   virtual bool SetSharedConfig(const RegistryPath & group, const RegistryPath & key, const wxString & value) = 0;
   virtual bool SetSharedConfig(const RegistryPath & group, const RegistryPath & key, const int & value) = 0;
   virtual bool SetSharedConfig(const RegistryPath & group, const RegistryPath & key, const bool & value) = 0;
   virtual bool SetSharedConfig(const RegistryPath & group, const RegistryPath & key, const float & value) = 0;
   virtual bool SetSharedConfig(const RegistryPath & group, const RegistryPath & key, const double & value) = 0;

   virtual bool RemoveSharedConfigSubgroup(const RegistryPath & group) = 0;
   virtual bool RemoveSharedConfig(const RegistryPath & group, const RegistryPath & key) = 0;

   virtual bool HasPrivateConfigGroup(const RegistryPath & group) = 0;
   virtual bool GetPrivateConfigSubgroups(const RegistryPath & group, RegistryPaths &subgroups) = 0;

   virtual bool GetPrivateConfig(const RegistryPath & group, const RegistryPath & key, wxString & value, const wxString & defval) = 0;
   virtual bool GetPrivateConfig(const RegistryPath & group, const RegistryPath & key, int & value, int defval) = 0;
   virtual bool GetPrivateConfig(const RegistryPath & group, const RegistryPath & key, bool & value, bool defval) = 0;
   virtual bool GetPrivateConfig(const RegistryPath & group, const RegistryPath & key, float & value, float defval) = 0;
   virtual bool GetPrivateConfig(const RegistryPath & group, const RegistryPath & key, double & value, double defval) = 0;

   virtual bool SetPrivateConfig(const RegistryPath & group, const RegistryPath & key, const wxString & value) = 0;
   virtual bool SetPrivateConfig(const RegistryPath & group, const RegistryPath & key, const int & value) = 0;
   virtual bool SetPrivateConfig(const RegistryPath & group, const RegistryPath & key, const bool & value) = 0;
   virtual bool SetPrivateConfig(const RegistryPath & group, const RegistryPath & key, const float & value) = 0;
   virtual bool SetPrivateConfig(const RegistryPath & group, const RegistryPath & key, const double & value) = 0;

   virtual bool RemovePrivateConfigSubgroup(const RegistryPath & group) = 0;
   virtual bool RemovePrivateConfig(const RegistryPath & group, const RegistryPath & key) = 0;
};

#if 0
/*************************************************************************************//**

\class ConfigHostInterface

\brief ConfigHostInterface appears not to be used.

*******************************************************************************************/
class ConfigHostInterface
{
public:
   virtual ~ConfigHostInterface() {};

};
#endif

#endif // __SNEEDACITY_CONFIGINTERFACE_H__
