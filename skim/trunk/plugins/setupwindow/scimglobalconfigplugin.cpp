/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef NO_CONFIG_H
#include "config.h"
#endif

#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_EVENT
#include <scim.h>

#include "utils/scimkdesettings.h"

#include "scimglobalconfigplugin.h"

#include "scim_global.h"
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h> 
#include <kdialogbase.h>
#include <qvbox.h> 
#include <keditlistbox.h> 
#include <kconfigdialogmanager.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qerrormessage.h> 
#include <qspinbox.h>

typedef KGenericFactory<ScimGlobalConfigPlugin> ScimGlobalConfigPluginLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_global, 
    ScimGlobalConfigPluginLoaderFactory( "kcm_skimplugin_scim_global" ) )

class ScimGlobalConfigPlugin::ScimGlobalConfigPluginPrivate {
public:
    ScimGlobalSettingUI * ui;
    int panelProgramIndex;
    int configModuleIndex;
    QStringList utf8Locales;
    QString frontendAddress;
    QString iMEngineAddress;
    QString configAddress;
    QString panelAddress;
    int socketTimeout;
    bool warningBoxCleanup;
    int keyboardIndex;
    scim::KeyEvent modifers;
};

ScimGlobalConfigPlugin::ScimGlobalConfigPlugin(QWidget *parent, 
  const char */*name*/, const QStringList &args)
    : KAutoCModule(ScimGlobalConfigPluginLoaderFactory::instance(), parent, args),
  d(new ScimGlobalConfigPluginPrivate)
{
    d->ui = new ScimGlobalSettingUI(this);
    setMainWidget(d->ui);
    d->ui->advSettings->hide();

    d->warningBoxCleanup = false;

    load();

    checkSettings();

    connect(d->ui->programCombo, SIGNAL(activated(int)), SLOT(widgetChanged()));
    connect(d->ui->ConfigCombo, SIGNAL(activated(int)), SLOT(widgetChanged()));

    connect(d->ui->KeyboardLayoutcomboBox, SIGNAL(activated(int)), SLOT(widgetChanged()));

    //advanced settings
//     connect(d->ui->localesDeleteButton, SIGNAL(clicked()), SLOT(deleteLocale()) );
    connect(d->ui->localesEditButton, SIGNAL(clicked()), SLOT(editLocale()) );
//     connect(d->ui->unicodeLocaleCombo, SIGNAL(textChanged( const QString &)), SLOT(widgetChanged()) );
//     connect(d->ui->unicodeLocaleCombo, SIGNAL(activated(int)), SLOT(widgetChanged()) );

    connect(d->ui->frontendAddressEdit, SIGNAL(textChanged(const QString&)), SLOT(widgetChanged()));
    connect(d->ui->iMEngineAddressEdit, SIGNAL(textChanged(const QString&)), SLOT(widgetChanged()));
    connect(d->ui->configAddressEdit, SIGNAL(textChanged(const QString&)), SLOT(widgetChanged()));
    connect(d->ui->panelAddressEdit, SIGNAL(textChanged(const QString&)), SLOT(widgetChanged()));

    connect(d->ui->socketTimeoutSpinBox, SIGNAL(valueChanged(int)), SLOT(widgetChanged()));

    connect(d->ui->altBox, SIGNAL(clicked()), SLOT(widgetChanged()));
    connect(d->ui->shiftBox, SIGNAL(clicked()), SLOT(widgetChanged()));
    connect(d->ui->controlBox, SIGNAL(clicked()), SLOT(widgetChanged()));
#if defined(Q_WS_X11)
    connect(d->ui->metaBox, SIGNAL(clicked()), SLOT(widgetChanged()));
    connect(d->ui->superBox, SIGNAL(clicked()), SLOT(widgetChanged()));
    connect(d->ui->hyperBox, SIGNAL(clicked()), SLOT(widgetChanged()));
    connect(d->ui->capslockBox, SIGNAL(clicked()), SLOT(widgetChanged()));
#else
    d->ui->metaBox->hide();
    d->ui->superBox->hide();
    d->ui->hyperBox->hide();
    d->ui->capslockBox->hide();
#endif
}


