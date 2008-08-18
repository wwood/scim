/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMLAUNCHER_H
#define SCIMLAUNCHER_H

#include "src/skimplugin.h"
#include "dcopobject.h"

class CompMgrClient : public SkimPlugin, public DCOPObject
{
Q_OBJECT
K_DCOP
        friend class CompositeManagerConfig;
        friend class TopWindowlistViewItem;
public:
    CompMgrClient(QObject *parent, const char *name, const QStringList & /*args*/);
    virtual ~CompMgrClient();
    virtual void setOpacity(QWidget *w, uint opacity, bool updateImmediate = true);
//     virtual void aboutToUnload();

k_dcop:
    void update(QString);

public slots:
    virtual void loadCompositeSettings();
    virtual void updateCompositeSettings(QString name = QString::null);

private:
    void create_X11_atoms();
    virtual void loadCompositeSettingsInternal();
    struct windowCompositeSetting
    {
        bool translucencyEnabled;
        int translucency;
        bool operator == (const windowCompositeSetting & st) const {
            return translucencyEnabled == st.translucencyEnabled && translucency == st.translucency;
        }
    };

    void fillWidgetSetting(const QString &, bool overwrite = true);

    typedef QMap<QString, windowCompositeSetting> WindowCompositeSettingRepository;
    WindowCompositeSettingRepository m_compSetting;
    bool m_enableComposite;
    bool m_useKompmgr;
    bool m_createdAtoms;
//   QValueStack< QPair<QWidget *, int> > widgetToUpdateComposite;
};

#endif
