/** @file scim_sctc_filter.cpp
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
 * $Id: scim_cns11643_filter.cpp,v 1.3 2006/01/24 11:43:26 suzhe Exp $
 *
 */

#include "scim_cns11643_filter.h"
#include "scim_cns11643_data.h"

#define scim_module_init cns11643_LTX_scim_module_init
#define scim_module_exit cns11643_LTX_scim_module_exit
#define scim_filter_module_init cns11643_LTX_scim_filter_module_init
#define scim_filter_module_create_filter cns11643_LTX_scim_filter_module_create_filter
#define scim_filter_module_get_filter_info cns11643_LTX_scim_filter_module_get_filter_info

using namespace scim;

// Private data definition.
static FilterInfo   __filter_info (String ("b6bafb7b-3044-40ce-89c8-c53031394192"),
                                   String (_("CNS11643 Filter")),
                                   String ("zh_TW,zh_HK,zh_CN,zh_SG"),
                                   String (SCIM_ICONDIR "/CNS11643-Filter.png"),
                                   String (_("Convert Chinese characters into CNS11643 TAGs.")));

static Property     __prop_tag_type (String ("/Filter/CNS11643/TagType"),
                                     String (_("BCNS")),
                                     String (""),
                                     String (_("Tag type.")));

static Property     __prop_root     (String ("/Filter/CNS11643/Mode"),
                                     String (_("NONE")),
                                     String (SCIM_ICONDIR "/cns11643.png"),
                                     String (_("No CNS11643 Conversion.")));

static Property     __prop_off      (String ("/Filter/CNS11643/Mode/Off"),
                                     String (_("NONE")),
                                     String (SCIM_ICONDIR "/cns11643.png"),
                                     String (_("No CNS11643 Conversion")));

static Property     __prop_ext_b    (String ("/Filter/CNS11643/Mode/EXT-B"),
                                     String (_("EXT-B")),
                                     String (SCIM_ICONDIR "/cns11643-ext-b.png"),
                                     String (_("Only do CNS11643 Conversion for characters in CJK EXT-B.")));

static Property     __prop_ext_ab   (String ("/Filter/CNS11643/Mode/EXT-AB"),
                                     String (_("EXT-AB")),
                                     String (SCIM_ICONDIR "/cns11643-ext-ab.png"),
                                     String (_("Only do CNS11643 Conversion for characters in CJK EXT-A and EXT-B.")));

static Property     __prop_all      (String ("/Filter/CNS11643/Mode/All"),
                                     String (_("ALL")),
                                     String (SCIM_ICONDIR "/cns11643-all.png"),
                                     String (_("Do CNS11643 Conversion for all characters.")));

//Private functions definition.
static WideString __ucs_to_cns11643_tag (const WideString &ucs, CNS11643WorkMode mode, CNS11643TagType type);


//Module Interface
extern "C" {
    void scim_module_init (void)
    {
    }

    void scim_module_exit (void)
    {
    }

    unsigned int scim_filter_module_init (const ConfigPointer &config)
    {
        return 1;
    }

    FilterFactoryPointer scim_filter_module_create_filter (unsigned int index)
    {
        if (index == 0)
            return new CNS11643FilterFactory ();

        return FilterFactoryPointer (0);
    }

    bool scim_filter_module_get_filter_info (unsigned int index, FilterInfo &info)
    {
        if (index == 0) {
            info = __filter_info;
            return true;
        }
        return false;
    }
}

//Implementation of private functions
static WideString __ucs_to_cns11643_tag (const WideString &ucs, CNS11643WorkMode mode, CNS11643TagType type)
{
    if (!mode) return ucs;

    WideString cns;
    WideString::const_iterator sit;
    CNS11643_UCS2CNSCodePair *p;
    CNS11643_UCS2CNSCodePair tmp;
    char buf [80];

    for (sit = ucs.begin (); sit != ucs.end (); ++sit) {
        if ((mode == CNS11643_MODE_ALL) || (mode == CNS11643_MODE_EXT_B && *sit >= 0x20000) ||
            (mode == CNS11643_MODE_EXT_AB && (*sit >= 0x20000 || (*sit >= 0x3400 && *sit < 0x4E00)))) {
            tmp.ucs = (uint32) *sit;
            tmp.cns = 0;
            p = std::lower_bound (cns11643_ucs2cns_table, cns11643_ucs2cns_table+CNS11643_NUM_UCS2CNS_ITEMS, tmp);
            if (p->ucs == tmp.ucs) {

                if (type == CNS11643_TAG_BCNS)
                    snprintf (buf, 80, "<page>%x</page><code>%4x</code>", (p->cns >> 16), (p->cns & 0xFFFF));
                else if (type == CNS11643_TAG_CNSD)
                    snprintf (buf, 80, "<cnsd:code enc=\"HEX8\" isXML=\"true\" isBase64=\"false\">%08x</cnsd:code>", p->cns);

                cns = cns + utf8_mbstowcs (buf);
            } else {
                cns.push_back (*sit);
            }
        } else {
            cns.push_back (*sit);
        }
    }
    return cns;
}


//Implementation of CNS11643FilterFactory.
CNS11643FilterFactory::CNS11643FilterFactory ()
{
}

