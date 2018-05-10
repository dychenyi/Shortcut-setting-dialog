#ifndef COMMANDFILE_H
#define COMMANDFILE_H
#include "shortcut.h"
#include <QWidget>
class CommandsFile : public QObject
{
    Q_OBJECT

public:
    CommandsFile(const QString &filename);

    QMap<QString, QKeySequence> importCommands() const;
    bool exportCommands(const QList<ShortcutItem *> &items, bool resetToDefault = false);

private:
    QString m_filename;
};



#endif // COMMANDFILE_H
