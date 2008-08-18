/*
  Copyright (C) 2004-2005 LiuCougar <liuspider@users.sourceforge.net>
*/

#include "qsciminputcontext.h"

#include <qinputcontextplugin.h>

class ScimInputContextPlugin : public QInputContextPlugin 
{
public:
    ScimInputContextPlugin() : QInputContextPlugin() {
        qDebug("ScimInputContextPlugin()");
    }

    ~ScimInputContextPlugin()
    {
        qDebug("~ScimInputContextPlugin()");
    }
    
    QStringList keys() const {
        QStringList keylist;
        keylist.push_back("scim");
        return keylist;
    }
    
    QStringList languages(const QString&) {  //FIXME
        QStringList langlist;
        langlist.push_back("zh_CN");
        langlist.push_back("zh_TW");
        langlist.push_back("zh_HK");
        langlist.push_back("ja");
        langlist.push_back("ko");
        return langlist;
    }
    
    QString description(const QString&) {
        return QString::fromUtf8(scim::String(_("Qt immodule plugin for SCIM")).c_str());
    }
        
    QInputContext *create(const QString &s ) {
        if (s.toLower() != "scim") {
            return 0;
        }

        return new scim::QScimInputContext();
    }

    QString displayName( const QString &key ) {
        return key;        
    }
};

Q_EXPORT_PLUGIN(ScimInputContextPlugin)
