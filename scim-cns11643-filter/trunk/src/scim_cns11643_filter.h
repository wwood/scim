/** @file scim_sctc_filter.h
 * definition of CNS11643Filter (Turn Chinese codes into CNS11643 TAGs) related classes.
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
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

#if !defined (__SCIM_CNS11643_FILTER_H)
#define __SCIM_CNS11643_FILTER_H

#define Uses_SCIM_FILTER
#define Uses_SCIM_FILTER_MODULE
#define Uses_SCIM_CONFIG_BASE
#include <scim.h>

// Include scim configuration header
#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#if defined(HAVE_LIBINTL_H) && defined(ENABLE_NLS)
  #include <libintl.h>
  #define _(String) dgettext(GETTEXT_PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

using namespace scim;

enum CNS11643WorkMode
{
    CNS11643_MODE_OFF = 0,
    CNS11643_MODE_EXT_B,
    CNS11643_MODE_EXT_AB,
    CNS11643_MODE_ALL,
};

enum CNS11643TagType
{
    CNS11643_TAG_BCNS,
    CNS11643_TAG_CNSD
};

class CNS11643FilterFactory : public FilterFactoryBase
{
    friend class CNS11643FilterInstance;

public:
    CNS11643FilterFactory ();

    virtual WideString  get_name () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_help () const;
    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
};

class CNS11643FilterInstance : public FilterInstanceBase
{
    bool               m_props_registered;

    CNS11643WorkMode   m_work_mode;
    CNS11643TagType    m_tag_type;

public:
    CNS11643FilterInstance (CNS11643FilterFactory *factory, const IMEngineInstancePointer &orig_inst);

public:
    virtual void focus_in ();

    virtual void trigger_property (const String &property);

protected:
    virtual void filter_commit_string (const WideString &str);
    virtual void filter_register_properties (const PropertyList &properties);
};

#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/