WideString
CNS11643FilterFactory::get_name () const
{
     WideString name = FilterFactoryBase::get_name ();
     return name.length () ? name : utf8_mbstowcs (__filter_info.name);
}

String
CNS11643FilterFactory::get_uuid () const
{
    String uuid = FilterFactoryBase::get_uuid ();
    return uuid.length () ? uuid : __filter_info.uuid;
}

String
CNS11643FilterFactory::get_icon_file () const
{
    String icon = FilterFactoryBase::get_icon_file ();
    return icon.length () ? icon : __filter_info.icon;
}

WideString
CNS11643FilterFactory::get_authors () const
{
    WideString authors = FilterFactoryBase::get_authors ();
    return authors.length () ? authors : utf8_mbstowcs (_("James Su <suzhe@tsinghua.org.cn>"));
}

WideString
CNS11643FilterFactory::get_help () const
{
    // No help yet.
    WideString help = FilterFactoryBase::get_help ();
    return help;
}

IMEngineInstancePointer
CNS11643FilterFactory::create_instance (const String& encoding, int id)
{
    return new CNS11643FilterInstance (this, FilterFactoryBase::create_instance (encoding, id));

    return FilterFactoryBase::create_instance (encoding, id);
}

//Implementation of CNS11643FilterInstance
CNS11643FilterInstance::CNS11643FilterInstance (CNS11643FilterFactory *factory, const IMEngineInstancePointer &orig_inst)
    : FilterInstanceBase (factory, orig_inst),
      m_props_registered (false),
      m_work_mode (CNS11643_MODE_OFF),
      m_tag_type (CNS11643_TAG_BCNS)
{
}

void
CNS11643FilterInstance::focus_in ()
{
    m_props_registered = false;

    FilterInstanceBase::focus_in ();

    if (!m_props_registered) {
        PropertyList props;
        filter_register_properties (props);
    }
}

void
CNS11643FilterInstance::trigger_property (const String &property)
{
    Property prop = __prop_root;
    bool changed = false;

    if (property == __prop_off.get_key ()) {
        m_work_mode = CNS11643_MODE_OFF;
        changed = true;
    } else if (property == __prop_ext_b.get_key ()) {
        m_work_mode = CNS11643_MODE_EXT_B;
        prop.set_icon (__prop_ext_b.get_icon ());
        prop.set_label (__prop_ext_b.get_label ());
        prop.set_tip (__prop_ext_b.get_tip ());
        changed = true;
    } else if (property == __prop_ext_ab.get_key ()) {
        m_work_mode = CNS11643_MODE_EXT_AB;
        prop.set_icon (__prop_ext_ab.get_icon ());
        prop.set_label (__prop_ext_ab.get_label ());
        prop.set_tip (__prop_ext_ab.get_tip ());
        changed = true;
    } else if (property == __prop_all.get_key ()) {
        m_work_mode = CNS11643_MODE_ALL;
        prop.set_icon (__prop_all.get_icon ());
        prop.set_label (__prop_all.get_label ());
        prop.set_tip (__prop_all.get_tip ());
        changed = true;
    } else if (property == __prop_tag_type.get_key ()) {
        prop = __prop_tag_type;
        if (m_tag_type == CNS11643_TAG_BCNS) {
            m_tag_type = CNS11643_TAG_CNSD;
            prop.set_label (_("CNSD"));
        } else if (m_tag_type == CNS11643_TAG_CNSD) {
            m_tag_type = CNS11643_TAG_BCNS;
            prop.set_label (_("BCNS"));
        }
        changed = true;
    }

    if (changed)
        update_property (prop);
    else
        FilterInstanceBase::trigger_property (property);
}

void
CNS11643FilterInstance::filter_commit_string (const WideString &str)
{
    if (m_work_mode)
        commit_string (__ucs_to_cns11643_tag (str, m_work_mode, m_tag_type));
    else
        commit_string (str);
}

void
CNS11643FilterInstance::filter_register_properties (const PropertyList &properties)
{
    PropertyList props = properties;

    Property prop = __prop_root;

    if (m_work_mode == CNS11643_MODE_EXT_B) {
        prop.set_icon (__prop_ext_b.get_icon ());
        prop.set_label (__prop_ext_b.get_label ());
        prop.set_tip (__prop_ext_b.get_tip ());
    } else if (m_work_mode == CNS11643_MODE_EXT_AB) {
        prop.set_icon (__prop_ext_ab.get_icon ());
        prop.set_label (__prop_ext_ab.get_label ());
        prop.set_tip (__prop_ext_ab.get_tip ());
    } else if (m_work_mode == CNS11643_MODE_ALL) {
        prop.set_icon (__prop_all.get_icon ());
        prop.set_label (__prop_all.get_label ());
        prop.set_tip (__prop_all.get_tip ());
    }

    props.push_back (prop);
    props.push_back (__prop_off);
    props.push_back (__prop_ext_b);
    props.push_back (__prop_ext_ab);
    props.push_back (__prop_all);

    prop = __prop_tag_type;

    if (m_tag_type == CNS11643_TAG_BCNS)
        prop.set_label (_("BCNS"));
    else if (m_tag_type == CNS11643_TAG_CNSD)
        prop.set_label (_("CNSD"));

    props.push_back (prop);

    register_properties (props);

    m_props_registered = true;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