ScimGlobalConfigPlugin::~ScimGlobalConfigPlugin()
{
    if(d->warningBoxCleanup)
        QErrorMessage::qtHandler ()->deleteLater();
}

void ScimGlobalConfigPlugin::editLocale()
{
    KDialogBase * editLocaleDlg = new KDialogBase(this,
        0, true, i18n("Edit Supported Locales"), KDialogBase::Ok|KDialogBase::Cancel);
    KEditListBox * localeList = new KEditListBox(editLocaleDlg->makeVBoxMainWidget(),
        0, false, KEditListBox ::Add | KEditListBox::Remove);

    for(int curl = 0; curl < d->ui->unicodeLocaleCombo->count(); curl++)
      localeList->insertItem(d->ui->unicodeLocaleCombo->text(curl));
    if(editLocaleDlg->exec() == QDialog::Accepted)
    {
      d->ui->unicodeLocaleCombo->clear();
      d->ui->unicodeLocaleCombo->insertStringList(localeList->items());
      widgetChanged();
    }
    delete editLocaleDlg;
}

void ScimGlobalConfigPlugin::widgetChanged()
{
    bool isChanged = false;

    if ( d->modifers.is_alt_down() != d->ui->altBox->isChecked() ||
         d->modifers.is_shift_down() != d->ui->shiftBox->isChecked() ||
#if defined(Q_WS_X11)
         d->modifers.is_meta_down() != d->ui->metaBox->isChecked() ||
         d->modifers.is_super_down() != d->ui->superBox->isChecked() ||
         d->modifers.is_hyper_down() != d->ui->hyperBox->isChecked() ||
         d->modifers.is_caps_lock_down() != d->ui->capslockBox->isChecked() ||
#endif
         d->modifers.is_control_down() != d->ui->controlBox->isChecked() )
         isChanged = true;

    if ( !isChanged )
        if ( configDialgoMgr()->hasChanged() ||
            d->keyboardIndex != d->ui->KeyboardLayoutcomboBox->currentItem() ||
            d->ui->programCombo->currentItem() != d->panelProgramIndex ||
            d->ui->ConfigCombo->currentItem() != d->configModuleIndex ||
            checkLocales() ||
            d->frontendAddress != d->ui->frontendAddressEdit->text() ||
            d->iMEngineAddress != d->ui->iMEngineAddressEdit->text() ||
            d->configAddress != d->ui->configAddressEdit->text() ||
            d->panelAddress != d->ui->panelAddressEdit->text() ||
            d->socketTimeout != d->ui->socketTimeoutSpinBox->value())
            isChanged = true;

    emit changed(isChanged);
}

// void ScimGlobalConfigPlugin::defaults(){}

