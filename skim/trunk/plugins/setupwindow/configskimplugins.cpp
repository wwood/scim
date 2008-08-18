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
#include "utils/skimplugininfo.h"

#include "configskimplugins.h"

#include <qlayout.h>
#include <kgenericfactory.h>
#include <klocale.h>

#include <kiconloader.h>

typedef KGenericFactory<ConfigSkimPlugins> ConfigSkimPluginsPluginLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_configplugin, 
    ConfigSkimPluginsPluginLoaderFactory( "kcm_skimplugin_configplugin" ) )

class ScimKDEPluginListItem : public QCheckListItem
{
  public:
    ScimKDEPluginListItem(SkimPluginInfo *info, QPixmap &pix, QListView *parent);
    void save(KConfigGroup *);
    void defaults();
    bool isChanged();

  protected:
    void stateChange(bool);

  private:
    SkimPluginInfo* mInfo;
};

ScimKDEPluginListItem::ScimKDEPluginListItem(SkimPluginInfo *info, QPixmap &pix, QListView *parent)
  : QCheckListItem(parent, info->name(), CheckBox), mInfo(info)
{
  setPixmap(0, pix);
  listView()->blockSignals(true);
  setOn(mInfo->isPluginEnabled());
  listView()->blockSignals(false);
}

bool ScimKDEPluginListItem::isChanged() 
{
  return (isOn() != mInfo->isPluginEnabled());
}

void ScimKDEPluginListItem::save(KConfigGroup * kc)
{
  mInfo->setPluginEnabled(isOn());
  mInfo->save(kc);
}

void ScimKDEPluginListItem::defaults()
{
  setOn(true);
}

void ScimKDEPluginListItem::stateChange(bool b)
{
//   if(!silentStateChange)
    static_cast<ScimKDEPluginListView *>(listView())->stateChanged(this, b);
}

ScimKDEPluginListView::ScimKDEPluginListView(QWidget *parent, const char *name)
  : KListView(parent, name)
{
}

void ScimKDEPluginListView::stateChanged(ScimKDEPluginListItem *item, bool b)
{
  emit stateChange(item, b);
}

class ConfigSkimPlugins::ConfigSkimPluginsPrivate {
public:
    ScimKDEPluginListView * list;
};

ConfigSkimPlugins::ConfigSkimPlugins(QWidget *parent, 
  const char */*name*/, const QStringList &args)
 : KCModule( ConfigSkimPluginsPluginLoaderFactory::instance(), parent, args),
   d(new ConfigSkimPluginsPrivate)
{
    (new QVBoxLayout(this,0,0))->setAutoAdd(true);
    d->list = new ScimKDEPluginListView(this);
    d->list->addColumn(i18n("Name"));
    d->list->addColumn(i18n("Comment"));
    connect(d->list, SIGNAL(stateChange(ScimKDEPluginListItem *, bool)), 
      this, SLOT(stateChange(ScimKDEPluginListItem *, bool)));
    load();
}


void ConfigSkimPlugins::stateChange(ScimKDEPluginListItem *, bool)
{
    ScimKDEPluginListItem * curItem;
    for (int i=0; i < d->list->childCount(); i++)
    {
      curItem = static_cast<ScimKDEPluginListItem *>(d->list->itemAtIndex(i));
      if( curItem->isChanged() ) {
        emit changed(true);
        return;
      }
    }
    emit changed(false);
}


void ConfigSkimPlugins::defaults()
{
    ScimKDEPluginListItem * curItem;
    for (int i=0; i < d->list->childCount(); i++)
    {
      curItem = static_cast<ScimKDEPluginListItem *>(d->list->itemAtIndex(i));
      curItem->defaults();
    }
}

void ConfigSkimPlugins::load()
{
    QValueList<SkimPluginInfo *> skimplugins = SkimPluginManager::self()->availablePlugins();
    QValueList<SkimPluginInfo *>::Iterator pit;

    d->list->clear();
    for ( pit = skimplugins.begin(); pit != skimplugins.end(); ++pit )
    {
        if((*pit)->isNoDisplay())
            continue;
        QPixmap pix = KGlobal::iconLoader()->loadIcon((*pit)->icon(), KIcon::Toolbar);
        ScimKDEPluginListItem *item = new ScimKDEPluginListItem(*pit, pix, d->list);
        item->setText(0, (*pit)->name());
        item->setText(1, (*pit)->comment());
    }

    emit changed(false);
}

void ConfigSkimPlugins::save()
{
    ScimKDEPluginListItem * curItem;
    KConfigGroup * kc = new KConfigGroup(ScimKdeSettings::self()->config(), "Plugins");
    for (int i=0; i < d->list->childCount(); i++)
    {
      curItem = static_cast<ScimKDEPluginListItem *>(d->list->itemAtIndex(i));
      curItem->save(kc);
    }
}

#include "configskimplugins.moc"
