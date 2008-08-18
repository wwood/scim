/***************************************************************************
 *   Copyright (C) 2003 by liuspider                                          *
 *   sharecash@163.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimattachfilter.h"
#include "filterinfodlgbase.h"

#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>

ScimAttachFilter::ScimAttachFilter(QMap<scim::String, scim::FilterInfo> & filters, QWidget *parent, const char *name)
 : AttachFilterUIBase(parent, name)
{
    filter_map = filters;

    QMap<scim::String, scim::FilterInfo>::Iterator it;
    for ( it = filter_map.begin(); it != filter_map.end(); ++it )
        name2uuid_map[QString::fromUtf8(it.data().name.c_str())] = it.key();

    connect(addButton, SIGNAL(clicked()), SLOT(addFilter()));
    connect(moreInfoButton, SIGNAL(clicked()), SLOT(moreInfo()));
    connect(removeButton, SIGNAL(clicked()), SLOT(removeFilter()));
    connect(moveDownButton, SIGNAL(clicked()), SLOT(moveDownFilter()));
    connect(moveUpButton, SIGNAL(clicked()), SLOT(moveUpFilter()));

    connect(filterListBox, SIGNAL(currentChanged(QListBoxItem* )), SLOT(updateButtons()));
    connect(installedFilterListBox, SIGNAL(currentChanged(QListBoxItem* )), SLOT(updateButtons()));
}

std::vector<scim::String> & ScimAttachFilter::attachedFilters()
{
    return installedFilters;
}

void ScimAttachFilter::moreInfo()
{
    if(QListBoxItem * item = filterListBox->selectedItem ())
    {
        scim::FilterInfo & info = filter_map[name2uuid_map[item->text()]];

        FilterInfoDlgBase filterInfoDlg(this);
        filterInfoDlg.setCaption(i18n("Filter %1").arg(item->text()));
        filterInfoDlg.setIcon(KGlobal::iconLoader()->loadIcon(info.icon.c_str(), KIcon::User));
        filterInfoDlg.nameLabel->setText(item->text());
        filterInfoDlg.descriptionLabel->setText(QString::fromUtf8(info.desc.c_str()));

        //retrieve the user friendly name of the language
        std::vector <scim::String> lang_ids;
        std::vector <scim::String> lang_names;
        scim::scim_split_string_list (lang_ids, info.langs);
        
        for (std::vector <scim::String>::const_iterator sit = lang_ids.begin (); sit != lang_ids.end (); ++sit) {
            scim::String name = scim::scim_get_language_name (*sit);
            if (std::find (lang_names.begin (), lang_names.end (), name) == lang_names.end ())
                lang_names.push_back (name);
        }

        scim::String langs = scim::scim_combine_string_list (lang_names);

        filterInfoDlg.langLabel->setText(QString::fromUtf8(langs.c_str()));
        filterInfoDlg.exec();
    }
}

void ScimAttachFilter::updateButtons()
{
    QListBoxItem * item = 0;
    if((item = filterListBox->selectedItem ()) &&
       installedFilterListBox->findItem(item->text(), Qt::ExactMatch) == 0)
        addButton->setEnabled(true);
    else
        addButton->setEnabled(false);

    if(item)
        moreInfoButton->setEnabled(true);
    else
        moreInfoButton->setEnabled(false);

    if(installedFilterListBox->selectedItem ())
    {
        if(installedFilterListBox->selectedItem()->next())
            moveDownButton->setEnabled(true);
        else
            moveDownButton->setEnabled(false);
        if(installedFilterListBox->selectedItem()->prev())
            moveUpButton->setEnabled(true);
        else
            moveUpButton->setEnabled(false);
        removeButton->setEnabled(true);
    }
    else
    {
        moveDownButton->setEnabled(false);
        moveUpButton->setEnabled(false);
        removeButton->setEnabled(false);
    }
}

void ScimAttachFilter::moveDownFilter()
{
    QListBoxItem * item = installedFilterListBox->selectedItem();
    if(item)
    {
        QListBoxItem * next_item = item->next();
        if(next_item)
        {
            installedFilterListBox->takeItem(item);
            installedFilterListBox->insertItem(item, next_item);
            installedFilterListBox->setSelected(item, true);
        }
    }
}

void ScimAttachFilter::moveUpFilter()
{
    QListBoxItem * item = installedFilterListBox->selectedItem();
    if(item)
    {
        QListBoxItem * prev_item = item->prev();
        if(prev_item)
        {
            installedFilterListBox->takeItem(item);
            installedFilterListBox->insertItem(item, prev_item->prev());
            installedFilterListBox->setSelected(item, true);
        }
    }
}

void ScimAttachFilter::setCurrentIMEngine(QString name, std::vector<scim::String> & _installedFilters)
{
    setCaption(i18n("Edit Filters for %1").arg(name));
    installedTextLabel->setText(i18n("&Installed Filters for %1").arg(name));

    installedFilters = _installedFilters;

    filterListBox->clear();
    installedFilterListBox->clear();

    QMap<scim::String, scim::FilterInfo>::Iterator it;
    for ( it = filter_map.begin(); it != filter_map.end(); ++it )
    {
        filterListBox->insertItem(
            KGlobal::iconLoader()->loadIcon(QString::fromUtf8(it.data().icon.c_str()), KIcon::User, fontMetrics().height()),
            QString::fromUtf8(it.data().name.c_str()));
    }

    filterListBox->setCurrentItem(0);

    for ( size_t i=0; i<installedFilters.size(); i++)
        if (filter_map.contains(installedFilters[i]))
        {
            installedFilterListBox->insertItem(
                    KGlobal::iconLoader()->loadIcon(
                        QString::fromUtf8(filter_map[installedFilters[i]].icon.c_str()), KIcon::User, fontMetrics().height()),
                    QString::fromUtf8(filter_map[installedFilters[i]].name.c_str()));
        }

    installedFilterListBox->setCurrentItem(0);
}

void ScimAttachFilter::addFilter()
{
    if(QListBoxItem * item = filterListBox->selectedItem ())
    {
        if(installedFilterListBox->findItem(item->text(), Qt::ExactMatch) == 0)
        {
            scim::String selected_uuid = name2uuid_map[item->text()];
            installedFilterListBox->insertItem(
                KGlobal::iconLoader()->loadIcon(
                    QString::fromUtf8(filter_map[selected_uuid].icon.c_str()), KIcon::User, fontMetrics().height()),
                QString::fromUtf8(filter_map[selected_uuid].name.c_str()));

            installedFilterListBox->setCurrentItem(installedFilterListBox->count()-1);

            if(std::find(installedFilters.begin(), installedFilters.end(), selected_uuid) == installedFilters.end())
            {
                installedFilters.push_back(selected_uuid);
            }
        }
    }
}

void ScimAttachFilter::removeFilter()
{
    if(QListBoxItem * item = installedFilterListBox->selectedItem ())
    {
        scim::String selected_uuid = name2uuid_map[item->text()];
        installedFilterListBox->takeItem(item);

        std::vector<scim::String>::iterator it = std::find(installedFilters.begin(), installedFilters.end(), selected_uuid);
        if(it != installedFilters.end())
            installedFilters.erase(it);
    }
}

#include "scimattachfilter.moc"
