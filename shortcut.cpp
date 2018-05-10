#include "shortcut.h"

#include <QKeyEvent>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidgetItem>
#include <QApplication>

int translateModifiers(Qt::KeyboardModifiers state,
                                         const QString &text)
{
    int result = 0;
    // The shift modifier only counts when it is not used to type a symbol
    // that is only reachable using the shift key anyway
    if ((state & Qt::ShiftModifier) && (text.size() == 0
                                        || !text.at(0).isPrint()
                                        || text.at(0).isLetterOrNumber()
                                        || text.at(0).isSpace()))
        result |= Qt::SHIFT;
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}
QString keySequenceToEditString(const QKeySequence &sequence)
{
    QString text = sequence.toString(QKeySequence::PortableText);

    return text;
}

QKeySequence keySequenceFromEditString(const QString &editString)
{
    QString text = editString;

    return QKeySequence::fromString(text, QKeySequence::PortableText);
}

bool keySequenceIsValid(const QKeySequence &sequence)
{
    if (sequence.isEmpty())
        return false;
    for (int i = 0; i < sequence.count(); ++i) {
        if (sequence[i] == Qt::Key_unknown)
            return false;
    }
    return true;
}

ShortcutButton::ShortcutButton(QWidget *parent)
    : QPushButton(parent)
    , m_key({{0, 0, 0, 0}})
{
    // Using ShortcutButton::tr() as workaround for QTBUG-34128
    setToolTip(ShortcutButton::tr("Click and type the new key sequence."));
    setCheckable(true);
    m_checkedText = ShortcutButton::tr("Stop Recording");
    m_uncheckedText = ShortcutButton::tr("Record");
    updateText();
    connect(this, &ShortcutButton::toggled, this, &ShortcutButton::handleToggleChange);
}

QSize ShortcutButton::sizeHint() const
{
    if (m_preferredWidth < 0) { // initialize size hint
        const QString originalText = text();
        ShortcutButton *that = const_cast<ShortcutButton *>(this);
        that->setText(m_checkedText);
        m_preferredWidth = QPushButton::sizeHint().width();
        that->setText(m_uncheckedText);
        int otherWidth = QPushButton::sizeHint().width();
        if (otherWidth > m_preferredWidth)
            m_preferredWidth = otherWidth;
        that->setText(originalText);
    }
    return QSize(m_preferredWidth, QPushButton::sizeHint().height());
}

bool ShortcutButton::eventFilter(QObject *obj, QEvent *evt)
{
    if (evt->type() == QEvent::ShortcutOverride) {
        evt->accept();
        return true;
    }
    if (evt->type() == QEvent::KeyRelease
               || evt->type() == QEvent::Shortcut
               || evt->type() == QEvent::Close/*Escape tries to close dialog*/) {
        return true;
    }
    if (evt->type() == QEvent::MouseButtonPress && isChecked()) {
        setChecked(false);
        return true;
    }
    if (evt->type() == QEvent::KeyPress) {
        QKeyEvent *k = static_cast<QKeyEvent*>(evt);
        int nextKey = k->key();
        if (m_keyNum > 3
                || nextKey == Qt::Key_Control
                || nextKey == Qt::Key_Shift
                || nextKey == Qt::Key_Meta
                || nextKey == Qt::Key_Alt) {
             return false;
        }

        nextKey |= translateModifiers(k->modifiers(), k->text());
        switch (m_keyNum) {
        case 0:
            m_key[0] = nextKey;
            break;
        case 1:
            m_key[1] = nextKey;
            break;
        case 2:
            m_key[2] = nextKey;
            break;
        case 3:
            m_key[3] = nextKey;
            break;
        default:
            break;
        }
        m_keyNum++;
        k->accept();
        emit keySequenceChanged(QKeySequence(m_key[0], m_key[1], m_key[2], m_key[3]));
        if (m_keyNum > 3)
            setChecked(false);
        return true;
    }
    return QPushButton::eventFilter(obj, evt);
}

void ShortcutButton::updateText()
{
    setText(isChecked() ? m_checkedText : m_uncheckedText);
}

void ShortcutButton::handleToggleChange(bool toogleState)
{
    updateText();
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;
    if (toogleState) {
        if (QApplication::focusWidget())
            QApplication::focusWidget()->clearFocus(); // funny things happen otherwise
        qApp->installEventFilter(this);
    } else {
        qApp->removeEventFilter(this);
    }
}
