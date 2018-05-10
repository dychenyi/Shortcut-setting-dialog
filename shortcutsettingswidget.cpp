#include "shortcutsettingswidget.h"
#include <QTreeWidgetItem>
#include <QKeyEvent>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidgetItem>
#include <QApplication>
#include <QDebug>

extern QKeySequence keySequenceFromEditString(const QString &editString);
extern QString keySequenceToEditString(const QKeySequence &sequence);
extern int translateModifiers(Qt::KeyboardModifiers state,
                                         const QString &text);
extern bool keySequenceIsValid(const QKeySequence &sequence);

Q_DECLARE_METATYPE(ShortcutItem*)
ShortcutSettingsWidget::ShortcutSettingsWidget(QWidget *parent)
    : CommandMappings(parent)
{
    setPageTitle(tr("Keyboard Shortcuts"));
    setTargetHeader(tr("Shortcut"));

//    connect(ActionManager::instance(), &ActionManager::commandListChanged,
//            this, &ShortcutSettingsWidget::initialize);
    connect(this, &ShortcutSettingsWidget::currentCommandChanged,
            this, &ShortcutSettingsWidget::handleCurrentCommandChanged);

    m_shortcutBox = new QGroupBox(tr("Shortcut"), this);
    m_shortcutBox->setEnabled(false);
    auto vboxLayout = new QVBoxLayout(m_shortcutBox);
    m_shortcutBox->setLayout(vboxLayout);
    auto hboxLayout = new QHBoxLayout;
    vboxLayout->addLayout(hboxLayout);
    m_shortcutEdit = new QLineEdit(m_shortcutBox);
//    m_shortcutEdit->setFiltering(true);
    m_shortcutEdit->setText(tr("Enter key sequence as text"));
    auto shortcutLabel = new QLabel(tr("Key sequence:"));
    shortcutLabel->setToolTip(  QLatin1String("<html><body>")
             + tr("Use \"Ctrl\", \"Alt\", \"Meta\", and \"Shift\" for modifier keys. "
                  "Use \"Escape\", \"Backspace\", \"Delete\", \"Insert\", \"Home\", and so on, for special keys. "
                  "Combine individual keys with \"+\", "
                  "and combine multiple shortcuts to a shortcut sequence with \",\". "
                  "For example, if the user must hold the Ctrl and Shift modifier keys "
                  "while pressing Escape, and then release and press A, "
                  "enter \"Ctrl+Shift+Escape,A\".")
             + QLatin1String("</body></html>"));
    auto shortcutButton = new ShortcutButton(m_shortcutBox);
    connect(shortcutButton, &ShortcutButton::keySequenceChanged,
            this, &ShortcutSettingsWidget::setKeySequence);
    auto resetButton = new QPushButton(tr("Reset"), m_shortcutBox);
    resetButton->setToolTip(tr("Reset to default."));
    connect(resetButton, &QPushButton::clicked,
            this, &ShortcutSettingsWidget::resetToDefault);
    hboxLayout->addWidget(shortcutLabel);
    hboxLayout->addWidget(m_shortcutEdit);
    hboxLayout->addWidget(shortcutButton);
    hboxLayout->addWidget(resetButton);

    m_warningLabel = new QLabel(m_shortcutBox);
    m_warningLabel->setTextFormat(Qt::RichText);
    QPalette palette = m_warningLabel->palette();
    palette.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
    m_warningLabel->setPalette(palette);
    connect(m_warningLabel, &QLabel::linkActivated, this, &ShortcutSettingsWidget::showConflicts);
    vboxLayout->addWidget(m_warningLabel);

    layout()->addWidget(m_shortcutBox);
    QWidget *applyW = new QWidget;
    auto hboxLayout2 = new QHBoxLayout;
    hboxLayout2->addStretch();
    auto applyButton = new QPushButton(tr("Save"), applyW);
    connect(applyButton, &QPushButton::clicked,
            this, &ShortcutSettingsWidget::save);
    hboxLayout2->addWidget(applyButton);

    applyW->setLayout(hboxLayout2);
    layout()->addWidget(applyW);

//    initialize();

//    m_shortcutEdit->setValidationFunction([this](Utils::FancyLineEdit *, QString *) {
//        return validateShortcutEdit();
//    });
}

//CommandsFile::CommandsFile(const QString &filename)
//    : m_filename(filename)
//{

//}


