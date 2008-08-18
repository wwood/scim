#include <dcopref.h>
#include <dcopclient.h>
#include <kapplication.h>

inline void UPDATE_WINDOW_OPACITY(QWidget * w)
{
    if(ScimKdeSettings::enable_Composite())
    {
        DCOPRef Skim_CompMgrClient(kapp->dcopClient()->appId(),"Skim_CompMgrClient");
        if(!Skim_CompMgrClient.isNull())
            Skim_CompMgrClient.call("update(QString)", QString(w->name()));
    }
}
