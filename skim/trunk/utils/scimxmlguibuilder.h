/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMXMLGUIBUILDER_H
#define SCIMXMLGUIBUILDER_H

#include <kxmlguibuilder.h>
#include <kxmlguiclient.h>
#include <kactioncollection.h>

/**
@author spider
*/
class ScimXMLGUIBuilderPrivate;
class ScimXMLGUIClient : public KXMLGUIClient
{
public:
    ScimXMLGUIClient(KActionCollection* _defaultAC = 0);
    ~ScimXMLGUIClient();
    virtual KActionCollection * actionCollection () const;
private:
    KActionCollection* m_defaultActionCollection;
};

class ScimXMLGUIBuilder : public KXMLGUIBuilder
{
public:
    ScimXMLGUIBuilder(QWidget *);

    ~ScimXMLGUIBuilder();

void setBuilderClient (KXMLGUIClient *client);

virtual QWidget * createContainer (QWidget *parent, int index, const QDomElement &element, int &id);
virtual void removeContainer (QWidget *container, QWidget *parent, QDomElement &element, int id);

private:
    ScimXMLGUIBuilderPrivate *d;
};

#endif
