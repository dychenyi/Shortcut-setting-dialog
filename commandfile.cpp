#include "commandfile.h"

#include <QKeySequence>
#include <QFile>
#include <QXmlStreamAttributes>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMap>
#include <QDateTime>



struct Context // XML parsing context with strings.
{
    Context();

    const QString mappingElement;
    const QString shortCutElement;
    const QString idAttribute;
    const QString keyElement;
    const QString valueAttribute;
};

Context::Context() :
    mappingElement(QLatin1String("mapping")),
    shortCutElement(QLatin1String("shortcut")),
    idAttribute(QLatin1String("id")),
    keyElement(QLatin1String("key")),
    valueAttribute(QLatin1String("value"))
{
}

/*!
    \class CommandsFile
    \brief The CommandsFile class provides a collection of import and export commands.
    \inheaderfile commandsfile.h
*/

/*!
    ...
*/
CommandsFile::CommandsFile(const QString &filename)
    : m_filename(filename)
{

}

/*!
    ...
*/
QMap<QString, QKeySequence> CommandsFile::importCommands() const
{
    QMap<QString, QKeySequence> result;

    QFile file(m_filename);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return result;

    Context ctx;
    QXmlStreamReader r(&file);

    QString currentId;

    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::StartElement: {
            const QStringRef name = r.name();

            if (name == ctx.shortCutElement) {
                if (!currentId.isEmpty()) // shortcut element without key element == empty shortcut
                    result.insert(currentId, QKeySequence());
                currentId = r.attributes().value(ctx.idAttribute).toString();
            } else if (name == ctx.keyElement) {

                const QXmlStreamAttributes attributes = r.attributes();
                if (attributes.hasAttribute(ctx.valueAttribute)) {
                    const QString keyString = attributes.value(ctx.valueAttribute).toString();
                    result.insert(currentId, QKeySequence(keyString));
                } else {
                    result.insert(currentId, QKeySequence());
                }
                currentId.clear();
            } // if key element
        } // case QXmlStreamReader::StartElement
        default:
            break;
        } // switch
    } // while !atEnd
    file.close();
    return result;
}

/*!
    ...
*/

bool CommandsFile::exportCommands(const QList<ShortcutItem *> &items, bool resetToDefault)
{
    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;


    const Context ctx;
    QXmlStreamWriter w(&file);
    w.setAutoFormatting(true);
    w.setAutoFormattingIndent(1); // Historical, used to be QDom.
    w.writeStartDocument();
    w.writeDTD(QLatin1String("<!DOCTYPE KeyboardMappingScheme>"));
    w.writeComment(QString::fromLatin1(" Written by %1, %2. ").
                   arg("Shortcut Setting",
                       QDateTime::currentDateTime().toString(Qt::ISODate)));
    w.writeStartElement(ctx.mappingElement);
    foreach (const ShortcutItem *item, items) {
        const QString id = item->m_name;

        if (item->m_key.isEmpty()) {
            w.writeEmptyElement(ctx.shortCutElement);
            w.writeAttribute(ctx.idAttribute, id);
        } else {
            w.writeStartElement(ctx.shortCutElement);
            w.writeAttribute(ctx.idAttribute, id);
            w.writeEmptyElement(ctx.keyElement);
            if(resetToDefault)
                w.writeAttribute(ctx.valueAttribute, item->m_default_key.toString());
            else
                w.writeAttribute(ctx.valueAttribute, item->m_key.toString());
            w.writeEndElement(); // Shortcut
        }
    }
    w.writeEndElement();
    w.writeEndDocument();



    return true;
}
