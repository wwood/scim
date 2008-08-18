#include <qapplication.h>
#include <qsettings.h>
#include <qlabel.h>

#include "testinputwidgets.h"


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget test;
    Ui::TestInputWidgets ui;
    ui.setupUi(&test);

    /*QSettings st;
    QString qtimenv = getenv("QT_IM_MODULE");
    if(qtimenv.length())
       test.currentImmodule->setText(qtimenv);
    else
       test.currentImmodule->setText(st.readEntry("/qt/DefaultInputMethod", "XIM"));
    a.setMainWidget( &test );*/
    test.move(350, 200);
    test.show();
    return a.exec();
}