void ShortcutSettingsWidget::save()
{
    exportAction();
    QPalette palette = m_warningLabel->palette();
    palette.setColor(QPalette::Active, QPalette::WindowText, Qt::green);
    m_warningLabel->setPalette(palette);
    m_warningLabel->setText(tr("Shortcut key is saved."));

}




void ShortcutSettingsWidget::handleCurrentCommandChanged(QTreeWidgetItem *current)
{

    if (!current || !current->data(0, Qt::UserRole).isValid()) {
        m_shortcutEdit->clear();
        m_warningLabel->clear();
        m_shortcutBox->setEnabled(false);
        return;
    } else {

        ShortcutItem *scitem = qvariant_cast<ShortcutItem *>(current->data(0, Qt::UserRole));
//        setKeySequence(scitem->m_key);
        m_shortcutEdit->setText(keySequenceToEditString(scitem->m_key));
        markCollisions(scitem);

        m_shortcutBox->setEnabled(true);

    }
}

bool ShortcutSettingsWidget::validateShortcutEdit() const
{
    m_warningLabel->clear();
    QPalette palette = m_warningLabel->palette();
    palette.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
    m_warningLabel->setPalette(palette);

    QTreeWidgetItem *current = commandList()->currentItem();
    if (!current || !current->data(0, Qt::UserRole).isValid())
        return true;

    ShortcutItem *item = qvariant_cast<ShortcutItem *>(current->data(0, Qt::UserRole));


    bool valid = false;

    const QString text = m_shortcutEdit->text().trimmed();

    const QKeySequence currentKey = keySequenceFromEditString(text);

    if (keySequenceIsValid(currentKey) || text.isEmpty()) {
        item->m_key = currentKey;
        auto that = const_cast<ShortcutSettingsWidget *>(this);
        if (item->m_default_key != item->m_key)
            that->setModified(current, true);
        else
            that->setModified(current, false);
        current->setText(2, item->m_key.toString(QKeySequence::NativeText));
        qDebug() << "item->m_key.toString(QKeySequence::NativeText)" << item->m_key.toString(QKeySequence::NativeText);

        valid = !that->markCollisions(item);
        if (!valid) {
            m_warningLabel->setText(
                        tr("Key sequence has potential conflicts. <a href=\"#conflicts\">Show.</a>"));
        }
    } else {
        m_warningLabel->setText(tr("Invalid key sequence."));
    }
    return valid;
}

bool ShortcutSettingsWidget::filterColumn(const QString &filterString, QTreeWidgetItem *item,
                                          int column) const
{
    QString text;
    if (column == item->columnCount() - 1) { // shortcut
        // filter on the shortcut edit text
        if (!item->data(0, Qt::UserRole).isValid())
            return true;
        ShortcutItem *scitem = qvariant_cast<ShortcutItem *>(item->data(0, Qt::UserRole));
        text = keySequenceToEditString(scitem->m_key);
    } else if (column == 0 && item->data(0, Qt::UserRole).isValid()) { // command id
        ShortcutItem *scitem = qvariant_cast<ShortcutItem *>(item->data(0, Qt::UserRole));
        text = scitem->m_name;
    } else {
        text = item->text(column);
    }
    return !text.contains(filterString, Qt::CaseInsensitive);
}

void ShortcutSettingsWidget::setKeySequence(const QKeySequence &key)
{

    m_shortcutEdit->setText(keySequenceToEditString(key));
    QTreeWidgetItem *current = commandList()->currentItem();
    ShortcutItem *scitem = qvariant_cast<ShortcutItem *>(current->data(0, Qt::UserRole));
    scitem->m_key = key;
    //        current->setText(2, scitem->m_key.toString(QKeySequence::NativeText));
    validateShortcutEdit();

}

void ShortcutSettingsWidget::showConflicts()
{
    QTreeWidgetItem *current = commandList()->currentItem();
    if (current && current->data(0, Qt::UserRole).isValid()) {
        ShortcutItem *scitem = qvariant_cast<ShortcutItem *>(current->data(0, Qt::UserRole));
        setFilterText(keySequenceToEditString(scitem->m_key));
    }
}

void ShortcutSettingsWidget::resetToDefault()
{
    QTreeWidgetItem *current = commandList()->currentItem();
    if (current && current->data(0, Qt::UserRole).isValid()) {
        ShortcutItem *scitem = qvariant_cast<ShortcutItem *>(current->data(0, Qt::UserRole));
        setKeySequence(scitem->m_default_key);
        foreach (ShortcutItem *item, m_scitems)
            markCollisions(item);
    }
}

