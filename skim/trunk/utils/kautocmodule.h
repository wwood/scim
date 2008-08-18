/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef KAUTOCMODULE_H
#define KAUTOCMODULE_H

#include <kcmodule.h>

class KConfig;
class KConfigSkeleton;
class KConfigDialogManager;

class KAutoCModule : public KCModule
{
	Q_OBJECT
	public:
// 		/**
// 		* Standard KCModule constructor.  Use KGlobal::config()
// 		*/
// 		KAutoCModule( QWidget * parent = 0, const char * name = 0, const QStringList & args = QStringList() );

		/**
		* Standard KCModule constructor.  Use KGlobal::config()
		*/
		KAutoCModule( KInstance * instance, QWidget * parent = 0, const QStringList & args = QStringList(), KConfigSkeleton * conf = 0 );

// 		/**
// 		*  @param config the KConfig to use
// 		*/
// 		KAutoCModule(KConfig* config, KInstance * instance, QWidget * parent = 0, const QStringList & args = QStringList() );

// 		/**
// 		*  @param config the KConfig to use
// 		*/
// 		KAutoCModule(KConfig* config, QWidget * parent = 0, const char * name=0 , const QStringList & args = QStringList() );


		~KAutoCModule();

		/**
		 * Set the main widget. @p widget will be lay out to take all available place in the module.
		 * @p widget must have this module as parent.
		 *
		 * This method automatically call KAutoConfig::addWidget() and KAutoConfig::retrieveSettings()
		 *
		 * @param widget the widget to place on the page and to add in the KAutoConfig
		 * @param group the name of the group where settings are stored in the config file
		 */
		void setMainWidget(QWidget *widget/*, const QString& group*/);

		/**
		 * @brief a reference to the KAutoConfig
		 *
		 * You can add or remove manually some widget from the KAutoWidget.
		 * If you choose to don't add the main widget with setMainWidget() , you need
		 * to call  KAutoConfig::retrieveSettings(true) yourself
		 *
		 * @return a reference to the KAutoConfig
		 */
		KConfigDialogManager *configDialgoMgr();

		/**
		 * Reload the config from the configfile.
		 *
		 * You can also reimplement this method, but you should always call the parent KCModule::load()
		 * be sure you know what you are doing
		 */
		virtual void load();

		/**
		 * Save the config to the configfile.
		 *
		 * You can also reimplement this method, but you should always call the parent KCModule::save()
		 * be sure you know what you are doing
		 */
		virtual void save();

		/**
		 * Reload the default config
		 *
		 * You can also reimplement this method, but you should always call the parent KCModule::defaults()
		 * be sure you know what you are doing
		 */
		virtual void defaults();


	protected slots:
		/**
		 * Some setting was modified, updates buttons
		 */
		virtual void slotWidgetModified();

	private:
		class KAutoCModulePrivate;
		KAutoCModulePrivate * d;
};


#endif
