/* @file scim_simple_config.cpp
 * implementation of KConfigConfig class.
 */

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
 * $Id: scim_kconfig_config.cpp,v 1.17 2006/01/30 11:56:43 liuspider Exp $
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_STL_MAP
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_C_STDIO

#ifndef NO_CONFIG_H
#include "config.h"
#endif

#include <scim.h>
#include "scim_kconfig_config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <qdatetime.h> 

#include <kconfig.h>
#include <kglobal.h>
#include <kapplication.h>

#define scim_module_init kconfig_LTX_scim_module_init
#define scim_module_exit kconfig_LTX_scim_module_exit
#define scim_config_module_init kconfig_LTX_scim_config_module_init
#define scim_config_module_create_config kconfig_LTX_scim_config_module_create_config

extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_CONFIG(1) << "Initializing KConfig Config module...\n";
    }

    void scim_module_exit (void)
    {
        SCIM_DEBUG_CONFIG(1) << "Exiting KConfig Config module...\n";
    }

    void scim_config_module_init ()
    {
        SCIM_DEBUG_CONFIG(1) << "Initializing KConfig Config module (more)...\n";
    }

    ConfigPointer scim_config_module_create_config ()
    {
        SCIM_DEBUG_CONFIG(1) << "Creating a KConfig Config instance...\n";
        return new KConfigConfig ();
    }
}

static QVariant findDefaultItem(const String & key)
{
    KConfigSkeleton * kcs = ScimKdeSettings::self();
    if(KConfigSkeletonItem * item = kcs->findItem(QString::fromUtf8(key.c_str()).replace('/','_')))
    {
        return item->property();
    }
    else
        return QVariant();
}

KConfigConfig::KConfigConfig ()
    : ConfigBase ()
{
    if(!kapp)
        KGlobal::_instance = new KInstance("skim");

    m_config = new KConfig("skimrc");//skimConfigSkeleton->config();
}

KConfigConfig::~KConfigConfig ()
{
    flush ();
    m_config->deleteLater();
    if(!kapp)
      delete KGlobal::_instance;
}

String
KConfigConfig::get_name () const
{
    return "kconfig";
}

bool
KConfigConfig::valid () const
{
    return ConfigBase::valid();
}

// String
bool
KConfigConfig::read (const String& key, String *pStr) const
{
    if (!valid () || !pStr || key.empty()) return false;
    reset_default_group();

    if(m_config->hasKey(QString::fromUtf8(key.c_str()))) {
        QString s = m_config->readEntry(QString::fromUtf8(key.c_str()));
        *pStr = s.utf8();
        return true;
    } else {
        QVariant var = findDefaultItem(key);
        if(var.isValid())
        {
            QString s = var.toString();
            *pStr = s.utf8();
            return true;
        }
        else
        {
            *pStr = String ("");
            return false;
        }
    }
}

// int
bool
KConfigConfig::read (const String& key, int *pl) const
{
    if (!valid () || !pl || key.empty()) return false;

    reset_default_group();

    if(m_config->hasKey(QString::fromUtf8(key.c_str()))) {
        *pl = m_config->readNumEntry(QString::fromUtf8(key.c_str()));
        return true;
    } else {
        QVariant var = findDefaultItem(key);
        if(var.isValid())
        {
            *pl = var.toInt();
            return true;
        }
        else
        {
            *pl = 0;
            return false;
        }
    }
}

// double
bool
KConfigConfig::read (const String& key, double* val) const
{
    if (!valid () || !val || key.empty()) return false;

    reset_default_group();

    if(m_config->hasKey(QString::fromUtf8(key.c_str()))) {
        *val = m_config->readDoubleNumEntry(QString::fromUtf8(key.c_str()));
        return true;
    } else {
        QVariant var = findDefaultItem(key);
        if(var.isValid())
        {
            *val = var.toDouble();
            return true;
        }
        else
        {
            *val = 0;
            return false;
        }
    }
}