void ShortcutSettingsWidget::importAction()
{
    if (!m_setting_filename.isEmpty()) {

        CommandsFile cf(m_setting_filename);
        QMap<QString, QKeySequence> mapping = cf.importCommands();

        foreach (ShortcutItem *item, m_scitems) {
            QString sid = item->m_name;
            if (mapping.contains(sid)) {
                item->m_key = mapping.value(sid);
                item->m_item->setText(2, item->m_key.toString(QKeySequence::NativeText));
                if (item->m_item == commandList()->currentItem())
                    currentCommandChanged(item->m_item);

                if (item->m_default_key != item->m_key)
                    setModified(item->m_item, true);
                else
                    setModified(item->m_item, false);
            }
        }

        foreach (ShortcutItem *item, m_scitems)
            markCollisions(item);
    }
}

void ShortcutSettingsWidget::defaultAction()
{
    m_warningLabel->clear();
    foreach (ShortcutItem *item, m_scitems) {
        item->m_key = item->m_default_key;
        item->m_item->setText(2, item->m_key.toString(QKeySequence::NativeText));
        setModified(item->m_item, false);
        if (item->m_item == commandList()->currentItem())
            currentCommandChanged(item->m_item);
    }

    foreach (ShortcutItem *item, m_scitems)
        markCollisions(item);
}

void ShortcutSettingsWidget::exportAction()
{
    if (!m_setting_filename.isEmpty()) {
        CommandsFile cf(m_setting_filename);
        cf.exportCommands(m_scitems);
    }
}
void ShortcutSettingsWidget::getKeySequence(QMap<QString, QKeySequence>& mapping) const
{
    mapping.clear();
    foreach (const ShortcutItem *item, m_scitems)
    {
        const QString id = item->m_name;

        if (item->m_key.isEmpty()) {

            mapping[id] = item->m_key;
        }
    }
}

void ShortcutSettingsWidget::clear()
{
    QTreeWidget *tree = commandList();
    for (int i = tree->topLevelItemCount()-1; i >= 0 ; --i) {
        delete tree->takeTopLevelItem(i);
    }
    qDeleteAll(m_scitems);
    m_scitems.clear();
}

void ShortcutSettingsWidget::initialize(const QList<ShortcutItem*>& itemIn)
{
    clear();
    m_scitems = itemIn;
    QMap<QString, QTreeWidgetItem *> sections;

    foreach (auto& s, m_scitems) {
//        QTreeWidgetItem *item = 0;
//        ShortcutItem *s = new ShortcutItem;
//        m_scitems << s;
//        item = new QTreeWidgetItem;
//        s->m_cmd = c;
//        s->m_item = item;

        const QString identifier = s->m_name;
        int pos = identifier.indexOf(QLatin1Char('.'));
        const QString section = identifier.left(pos);
        const QString subId = identifier.mid(pos + 1);
        if (!sections.contains(section)) {
            QTreeWidgetItem *categoryItem = new QTreeWidgetItem(commandList(), QStringList(section));
            QFont f = categoryItem->font(0);
            f.setBold(true);
            categoryItem->setFont(0, f);
            sections.insert(section, categoryItem);
            commandList()->expandItem(categoryItem);
        }
        sections[section]->addChild(s->m_item);
        commandList()->setColumnWidth(1, 300);
        s->m_item->setText(0, subId);
        s->m_item->setText(1, s->m_desp);
        s->m_item->setText(2, s->m_key.toString(QKeySequence::NativeText));
        if (s->m_default_key != s->m_key)
            setModified(s->m_item, true);

        s->m_item->setData(0, Qt::UserRole, qVariantFromValue(s));

        markCollisions(s);
    }
    filterChanged(filterText());
}

bool ShortcutSettingsWidget::markCollisions(ShortcutItem *item)
{
    bool hasCollision = false;
    if (!item->m_key.isEmpty()) {
        foreach (ShortcutItem *currentItem, m_scitems) {
            currentItem->m_item->setForeground(2, Qt::black);
            if (currentItem->m_key.isEmpty() || item == currentItem
                    || item->m_key != currentItem->m_key)
                continue;

                currentItem->m_item->setForeground(2, Qt::red);
                hasCollision = true;
        }
    }
    if(hasCollision)
        item->m_item->setForeground(2, Qt::red);
    return hasCollision;
}


ShortcutSettingsWidget::~ShortcutSettingsWidget()
{
    qDeleteAll(m_scitems);
}
