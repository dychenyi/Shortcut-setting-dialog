#ifndef SHORTCUT_H
#define SHORTCUT_H
#include <QObject>
#include <QWidget>
#include <QPushButton>


class QTreeWidget;
class QTreeWidgetItem;


struct ShortcutItem
{
    ShortcutItem(const QString& name,
                 const QString& desp,
                 const QKeySequence& default_key,
                 const QKeySequence& key,
                 QTreeWidgetItem *item)
                :m_name(name),
                 m_desp(desp),
                 m_default_key(default_key),
                 m_key(key),
                 m_item(item)
    {

    }
    QString m_name;
    QString m_desp;
    QKeySequence m_default_key;
    QKeySequence m_key;
    QTreeWidgetItem *m_item;
};


class ShortcutButton : public QPushButton
{
    Q_OBJECT
public:
    ShortcutButton(QWidget *parent = 0);

    QSize sizeHint() const;

signals:
    void keySequenceChanged(const QKeySequence &sequence);

protected:
    bool eventFilter(QObject *obj, QEvent *evt);

private:
    void updateText();
    void handleToggleChange(bool toggleState);

    QString m_checkedText;
    QString m_uncheckedText;
    mutable int m_preferredWidth = -1;
    std::array<int, 4> m_key;
    int m_keyNum = 0;
};

#endif // SHORTCUT_H
