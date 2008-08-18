/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_FILTER_MANAGER
#define Uses_SCIM_COMPOSE_KEY
#include "scimimenginesettings.h"
#include "scimimenginesettingsui.h"
#include "skimkeygrabber.h"
#include "scimattachfilter.h"

#include <qlistview.h> 
#include <qpushbutton.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpixmap.h> 
#include <qiconset.h> 
#include <kgenericfactory.h>
#include <klocale.h>
#include <kiconloader.h>

typedef KGenericFactory<ScimIMEngineSettings> KCMScimIMEngSettingsLoaderFactory;

//copied from scim/modules/SetupUI/scim_imengine_setup.cpp
typedef std::map <scim::String, scim::KeyEventList>                                       MapStringKeyEventList;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_imengines, 
    KCMScimIMEngSettingsLoaderFactory( "kcm_skimplugin_scim_imengines" ) )

class ScimIMEngineSettings::ScimIMEngineSettingsPrivate {
public:
    struct itemExtraInfo{
        bool enabled;
        int uuidIndex;
        std::vector<scim::String> installedFilters;
        std::vector<scim::String> originalFilters;
    };
    QMap<QCheckListItem*, itemExtraInfo> itemInfos;
    QStringList uuids;
    QMap<QString, QString> hotkey_map;
    QMap<scim::String, scim::FilterInfo> filter_map;
    bool enabledIMEsChanged, hotkeysChanged, filtersChanged;
    ScimAttachFilter* attachFilterDlg;
    scim::FilterManager *filter_mgr;
};

ScimIMEngineSettings::ScimIMEngineSettings(QWidget *parent, 
  const char */*name*/, const QStringList &args)
 : KAutoCModule( KCMScimIMEngSettingsLoaderFactory::instance(), parent, args ),
   d(new ScimIMEngineSettingsPrivate)
{
    ui = new SCIMIMEConfigUI(this);
    setMainWidget(ui);
    connect(ui->listView, SIGNAL(clicked(QListViewItem *, const QPoint &, int)),
            SLOT(checkBoxModified(QListViewItem*, const QPoint &, int)));
    connect(ui->listView, SIGNAL(currentChanged(QListViewItem *)), SLOT(updateEditHotkeysBtnStates(QListViewItem *)));
    connect(ui->enableAllBtn, SIGNAL(clicked()), SLOT(enableAllIMEs()));
    connect(ui->disableAllBtn, SIGNAL(clicked()), SLOT(disableAllIMEs()));
    connect(ui->editHotkeysBtn, SIGNAL(clicked()), SLOT(editHotkeys()));
    connect(ui->editFilterBtn, SIGNAL(clicked()), SLOT(editIMFilters()));

    m_config = SkimPluginManager::scimConfigObject();

    d->enabledIMEsChanged = d->hotkeysChanged = d->filtersChanged = false;
    d->attachFilterDlg = 0;
    d->filter_mgr = new scim::FilterManager(m_config);
    load();

    ui->editHotkeysBtn->setEnabled(false);
    ui->editFilterBtn->setEnabled(false);
}

void ScimIMEngineSettings::updateEditHotkeysBtnStates(QListViewItem * curit){
    if(!curit)
      return;

    QCheckListItem * item;
    if(!(item = dynamic_cast<QCheckListItem *>(curit)))
      return;

    if(item->type() == QCheckListItem::CheckBox)
    {
        ui->editHotkeysBtn->setEnabled(true);
        ui->editFilterBtn->setEnabled(true);
    }
    else
    {
        ui->editHotkeysBtn->setEnabled(false);
        ui->editFilterBtn->setEnabled(false);
    }
}

void ScimIMEngineSettings::enableAllIMEs() {
    toggleAllIMEs(true);
}

void ScimIMEngineSettings::disableAllIMEs() {
    toggleAllIMEs(false);
}

void ScimIMEngineSettings::toggleAllIMEs(bool enabled) {
    bool sthchanged = false;
    QListViewItemIterator it(ui->listView);
    QCheckListItem * item;
    while ( it.current() ) {
        if((item = dynamic_cast<QCheckListItem *>(it.current())) && item->type() == QCheckListItem::CheckBox && 
            d->itemInfos.contains(item)) {
            item->setOn(enabled);
            if(item->isOn() != d->itemInfos[item].enabled)
            {
                sthchanged = true;
                break;
            }
        }
        it++;
    }
    if(sthchanged)
        emit changed(true);
}

