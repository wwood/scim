/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#define USE_SCIM_KDE_SETTINGS
#include "src/skimpluginmanager.h"
#include "utils/scimkdesettings.h"
#include "compmgrclient.h"
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>

//internal variables
static Atom skim_wm_window_opacity;
static Atom skim_wm_window_shadow;

#define OPAQUE  0xffffffff

#include <qwidgetlist.h>

#endif

#include <klocale.h>

#include <kapplication.h>
#include <kgenericfactory.h>

#include <kconfig.h>
#include <kconfigskeleton.h>


K_EXPORT_COMPONENT_FACTORY( skimplugin_compmgrclient,
                            KGenericFactory<CompMgrClient>( "skimplugin_compmgrclient" ) )

CompMgrClient::CompMgrClient(QObject *parent, const char *name, const QStringList & /*args*/)
    : SkimPlugin(KGenericFactory<CompMgrClient>::instance(), parent, name),
      DCOPObject("Skim_CompMgrClient"), m_useKompmgr(true), m_createdAtoms(false)
{
    connect( this, SIGNAL(settingsChanged()), SLOT(loadCompositeSettings()));
    loadCompositeSettings();
}

void CompMgrClient::loadCompositeSettingsInternal()
{
    m_enableComposite = ScimKdeSettings::enable_Composite();

    disconnect(SkimPluginManager::self(), SIGNAL(allPluginsLoaded()), this, SLOT(updateCompositeSettings()));
    if(m_enableComposite)
    {
        if(!m_createdAtoms || m_useKompmgr != ScimKdeSettings::use_Kompmgr())
        {
            m_useKompmgr = ScimKdeSettings::use_Kompmgr();
            create_X11_atoms();
        }
        connect(SkimPluginManager::self(), SIGNAL(allPluginsLoaded()), this, SLOT(updateCompositeSettings()));
    }
    
    QStringList availableWindows;
    QValueList<QObject *>  list = SkimPluginManager::self()->specialProperyObjects();
    QValueList<QObject *>::iterator it;

    QWidget * w;
    for ( it = list.begin(); it != list.end(); ++it ) {
        w = (*it)->isWidgetType()?static_cast<QWidget *>(*it) : 0;
        if(w)
            availableWindows << w->name();
    }

    m_compSetting.clear();
    for(uint i=0; i < availableWindows.size(); i++)
    {
        fillWidgetSetting(availableWindows[i]);
    }
}

void CompMgrClient::fillWidgetSetting(const QString & name, bool overwrite)
{
    if(!m_compSetting.contains(name) || overwrite)
    {
        QString group("Composite_");
        group += name;
        windowCompositeSetting winsetting;
        KConfig * config = ScimKdeSettings::self()->config();
        if(config->hasGroup(group))
        {
            config->setGroup(group);
            winsetting.translucencyEnabled = config->readBoolEntry("EnableTranslucency", true);
            winsetting.translucency = config->readNumEntry("Translucency", 75);
        } else
        {
            //default value: EnableTranslucency is true
            winsetting.translucencyEnabled = true;
            winsetting.translucency = 75;
        }
        m_compSetting[name] = winsetting;
    }
}

void CompMgrClient::loadCompositeSettings()
{
    loadCompositeSettingsInternal();
    updateCompositeSettings();
}

void CompMgrClient::update(QString name)
{
    updateCompositeSettings(name);
}

void CompMgrClient::create_X11_atoms() {
#ifdef Q_WS_X11
    m_createdAtoms = true;
    const int max = 20;
    Atom* atoms[max];
    const char* names[max];
    Atom atoms_return[max];
    int n = 0;

    atoms[n] = &skim_wm_window_opacity;
    if(m_useKompmgr)
        names[n++] = (char*) "_KDE_WM_WINDOW_OPACITY";
    else
        names[n++] = (char*) "_NET_WM_WINDOW_OPACITY";

    atoms[n] = &skim_wm_window_shadow;
    names[n++] = (char*) "_KDE_WM_WINDOW_SHADOW";

    // we need a const_cast for the X API
    XInternAtoms( qt_xdisplay(), const_cast<char**>(names), n, false, atoms_return );
    for (int i = 0; i < n; i++ )
        *atoms[i] = atoms_return[i];

#endif
}

void CompMgrClient::setOpacity(QWidget *w, uint opacity, bool updateImmediate){
#ifdef Q_WS_X11
    /* get property */
    uint currentOpacity;
    unsigned char *data;

    Atom actual;
    int format;
    unsigned long n, left;
    XGetWindowProperty(qt_xdisplay(), w->winId(), skim_wm_window_opacity,
                       0L, 1L, False, XA_CARDINAL, &actual, &format, &n, &left,
                       (unsigned char **) &data);

    if (data != None)
    {
        memcpy (&currentOpacity, data, sizeof (unsigned int));
        XFree(( void *) data );
        kdDebug() << "Found property: " << (double) currentOpacity / OPAQUE << "\n";
    }
    else
        currentOpacity = OPAQUE;

    if( currentOpacity != (OPAQUE / 100 * opacity) )
    {
        QPoint oldPos = w->pos();
        bool visible = w->isVisible();
        if(updateImmediate && !visible) {
        //in order to make it take effect immediately, we have to call SyncX, however before
        //that this widget must be shown first
            w->move(-2000, -2000);
            XMapWindow(qt_xdisplay(), w->winId());
            QApplication::syncX ();
        }
    
        if(opacity > 99)
            XDeleteProperty (qt_xdisplay(), w->winId(), skim_wm_window_opacity);
        else
        {
            opacity = (uint) (OPAQUE / 100 * opacity);
            XChangeProperty(qt_xdisplay(), w->winId(), skim_wm_window_opacity,
                            XA_CARDINAL, 32, PropModeReplace,
                            (unsigned char *) &opacity, 1L);
        }
    
        if(updateImmediate) {
            QApplication::syncX ();
            if(!visible)
                XUnmapWindow(qt_xdisplay(), w->winId());
            w->move(oldPos);
        }
    }
#endif
}

void CompMgrClient::updateCompositeSettings(QString widgetname)
{
    if(m_enableComposite)
    {
        QValueList<QObject *>  list = SkimPluginManager::self()->specialProperyObjects();   //TODO: add a property name
        QValueList<QObject *>::iterator it;

        QWidget * w;
        for ( it = list.begin(); it != list.end(); ++it ) {
            w = (*it)->isWidgetType()?static_cast<QWidget *>(*it) : 0;
            if(w)
            {
                if( widgetname == QString::null || w->name() == widgetname)
                {
                    fillWidgetSetting(QString(w->name()), false);
                    if(m_compSetting[w->name()].translucencyEnabled)
                    {
                        setOpacity(w, m_compSetting[w->name()].translucency);
                        kdDebug() << "Updating transparent window '" << w->name() << "'\n";
                    } else
                        setOpacity(w, 100);
                }
            }
        }
    }
}

CompMgrClient::~CompMgrClient()
{
}


#include "compmgrclient.moc"
