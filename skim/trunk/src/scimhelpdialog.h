/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMHELPDIALOG_H
#define SCIMHELPDIALOG_H

#include <qtextbrowser.h>
#include <kaboutdialog.h>

/**
@author spider
*/
class ScimHelpDialog : public KAboutDialog
{
Q_OBJECT
public:
    ScimHelpDialog(QWidget *parent = 0, const char *name = 0);
    void setGeneralInfo(QString);

private:
    KAboutContainer *m_generalPage;
    QTextBrowser *m_generalBrowser;
    KAboutContainer *m_curIMPage;
};

#endif
