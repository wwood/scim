/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef KCMLOADER_H
#define KCMLOADER_H

#include "utils/kautocmodule.h"

class KCMLoader : public KAutoCModule
{
Q_OBJECT
public:
    KCMLoader(QWidget *parent, const char * /* name */, const QStringList &args );

//     ~KCMLoader();
};

#endif
