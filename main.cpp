#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
//    qDebug() <<  settings.fileName();

    QList<ShortcutItem*> m_scitems;
    m_scitems << new ShortcutItem("view.zoom fit", "zoom fit in physical view",
                     QKeySequence("F"),  QKeySequence("F"), new QTreeWidgetItem())
    << new ShortcutItem("view.zoom in", "zoom in in physical view",
                     QKeySequence("Z"),  QKeySequence("Z"), new QTreeWidgetItem())
    << new ShortcutItem("view.zoom out", "zoom in in physical view",
                     QKeySequence("Shift+Z"),  QKeySequence("Shift+Z"), new QTreeWidgetItem())
;

    //write default setting to user's drive
    CommandsFile cmd("d:\\testsettings");
    cmd.exportCommands(m_scitems);


    //load user's setting
    ShortcutSettingsWidget w;
    w.initialize(m_scitems);
    w.setSettingFilename("d:\\testsettings");
    w.show();

    return a.exec();
}
