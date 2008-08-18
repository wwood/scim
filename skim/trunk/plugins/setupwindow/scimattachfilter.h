/***************************************************************************
 *   Copyright (C) 2003 by liuspider                                          *
 *   sharecash@163.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef SCIMATTACHFILTER_H
#define SCIMATTACHFILTER_H

#define Uses_SCIM_FILTER_MANAGER
#include "src/skimpluginmanager.h"

#include "attachfilteruibase.h"

class ScimAttachFilter : public AttachFilterUIBase
{
Q_OBJECT
    std::vector<scim::String> installedFilters;
    QMap<scim::String, scim::FilterInfo> filter_map;
    QMap<QString, scim::String> name2uuid_map;

public:
    ScimAttachFilter(QMap<scim::String, scim::FilterInfo> & filters, QWidget *parent = 0, const char *name = 0);
    void setCurrentIMEngine(QString name, std::vector<scim::String> & _installedFilters);
    std::vector<scim::String> & attachedFilters();

protected slots:
    void addFilter();
    void removeFilter();
    void moveDownFilter();
    void moveUpFilter();
    void moreInfo();
    void updateButtons();
};

#endif
