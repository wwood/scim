/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimxmlguibuilder.h"

#include <ktoolbar.h>
#include <kxmlguiclient.h>
#include <iostream>

ScimXMLGUIClient::ScimXMLGUIClient(KActionCollection* _defaultAC) 
  : KXMLGUIClient(), m_defaultActionCollection(_defaultAC){}

ScimXMLGUIClient::~ScimXMLGUIClient(){}

KActionCollection * ScimXMLGUIClient::actionCollection () const
{
    if(m_defaultActionCollection)
      return m_defaultActionCollection;
    else
      return KXMLGUIClient::actionCollection();
}

class ScimXMLGUIBuilderPrivate
{
public:
    ScimXMLGUIBuilderPrivate() {
    }
  ~ScimXMLGUIBuilderPrivate() {
  }

    QWidget *m_widget;

    QString tagMainWindow;
    QString tagMenuBar;
    QString tagMenu;
    QString tagToolBar;
    QString tagStatusBar;

    QString tagSeparator;
    QString tagTearOffHandle;
    QString tagMenuTitle;

    QString attrName;
    QString attrLineSeparator;

    QString attrText1;
    QString attrText2;

    QString attrIcon;

    KInstance *m_instance;
    KXMLGUIClient *m_client;
};

ScimXMLGUIBuilder::ScimXMLGUIBuilder(QWidget *widget)
 : KXMLGUIBuilder(widget)
{
  d = new ScimXMLGUIBuilderPrivate;
  d->m_widget = widget;

  d->tagMainWindow = QString::fromLatin1( "mainwindow" );
  d->tagMenuBar = QString::fromLatin1( "menubar" );
  d->tagMenu = QString::fromLatin1( "menu" );
  d->tagToolBar = QString::fromLatin1( "toolbar" );
  d->tagStatusBar = QString::fromLatin1( "statusbar" );

  d->tagSeparator = QString::fromLatin1( "separator" );
  d->tagTearOffHandle = QString::fromLatin1( "tearoffhandle" );
  d->tagMenuTitle = QString::fromLatin1( "title" );

  d->attrName = QString::fromLatin1( "name" );
  d->attrLineSeparator = QString::fromLatin1( "lineseparator" );

  d->attrText1 = QString::fromLatin1( "text" );
  d->attrText2 = QString::fromLatin1( "Text" );

  d->attrIcon = QString::fromLatin1( "icon" );

  d->m_instance = 0;
  d->m_client = 0;
}


ScimXMLGUIBuilder::~ScimXMLGUIBuilder()
{
  delete d;
}

void ScimXMLGUIBuilder::setBuilderClient (KXMLGUIClient *client)
{
  d->m_client = client;
  KXMLGUIBuilder::setBuilderClient(client);
}

QWidget * ScimXMLGUIBuilder::createContainer (QWidget *parent, int /*index*/, const QDomElement &element, int &id)
{
  id = -1;
  if ( element.tagName().lower() == d->tagToolBar )
  {
    QCString name = element.attribute( d->attrName ).utf8();

    KToolBar *bar = dynamic_cast<KToolBar*>(d->m_widget->child( name, "KToolBar" ));
    if( !bar )
    {
       std::cerr << "Can not find KToolBar with name '" << name << "' in widget " << parent->name() << "\n";
       return 0;
    }

    if ( d->m_client && !d->m_client->xmlFile().isEmpty() )
        bar->setXMLGUIClient( d->m_client );

    bar->loadState( element );

    return bar;
  }
  return 0;
}

void ScimXMLGUIBuilder::removeContainer (QWidget *container, 
  QWidget */*parent*/, QDomElement &element, int /*id*/)
{
    if ( ::qt_cast<KToolBar *>( container ) )
    {
        KToolBar *tb = static_cast<KToolBar *>( container );

        tb->saveState( element );
    }
}