#define PIX_COLUMN 0
#define NAME_COLUMN 0
#define HOT_KEYS_COLUMN 1
#define FILTER_COLUMN 2
void ScimIMEngineSettings::load() {
    clear();

    {// Load Hotkeys.
        scim::IMEngineHotkeyMatcher hotkey_matcher;
        hotkey_matcher.load_hotkeys (m_config);
        scim::KeyEventList keys;
        std::vector <scim::String> uuids;
        MapStringKeyEventList hotkey_map;

        if (hotkey_matcher.get_all_hotkeys(keys, uuids) > 0) {
            scim::String str;

            for (uint i = 0; i < keys.size (); ++i) {
                hotkey_map[uuids[i]].push_back(keys [i]);
            }

            for (MapStringKeyEventList::iterator it = hotkey_map.begin (); it != hotkey_map.end (); ++it){
                scim::scim_key_list_to_string (str, it->second);
                d->hotkey_map [it->first.c_str()] = str.c_str();
            }
        }
    }

    // Load Filters
    scim::FilterInfo filterinfo;
    for (size_t i=0; i<d->filter_mgr->number_of_filters(); i++)
    {
        if(d->filter_mgr->get_filter_info(i, filterinfo))
        {
            d->filter_map[filterinfo.uuid] = filterinfo;
        }
    }

    QStringList & uuids = d->uuids;
    QStringList names;
    QStringList langs;
    QStringList icons;
    
    std::map <QString, std::vector <size_t> > groups;
    
    QString      lang_name;
    size_t i;
    
    get_factory_list (uuids, names, langs, icons);

    for (i = 0; i < uuids.size (); ++i) {
        groups [langs [i]].push_back (i);
    }
    
    // Add language group
    std::vector <scim::String> filter_list;
    for (std::map <QString, std::vector <size_t> >::iterator it = groups.begin ();
         it != groups.end (); ++ it) {
        scim::String raw_name(it->first.utf8());
        lang_name = QString::fromUtf8(scim::scim_get_language_name (raw_name).c_str());
        QCheckListItem * langGroupItem = 
          new QCheckListItem(ui->listView, lang_name, QCheckListItem::CheckBoxController);

        // Add factories for this group
//         bool inconsistent = false;
        std::vector<scim::String> disabled;
        disabled = scim::scim_global_config_read (
            scim::String (SCIM_GLOBAL_CONFIG_DISABLED_IMENGINE_FACTORIES), disabled);
        QString disabledFactories = QString::fromLatin1(scim::scim_combine_string_list(disabled).c_str());
        for (i = 0; i < it->second.size (); ++i) {
          size_t id = it->second [i];
          QCheckListItem * item =
            new QCheckListItem(langGroupItem, names [id], QCheckListItem::CheckBox);

          //check installed filters
          if(d->filter_mgr->get_filters_for_imengine(scim::String(uuids[id].latin1()), filter_list))
            setIMFilters(item, filter_list);

          ScimIMEngineSettingsPrivate::itemExtraInfo info = {false, it->second [i], filter_list, filter_list};

          item->setPixmap(PIX_COLUMN,
                          KGlobal::iconLoader()->loadIcon(
                                  icons [id], KIcon::User, ui->fontMetrics().height()));
          if(!disabledFactories.contains(uuids[id])) {
            item->setOn(true);
            info.enabled = true;
          }
          d->itemInfos[item] = info;
          if(d->hotkey_map.contains(uuids[id]))
              item->setText(HOT_KEYS_COLUMN, d->hotkey_map[uuids[id]]);
//             inconsistent = true;
        }
//         if(inconsistent)
//           langGroupItem->setState(QCheckListItem::NoChange);
    }
}
void ScimIMEngineSettings::setIMFilters(QCheckListItem * item, std::vector <scim::String> & filter_list)
{
    //TODO: in Qt 3, it is impossible to set more than one icon/pixmap in a column
    //in Qt 4, use clear() to remove all existing data
    if(filter_list.size () == 1)
    {
        item->setPixmap(FILTER_COLUMN,
                        KGlobal::iconLoader()->loadIcon(
                                QString::fromUtf8(d->filter_map[filter_list[0]].icon.c_str()), KIcon::User,
                                ui->fontMetrics().height()));
    } else
        item->setPixmap(FILTER_COLUMN, 0);

    QStringList display_filter;
    for (size_t j = 0; j < filter_list.size (); ++j)
        if(d->filter_map.contains(filter_list[j]))
            display_filter << QString::fromUtf8(d->filter_map[filter_list[j]].name.c_str());
    else
        filter_list.erase(filter_list.begin() + j);

    item->setText(FILTER_COLUMN, display_filter.join("\n"));
}

