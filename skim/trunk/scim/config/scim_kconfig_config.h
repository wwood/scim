/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002 James Su <suzhe@turbolinux.com.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 */

#if !defined (__SCIM_KCONFIG_CONFIG_H)
#define __SCIM_KCONFIG_CONFIG_H

#include <sys/time.h>
#include "scimkdesettings.h"

using namespace scim;

class KConfigConfig : public ConfigBase
{
    class KConfig * m_config;
    ScimKdeSettings * skimConfigSkeleton;

public:
    KConfigConfig ();

    virtual ~KConfigConfig ();

    virtual bool valid () const;

    // String
    virtual bool read (const String& key, String *pStr) const;

    // int
    virtual bool read (const String& key, int *pl) const;

    // double
    virtual bool read (const String& key, double* val) const;

    // bool
    virtual bool read (const String& key, bool* val) const;

    //String list
    virtual bool read (const String& key, std::vector <String>* val) const;

    //int list
    virtual bool read (const String& key, std::vector <int>* val) const;

    // write the value (return true on success)
    virtual bool write (const String& key, const String& value);
    virtual bool write (const String& key, int value);
    virtual bool write (const String& key, double value);
    virtual bool write (const String& key, bool value);
    virtual bool write (const String& key, const std::vector <String>& value);
    virtual bool write (const String& key, const std::vector <int>& value);

    // permanently writes all changes
    virtual bool flush();

    // delete entries
    virtual bool erase (const String& key );

    // reload the configurations.
    virtual bool reload ();

    virtual String get_name () const;
private:
    void reset_default_group(void) const;
};
#endif

