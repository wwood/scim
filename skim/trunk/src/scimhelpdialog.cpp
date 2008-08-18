/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimhelpdialog.h"
#include <klocale.h>

ScimHelpDialog::ScimHelpDialog(QWidget *parent, const char *name)
 : KAboutDialog(KAboutDialog::AbtTabbed, name, KDialogBase::Close, KDialogBase::Close, parent)
{
    m_generalPage = addContainerPage( i18n("&General") );
    m_generalBrowser = new QTextBrowser(m_generalPage);
    //FIXME: the line below does NOT take effect
    m_generalBrowser->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding, 0, 0 ) );
    m_generalBrowser->setMinimumSize(600, 500);
    m_generalPage->addWidget(m_generalBrowser);
}

void ScimHelpDialog::setGeneralInfo(QString info)
{
    m_generalBrowser->setText( info );
}

#include "scimhelpdialog.moc"