// bool
bool
KConfigConfig::read (const String& key, bool* val) const
{
    if (!valid () || !val || key.empty()) return false;

    reset_default_group();

    if(m_config->hasKey(QString::fromUtf8(key.c_str()))) {
        *val = m_config->readBoolEntry(QString::fromUtf8(key.c_str()));
        return true;
    } else {
        QVariant var = findDefaultItem(key);
        if(var.isValid())
        {
            *val = var.toBool();
            return true;
        }
        else
        {
            *val = false;
            return false;
        }
    }
}

//String list
bool
KConfigConfig::read (const String& key, std::vector <String>* val) const
{
    if (!valid () || !val || key.empty()) return false;

    reset_default_group();

    if(m_config->hasKey(QString::fromUtf8(key.c_str()))) {
        val->clear();
        QStringList ls = m_config->readListEntry(QString::fromUtf8(key.c_str()));
        for(uint i = 0; i < ls.size(); i++)
            val->push_back(String(ls[i].utf8()));
        return true;
    } else
        return false;
}

//int list
bool
KConfigConfig::read (const String& key, std::vector <int>* val) const
{
    if (!valid () || !val || key.empty()) return false;

    reset_default_group();

    if(m_config->hasKey(QString::fromUtf8(key.c_str()))) {
        val->clear();
        QValueList<int> ls = m_config->readIntListEntry(QString::fromUtf8(key.c_str()));
        for(uint i = 0; i < ls.size(); i++)
            val->push_back(ls[i]);
        return true;
    } else
        return false;
}

// write the value (return true on success)
bool
KConfigConfig::write (const String& key, const String& value)
{
    if (!valid () || key.empty()) return false;

    reset_default_group();
    m_config->writeEntry(QString::fromUtf8(key.c_str()), QString::fromUtf8(value.c_str()));

//     m_need_reload = true;

    return true;
}

bool
KConfigConfig::write (const String& key, int value)
{
    if (!valid () || key.empty()) return false;

    reset_default_group();
    m_config->writeEntry(QString::fromUtf8(key.c_str()), value);

    return true;
}

bool
KConfigConfig::write (const String& key, double value)
{
    if (!valid () || key.empty()) return false;
    
    reset_default_group();
    m_config->writeEntry(QString::fromUtf8(key.c_str()), value);

    return true;
}

bool
KConfigConfig::write (const String& key, bool value)
{
    if (!valid () || key.empty()) return false;

    reset_default_group();
    m_config->writeEntry(QString::fromUtf8(key.c_str()), value);

    return true;
}

bool
KConfigConfig::write (const String& key, const std::vector <String>& value)
{
    if (!valid () || key.empty()) return false;

    QStringList ls;
    for(uint i = 0; i < value.size(); i++)
        ls.push_back(QString::fromUtf8(value[i].c_str()));

    reset_default_group();
    m_config->writeEntry(QString::fromUtf8(key.c_str()), ls);

    return true;
}

bool
KConfigConfig::write (const String& key, const std::vector <int>& value)
{
    if (!valid () || key.empty()) return false;

    QValueList<int> ls;
    for(uint i = 0; i < value.size(); i++)
        ls.push_back(value[i]);

    reset_default_group();
    m_config->writeEntry(QString::fromUtf8(key.c_str()), ls);

    return true;
}

// permanently writes all changes
bool
KConfigConfig::flush()
{
    if (!valid ()) return false;

    timeval m_update_timestamp;
    gettimeofday (&m_update_timestamp, 0);

    char buf [128];
    snprintf (buf, 128, "%lu:%lu", m_update_timestamp.tv_sec, m_update_timestamp.tv_usec);
    write(String (SCIM_CONFIG_UPDATE_TIMESTAMP), String (buf));

    m_config->sync();

    return true;
}

// delete entries
bool
KConfigConfig::erase (const String& key)
{
    if (!valid ()) return false;

    reset_default_group();
    m_config->deleteEntry(QString::fromUtf8(key.c_str()));

    return true;
}

bool
KConfigConfig::reload ()
{
    if (!valid ()) return false;

    m_config->reparseConfiguration();
    // notify scim-lib modules to reload config
    return ConfigBase::reload ();

}

void 
KConfigConfig::reset_default_group () const
{
    m_config->setGroup("SCIM");
}
