/***************************************************************************
 *   Copyright (C) 2003 by liuspider                                          *
 *   sharecash@163.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "compmgrclient.h"
#include "utils/scimkdesettings.h"
#include "compositemanagerconfig.h"

#include <kgenericfactory.h>
#include "compositemanagerbase.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlistview.h>
#include <kapplication.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qpixmap.h>

typedef KGenericFactory<CompositeManagerConfig> CompositeManagerConfigFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_compmgrclient,
                            CompositeManagerConfigFactory( "kcm_skimplugin_compmgrclient" ) )

class TopWindowlistViewItem : public QListViewItem
{
#define TRANSLUCENCY_COLUMN 1
    public:
        typedef CompMgrClient::windowCompositeSetting windowSetting;
        TopWindowlistViewItem(QListView * list, windowSetting & it, QString name,
                              QString caption)
        :QListViewItem(list, caption), originalWindowSetting(it), widget_name(name)
        {
            setTranslucency(it.translucencyEnabled, it.translucency);
        }

        void save(KConfig * config)
        {
            if(isChanged())
            {
                QString group("Composite_");
                group += widget_name;
                config->setGroup(group);
                config->writeEntry("EnableTranslucency", newWindowSetting.translucencyEnabled);
                config->writeEntry("Translucency", newWindowSetting.translucency);
            }
        }

        void defaults()
        {
            setTranslucency(true, 75);
        }

        bool isChanged()
        {
            return !(originalWindowSetting == newWindowSetting);
        }

        windowSetting & currentSetting() { return newWindowSetting; }

        void setTranslucency(bool e, int value)
        {
            newWindowSetting.translucency = value;
            newWindowSetting.translucencyEnabled = e;
            setText(TRANSLUCENCY_COLUMN, QString("%1%").arg(value));
        }

    private:
        windowSetting originalWindowSetting;
        windowSetting newWindowSetting;
        QString widget_name;
};

CompositeManagerConfig::CompositeManagerConfig(QWidget *parent,
    const char */*name*/, const QStringList &args)
    : KAutoCModule(CompositeManagerConfigFactory::instance(), parent, args)
{
    m_ui = new CompositeManagerWidgetBase(this);
    setMainWidget(m_ui);
    connect(m_ui->TopWindowlistView, SIGNAL(selectionChanged( QListViewItem * ) ),
            SLOT(selectedWindowChanged( QListViewItem * )));
    connect(m_ui->activeWindowTranslucencyBox, SIGNAL(toggled(bool)), SLOT(modifyCurrentWindowSetting()));
    connect(m_ui->ActiveSpinBox, SIGNAL(valueChanged(int)), SLOT(modifyCurrentWindowSetting()));
    load();
}

void CompositeManagerConfig::selectedWindowChanged( QListViewItem * i )
{
    if(i)
    {
        if(TopWindowlistViewItem* item = dynamic_cast<TopWindowlistViewItem *>(i))
        {
            m_ui->translucencyGroupBox->setEnabled(true);
            TopWindowlistViewItem::windowSetting & newsetting= item->currentSetting();
            m_ui->activeWindowTranslucencyBox->setEnabled(true);

            //ActiveSpinBox must be blocked otherwise it will trigger slotWidgetModified
            m_ui->ActiveSpinBox->blockSignals(true);
            m_ui->activeWindowTranslucencyBox->blockSignals(true);

            //this will set ActiveSpinBox, for ActiveSpinBox is connect to ActiveSlider's signal
            m_ui->ActiveSlider->setValue(newsetting.translucency);
            m_ui->ActiveSpinBox->blockSignals(false);
            m_ui->activeWindowTranslucencyBox->blockSignals(false);

            m_ui->activeWindowTranslucencyBox->setChecked(newsetting.translucencyEnabled);
        }
    }else
        m_ui->translucencyGroupBox->setEnabled(false);
}

void CompositeManagerConfig::modifyCurrentWindowSetting()
{
    if(TopWindowlistViewItem* item = dynamic_cast<TopWindowlistViewItem *>(m_ui->TopWindowlistView->currentItem()))
    {
        item->setTranslucency(m_ui->activeWindowTranslucencyBox->isChecked(),
                              m_ui->ActiveSlider->value());
    }

    slotWidgetModified();
}

void CompositeManagerConfig::slotWidgetModified()
{
    QListViewItem * i = m_ui->TopWindowlistView->firstChild();
    while( i ) {
        if(TopWindowlistViewItem* item = dynamic_cast<TopWindowlistViewItem *>(i))
            if(item->isChanged())
            {
                emit changed(true);
                break;
            }
        i = i->nextSibling();
    }

    if(!i)   //if nother is changed call the parent implementation
        KAutoCModule::slotWidgetModified();
}

void CompositeManagerConfig::load()
{
    KAutoCModule::load();
    m_ui->mainGroupBox->setEnabled(ScimKdeSettings::enable_Composite());
    if(SkimPlugin * plugin = SkimPluginManager::self()->plugin("skimplugin_m_compmgrclient"))
    {
        m_compmgrclient = static_cast<CompMgrClient *>(plugin);
        m_compmgrclient->loadCompositeSettingsInternal();

        QMap<QString, QWidget *> displayList;
        {
            QValueList<QObject *>  list = SkimPluginManager::self()->specialProperyObjects();
            QValueList<QObject *>::iterator it;
            QWidget * w;

            for ( it = list.begin(); it != list.end(); ++it ) {
                w = (*it)->isWidgetType()?static_cast<QWidget *>(*it) : 0;
                if(w && w->isTopLevel() && m_compmgrclient->m_compSetting.contains(w->name()))
                {
                    displayList[w->name()] = w;
                }
            }
        }

        CompMgrClient::WindowCompositeSettingRepository::Iterator it;
        m_ui->TopWindowlistView->clear();
        for ( it = m_compmgrclient->m_compSetting.begin(); it != m_compmgrclient->m_compSetting.end(); ++it ) {
            if(displayList.contains(it.key()))
                (void) new TopWindowlistViewItem( m_ui->TopWindowlistView, it.data(), it.key(),
                                                  displayList[it.key()]->caption() );
        }
        //on load, nothing is selected, so display detail editors
        m_ui->translucencyGroupBox->setEnabled(false);
    } else
    {
        m_compmgrclient = 0;
        m_ui->mainGroupBox->setEnabled(false);
    }
}

void CompositeManagerConfig::save()
{
    KConfig * config = ScimKdeSettings::self()->config();
    QListViewItem * i = m_ui->TopWindowlistView->firstChild();
    while( i ) {
        if(TopWindowlistViewItem* item =
           dynamic_cast<TopWindowlistViewItem *>(i))
            item->save(config);
        i = i->nextSibling();
    }
    KAutoCModule::save();
}

void CompositeManagerConfig::defaults()
{
    QListViewItem * i = m_ui->TopWindowlistView->firstChild();
    while( i ) {
        if(TopWindowlistViewItem* item =
           dynamic_cast<TopWindowlistViewItem *>(i))
            item->defaults();
        i = i->nextSibling();
    }
    KAutoCModule::defaults();
    selectedWindowChanged(m_ui->TopWindowlistView->currentItem());    //update detail editor
}

CompositeManagerConfig::~CompositeManagerConfig()
{
}


#include "compositemanagerconfig.moc"