void ScimGlobalConfigPlugin::load()
{
    KAutoCModule::load();

    //load keyboard layouts
    d->ui->KeyboardLayoutcomboBox->clear();
    for (size_t i = 0; i < scim::SCIM_KEYBOARD_NUM_LAYOUTS; ++i) {
        QString keyboardName = QString::fromUtf8(
                scim::scim_keyboard_layout_get_display_name (static_cast<scim::KeyboardLayout> (i)).c_str ());
        d->ui->KeyboardLayoutcomboBox->insertItem(keyboardName);
    }

    d->keyboardIndex = static_cast<int>(scim::scim_get_default_keyboard_layout ());
    d->ui->KeyboardLayoutcomboBox->setCurrentItem(d->keyboardIndex);

    //enabled modifiers
    scim::scim_string_to_key (d->modifers, scim::String(ScimKdeSettings::_Hotkeys_FrontEnd_ValidKeyMask().latin1()));
    d->ui->altBox->setChecked(d->modifers.is_alt_down());
    d->ui->shiftBox->setChecked(d->modifers.is_shift_down());
    d->ui->controlBox->setChecked(d->modifers.is_control_down());
#if defined(Q_WS_X11)
    d->ui->metaBox->setChecked(d->modifers.is_meta_down());
    d->ui->superBox->setChecked(d->modifers.is_super_down());
    d->ui->hyperBox->setChecked(d->modifers.is_hyper_down());
    d->ui->capslockBox->setChecked(d->modifers.is_caps_lock_down());
#endif

    //load settings for "Other" tab
    QString panelProgram = QString::fromUtf8(
      scim::scim_global_config_read(
        SCIM_GLOBAL_CONFIG_DEFAULT_PANEL_PROGRAM, scim::String ("scim-panel-kde")).c_str());
    d->panelProgramIndex = 0;
    if(panelProgram.contains("scim-panel-gtk"))
      d->panelProgramIndex = 1;
    d->ui->programCombo->setCurrentItem(d->panelProgramIndex);

    QString configModule = QString::fromUtf8(
      scim::scim_global_config_read(
        SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, scim::String ("kconfig")).c_str());

    std::vector<scim::String>  config_list;
    scim::scim_get_config_module_list(config_list);
    if (std::find (config_list.begin (),
                   config_list.end (),
                   scim::String(configModule.latin1())) == config_list.end ())
        configModule = QString::fromUtf8(config_list[0].c_str());
    d->configModuleIndex = 0;
    d->ui->ConfigCombo->clear();
    for(uint i = 0; i < config_list.size(); i++) {
        QString item = QString::fromUtf8(config_list[i].c_str());
        d->ui->ConfigCombo->insertItem(item);
        if( item == configModule )
          d->configModuleIndex = i;
    }
    d->ui->ConfigCombo->setCurrentItem(d->configModuleIndex);

    //advanced settings
    QString utf8Locales = QString::fromUtf8(
      scim::scim_global_config_read(
        SCIM_GLOBAL_CONFIG_SUPPORTED_UNICODE_LOCALES, scim::String ("en_US.UTF-8")).c_str());
    d->utf8Locales = QStringList::split(",",utf8Locales);
    d->ui->unicodeLocaleCombo->clear();
    d->ui->unicodeLocaleCombo->insertStringList( d->utf8Locales );

    d->frontendAddress = QString::fromUtf8(
      scim::scim_global_config_read(
        SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_FRONTEND_ADDRESS, 
          scim::String ("local:/tmp/scim-socket-frontend")).c_str());
    d->ui->frontendAddressEdit->setText(d->frontendAddress);

    d->iMEngineAddress = QString::fromUtf8(
      scim::scim_global_config_read(
        SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_IMENGINE_ADDRESS, 
          scim::String ("local:/tmp/scim-socket-frontend")).c_str());
    d->ui->iMEngineAddressEdit->setText(d->iMEngineAddress);

    d->configAddress = QString::fromUtf8(
      scim::scim_global_config_read(
        SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_CONFIG_ADDRESS, 
          scim::String ("local:/tmp/scim-socket-frontend")).c_str());
    d->ui->configAddressEdit->setText(d->configAddress);

    d->panelAddress = QString::fromUtf8(
      scim::scim_global_config_read(
        SCIM_GLOBAL_CONFIG_DEFAULT_PANEL_SOCKET_ADDRESS, 
          scim::String ("local:/tmp/scim-panel-socket")).c_str());
    d->ui->panelAddressEdit->setText(d->panelAddress);

    d->socketTimeout = scim::scim_global_config_read(
      SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_TIMEOUT, 5000);
    d->ui->socketTimeoutSpinBox->setValue(d->socketTimeout);
}

void ScimGlobalConfigPlugin::checkSettings()
{
    if(d->ui->ConfigCombo->currentText() != ("kconfig")) {
        QErrorMessage * warningBox = QErrorMessage::qtHandler ();
        d->warningBoxCleanup = true;
        warningBox->message(i18n("If you want to use skim (scim-panel-kde) or scim-qtimm (Qt immodule support for SCIM), please select kconfig as the config module."));
    }
}

static void addCheckModifier(scim::KeyEvent & key, QCheckBox * checkbox, UINT16 modifier)
{
    if(checkbox->isChecked())
        key.mask |= modifier;
}

