/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/

#ifndef INPUTWINDOW_H
#define INPUTWINDOW_H

#include "src/skimpluginmanager.h"
#include "scimdragableframe.h"

#include <kaction.h>

class ScimLineEdit;
class ScimLookupTable;
class ScimStringListItem;

class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class inputWindow : public ScimDragableFrame
{
    Q_OBJECT

public:
    inputWindow( SkimPluginManager*, QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~inputWindow();
    void toggleStick();

public slots:
    virtual void adjustSize();

    virtual void updateAuxString( const QString &, const scim::AttributeList & );
    virtual void updateLookupTable( const scim::LookupTable &table, size_t & exact_item_num );
    virtual void updatePreeditString(const QString &, const scim::AttributeList &);
    virtual void updateSpotLocation( int, int );

    virtual void showAuxString();
    virtual void showLookupTable();
    virtual void showPreeditString();

    virtual void hideAuxString();
    virtual void hideLookupTable();
    virtual void hidePreeditString();

    virtual void changeSetting();

protected:
    bool can_hide();
    inline void try_hide();
    QVBoxLayout* m_inputWindowLayout;
    int m_insertPointX, m_insertPointY;

protected slots:
    virtual void languageChange();
    void hideInputWindow ();
    void disableUpdates();
    void enableUpdates();

private:
    ScimLineEdit* m_preEditText;
    ScimStringListItem* m_auxText;
    ScimLookupTable* m_lookupLbl;

    SkimPluginManager* m_allModules;
    scim::SocketServerThread* m_inputServer;
    bool m_sticky, m_inTransaction;
    KToggleAction* m_stickAction;
};

#include "src/skimplugin.h"

class InputWindowPlugin : public SkimPlugin
{
Q_OBJECT
public:
    InputWindowPlugin( QObject *parent, const char *name, const QStringList &args );

    virtual ~InputWindowPlugin();
public slots:
    void toggleStick();
private:
    inputWindow* m_inputw;
};

#endif // INPUTWINDOW_H