void ScimIMEngineSettings::save()
{
    QCheckListItem * item;
    if(d->enabledIMEsChanged)   //write disabled IMEs
    {
        std::vector<scim::String> disabledUUIDsList;
        QListViewItemIterator it(ui->listView/*, QListViewItemIterator::NotChecked*/);
        while ( it.current() ) {
            if((item = dynamic_cast<QCheckListItem *>(it.current())) && d->itemInfos.contains(item)) {
                d->itemInfos[item].enabled = item->isOn();
                if(!item->isOn())
                    disabledUUIDsList.push_back(d->uuids[d->itemInfos[item].uuidIndex].latin1());
            }
            it++;
        }

        scim::scim_global_config_write (
            scim::String (SCIM_GLOBAL_CONFIG_DISABLED_IMENGINE_FACTORIES), disabledUUIDsList);
    }

    if(d->hotkeysChanged || d->filtersChanged)   // Save Hotkeys.and filter settings
    {
        scim::IMEngineHotkeyMatcher hotkey_matcher;

        QListViewItemIterator it(ui->listView);
        scim::KeyEventList keylist;
        d->hotkey_map.clear();
        while ( it.current() ) {
            if((item = dynamic_cast<QCheckListItem *>(it.current())) && item->type() == QCheckListItem::CheckBox
                && d->itemInfos.contains(item)) {
                if(!item->text(HOT_KEYS_COLUMN).isNull()) {
                    if (scim::scim_string_to_key_list (keylist, scim::String (item->text(HOT_KEYS_COLUMN).latin1()))) {
                        hotkey_matcher.add_hotkeys (keylist, d->uuids[d->itemInfos[item].uuidIndex].latin1());
                        d->hotkey_map[d->uuids[d->itemInfos[item].uuidIndex]] = item->text(HOT_KEYS_COLUMN);
                    }
                }
                if(d->itemInfos[item].originalFilters != d->itemInfos[item].installedFilters)
                {
                    d->filter_mgr->set_filters_for_imengine(
                            scim::String(d->uuids[d->itemInfos[item].uuidIndex].latin1()),
                    d->itemInfos[item].installedFilters);
                    d->itemInfos[item].originalFilters = d->itemInfos[item].installedFilters;
                }
            }
            it++;
        }

        hotkey_matcher.save_hotkeys (m_config);
    }

    d->enabledIMEsChanged = d->hotkeysChanged = d->filtersChanged = false;
    m_config->flush();
}

void ScimIMEngineSettings::editHotkeys(){
    bool sthchanged = false;
    QListViewItem * cit = ui->listView->currentItem();
    QCheckListItem * item;
    if(!cit || !(item = dynamic_cast<QCheckListItem *>(cit)) || item->type() != QCheckListItem::CheckBox) {
        ui->editHotkeysBtn->setEnabled(false);
        ui->editFilterBtn->setEnabled(false);
        return;
    }
    SkimShortcutListEditor keyEditor(ui);
    QStringList keylist = QStringList::split(',', item->text(HOT_KEYS_COLUMN));
    keyEditor.setStringList(keylist);
    keyEditor.setCaption(i18n("Specify Hotkeys for IME \"%1\"").arg(item->text(NAME_COLUMN)));
    if(keyEditor.exec() == QDialog::Accepted ) {
        item->setText(HOT_KEYS_COLUMN, keyEditor.getCombinedString());
        if(d->itemInfos.contains(item))
            if(d->hotkey_map.contains(d->uuids[d->itemInfos[item].uuidIndex])) {
                if(item->text(HOT_KEYS_COLUMN) != d->hotkey_map[d->uuids[d->itemInfos[item].uuidIndex]])
                    sthchanged = true;
            } else if(!item->text(HOT_KEYS_COLUMN).isNull())
                sthchanged = true;

        if(sthchanged) {
            d->hotkeysChanged = true;
            emit changed(true);
        } else {
            d->hotkeysChanged = false;
            QListViewItemIterator it(ui->listView);
            while ( it.current() ) {
                if((item = dynamic_cast<QCheckListItem *>(it.current()))) {
                    if(d->itemInfos.contains(item)) {
                        if(d->hotkey_map.contains(d->uuids[d->itemInfos[item].uuidIndex])) {
                            if(item->text(HOT_KEYS_COLUMN) != d->hotkey_map[d->uuids[d->itemInfos[item].uuidIndex]]) {
                                d->hotkeysChanged = true;
                                break;
                            }
                        } else if(!item->text(HOT_KEYS_COLUMN).isNull()){
                            d->hotkeysChanged = true;
                            break;
                        }
                    }
                }
                it++;
            }

            checkModification();
        }
    }
}

void ScimIMEngineSettings::defaults()
{
    QCheckListItem * item;
    QListViewItemIterator it(ui->listView);
    while ( it.current() ) {
        if((item = dynamic_cast<QCheckListItem *>(it.current()))) {
          item->setOn(true);
        }
        it++;
    }
}