void ScimGlobalConfigPlugin::save()
{
    checkSettings();

    KAutoCModule::save();

    //save keyboard layout
    int kindex = d->ui->KeyboardLayoutcomboBox->currentItem();

    if (kindex >= 0 && kindex < scim::SCIM_KEYBOARD_NUM_LAYOUTS && kindex != d->keyboardIndex)
    {
        scim::KeyboardLayout keyboard = static_cast<scim::KeyboardLayout>(kindex);
        scim::scim_set_default_keyboard_layout (keyboard);
    }

    d->keyboardIndex = static_cast<int>(scim::scim_get_default_keyboard_layout ());
    d->ui->KeyboardLayoutcomboBox->setCurrentItem(d->keyboardIndex);
    {
        scim::String newvalue("scim-panel-gtk");
        if( d->ui->programCombo->currentText() != "scim-panel-gtk" ) {
            QString path = KStandardDirs::findExe(d->ui->programCombo->currentText());
            if(!path.isNull())
                newvalue = scim::String(path.utf8());
        }   
        scim::scim_global_config_write(
              SCIM_GLOBAL_CONFIG_DEFAULT_PANEL_PROGRAM, newvalue);
    }

    {
        scim::String newvalue(d->ui->ConfigCombo->currentText().utf8());
        scim::scim_global_config_write(
              SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, newvalue);
    }

    //enabled modifiers
    scim::KeyEvent key;
    addCheckModifier(key, d->ui->controlBox, scim::SCIM_KEY_ControlMask);
    addCheckModifier(key, d->ui->altBox, scim::SCIM_KEY_AltMask);
    addCheckModifier(key, d->ui->shiftBox, scim::SCIM_KEY_ShiftMask);
#if defined(Q_WS_X11)
    addCheckModifier(key, d->ui->capslockBox, scim::SCIM_KEY_CapsLockMask);
    addCheckModifier(key, d->ui->metaBox, scim::SCIM_KEY_MetaMask);
    addCheckModifier(key, d->ui->hyperBox, scim::SCIM_KEY_HyperMask);
    addCheckModifier(key, d->ui->superBox, scim::SCIM_KEY_SuperMask);
#endif
    if(key.mask != d->modifers.mask)
    {
        scim::String keystr;
        scim::scim_key_to_string (keystr, key);
        ScimKdeSettings::set_Hotkeys_FrontEnd_ValidKeyMask(keystr.c_str());
    }

    //advanced settings

    if(checkLocales()) {
      d->utf8Locales.clear();
      for(int i = 0; i < d->ui->unicodeLocaleCombo->count(); i++)
        d->utf8Locales << d->ui->unicodeLocaleCombo->text(i);
      scim::String newvalue(d->utf8Locales.join(",").utf8());
      scim::scim_global_config_write(
              SCIM_GLOBAL_CONFIG_SUPPORTED_UNICODE_LOCALES, newvalue);
    }

    if(d->frontendAddress != d->ui->frontendAddressEdit->text()) {
      scim::String newvalue(d->ui->frontendAddressEdit->text().utf8());
      scim::scim_global_config_write(
              SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_FRONTEND_ADDRESS, newvalue);
    }

    if(d->iMEngineAddress != d->ui->iMEngineAddressEdit->text()) {
      scim::String newvalue(d->ui->iMEngineAddressEdit->text().utf8());
      scim::scim_global_config_write(
              SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_IMENGINE_ADDRESS, newvalue);
    }

    if(d->configAddress != d->ui->configAddressEdit->text()) {
      scim::String newvalue(d->ui->configAddressEdit->text().utf8());
      scim::scim_global_config_write(
              SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_CONFIG_ADDRESS, newvalue);
    }

    if(d->panelAddress != d->ui->panelAddressEdit->text()) {
      scim::String newvalue(d->ui->panelAddressEdit->text().utf8());
      scim::scim_global_config_write(
              SCIM_GLOBAL_CONFIG_DEFAULT_PANEL_SOCKET_ADDRESS, newvalue);
    }

    if(d->socketTimeout != d->ui->socketTimeoutSpinBox->value())
      scim::scim_global_config_write(
              SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_TIMEOUT, d->ui->socketTimeoutSpinBox->value());

    scim::scim_global_config_flush();

    load();
}

bool ScimGlobalConfigPlugin::checkLocales()
{
    bool changed = ((int)d->utf8Locales.size() != d->ui->unicodeLocaleCombo->count());
    if(!changed) {
      for(uint i = 0; i < d->utf8Locales.size(); i++)
        if(d->utf8Locales[i] != d->ui->unicodeLocaleCombo->text(i)) {
          changed = true;
          break;
        }
    }
    return changed;
}
#include "scimglobalconfigplugin.moc"
