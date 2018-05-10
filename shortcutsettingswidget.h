#ifndef SHORTCUTSETTINGSWIDGET_H
#define SHORTCUTSETTINGSWIDGET_H

#include "shortcut.h"
#include "commandfile.h"
#include "commandmappings.h"

#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>

QT_BEGIN_NAMESPACE
class QTreeWidget;
class QTreeWidgetItem;

QT_END_NAMESPACE

class ShortcutSettingsWidget : public CommandMappings
{
    Q_OBJECT

public:
    ShortcutSettingsWidget(QWidget *parent = 0);
    ~ShortcutSettingsWidget() override;

    void apply();
    void setSettingFilename(const QString& f) {m_setting_filename = f; importAction();}
    void initialize(const QList<ShortcutItem*>& itemIn);
    void getKeySequence(QMap<QString, QKeySequence>& mapping) const ;

protected:
    void importAction() override;
    void exportAction() override;
    void defaultAction() override;
    bool filterColumn(const QString &filterString, QTreeWidgetItem *item, int column) const override;
protected:
    void save();
private:

    void handleCurrentCommandChanged(QTreeWidgetItem *current);
    void resetToDefault();
    bool validateShortcutEdit() const;
    bool markCollisions(ShortcutItem *);
    void setKeySequence(const QKeySequence &key);
    void showConflicts();
    void clear();

    QList<ShortcutItem *> m_scitems;
    QGroupBox *m_shortcutBox;
    QLineEdit *m_shortcutEdit;
    QLabel *m_warningLabel;

    QString m_setting_filename;
};

#endif // SHORTCUTSETTINGSWIDGET_H