void ScimIMEngineSettings::editIMFilters()
{
    QListViewItem * cit = ui->listView->currentItem();
    QCheckListItem * item;
    if(!cit || !(item = dynamic_cast<QCheckListItem *>(cit)) || item->type() != QCheckListItem::CheckBox) {
//         ui->editHotkeysBtn->setEnabled(false);
        return;
    }
    if(!d->attachFilterDlg)
        d->attachFilterDlg = new ScimAttachFilter(d->filter_map, ui);

    if(d->itemInfos.contains(item))
    {
        d->attachFilterDlg->setCurrentIMEngine(item->text(NAME_COLUMN), d->itemInfos[item].installedFilters);
    }

    if(d->attachFilterDlg->exec() == QDialog::Accepted)
    {
        d->itemInfos[item].installedFilters = d->attachFilterDlg->attachedFilters();
        setIMFilters(item, d->itemInfos[item].installedFilters);

        if(d->itemInfos[item].originalFilters != d->itemInfos[item].installedFilters)
        {
            d->filtersChanged = true;
            emit changed(true);
        }
        else
        {
            d->filtersChanged = false;
            QMap<QCheckListItem*, ScimIMEngineSettingsPrivate::itemExtraInfo>::Iterator it= d->itemInfos.begin();
            while ( it !=  d->itemInfos.end()) {
                if(it.data().originalFilters != it.data().installedFilters)
                {
                    d->filtersChanged = true;
                    break;
                }
                it++;
            }

            checkModification();
        }
    }
}

void ScimIMEngineSettings::checkBoxModified(QListViewItem * curit, const QPoint &, int column)
{
    if(!curit)
        return;

    QCheckListItem * item;
    if(!(item = dynamic_cast<QCheckListItem *>(curit)))
        return;

    if(item->type() != QCheckListItem::CheckBox 
        && item->type() != QCheckListItem::CheckBoxController )
        return;

    if(column == HOT_KEYS_COLUMN)
        editHotkeys();
    else if(column == FILTER_COLUMN)
    {
        editIMFilters();
    }
    else
    {
        QListViewItemIterator it(ui->listView/*, QListViewItemIterator::Checked*/);
        d->enabledIMEsChanged = false;
        while ( it.current() ) {
            if((item = dynamic_cast<QCheckListItem *>(it.current()))) {
            if(d->itemInfos.contains(item)) {
                if(item->isOn() != d->itemInfos[item].enabled) {
                d->enabledIMEsChanged = true;
                break;
                }
            }
            }
            it++;
        }

        checkModification();
    }
}

ScimIMEngineSettings::~ScimIMEngineSettings()
{
    clear();
}

void ScimIMEngineSettings::checkModification()
{
    emit changed(d->filtersChanged || d->enabledIMEsChanged || d->hotkeysChanged);
}

void ScimIMEngineSettings::clear()
{
    d->hotkey_map.clear();
    d->filter_map.clear();
    d->itemInfos.clear();
    ui->listView->clear();
}

void ScimIMEngineSettings::get_factory_list (QStringList &uuids,
                          QStringList &names,
                          QStringList &langs,
                          QStringList &icons)
{
    std::vector<scim::String>    module_list;
    scim::IMEngineFactoryPointer factory;
    scim::IMEngineModule         module;

    scim::scim_get_imengine_module_list (module_list);

    uuids.clear ();
    names.clear ();
    langs.clear ();
    icons.clear ();

    // Add "English/European" factory first.
    factory = new scim::ComposeKeyFactory ();
    uuids.push_back (QString::fromLatin1(factory->get_uuid ().c_str()));
    names.push_back (QString::fromUtf8(scim::utf8_wcstombs (factory->get_name ()).c_str()));
    langs.push_back (QString::fromUtf8(scim::scim_get_normalized_language (factory->get_language ()).c_str()));
    icons.push_back (QString::fromUtf8(factory->get_icon_file ().c_str()));

    for (size_t i = 0; i < module_list.size (); ++ i) {
        module.load (module_list [i], m_config);
        if (module.valid ()) {
            for (size_t j = 0; j < module.number_of_factories (); ++j) {
                //FIXME
//                 try {
                    factory = module.create_factory (j);
//                 } catch (...) {
//                     factory.reset ();
//                 }

                if (!factory.null ()) {
                    if (uuids.find(QString::fromLatin1(factory->get_uuid ().c_str())) == uuids.end ()) {
                        uuids.push_back (QString::fromLatin1(factory->get_uuid ().c_str()));
                        names.push_back (
                          QString::fromUtf8(scim::utf8_wcstombs (factory->get_name ()).c_str()));
                        langs.push_back (QString::fromUtf8(
                          scim::scim_get_normalized_language (factory->get_language ()).c_str()));
                        icons.push_back (QString::fromUtf8(factory->get_icon_file ().c_str()));
                    }
                    factory.reset ();
                }
            }
            module.unload ();
        }
    }
}

#include "scimimenginesettings.moc"
