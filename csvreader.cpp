/**
 * file csvreader.cpp
 * Found on the web and edited
 **/
#include "csvreader.h"
#include <QTextStream>
#include <QTextCodec>

CsvReader::CsvReader(QObject *parent, const QString& file_name,
                     bool encoding) : QObject(parent)
{
    csvFile.setFileName(file_name);
    utf8Enc = encoding;
}

CsvReader::~CsvReader()
{
    fileClose();
}

/**
 * @brief CsvReader::setSeparator
 * @param sep
 */
void CsvReader::setSeparator(QChar sep)
{
    separator = sep;
}

/**
 * @brief CsvReader::getSeparator
 * @return
 */
QChar CsvReader::getSeparator()
{
    return separator;
}

/**
 * @brief CsvReader::fileOpen
 * @return
 */
bool CsvReader::fileOpen()
{
    if (!csvFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    else
        return true;
}

/**
 * @brief CsvReader::setFileName
 * @param name
 */
void CsvReader::setFileName(const QString &name)
{
    csvFile.setFileName(name);
}

/**
 * @brief CsvReader::fileClose
 */
void CsvReader::fileClose()
{
    if(isOpen()) csvFile.close();
}

/**
 * @brief CsvReader::isOpen
 * @return
 */
bool CsvReader::isOpen() const
{
    return csvFile.isOpen();
}

/**
 * @brief trimCsv
 * @param item
 * @return
 */
QString CsvReader::trimCsv(QString &item)
{
    if ((!item.isEmpty()) && (item[0] == QChar(34)))
        item.remove(0, 1);
    if ((!item.isEmpty()) && (!item.isNull()) && (item[item.count() - 1] == QChar(34)))
        item.remove(item.count() - 1, 1);
    if (!item.isEmpty())
        item = item.replace("\"\"", "\"");
    return item;
}

/**
 * @brief CsvReader::csvRead
 * @param list
 */
void CsvReader::csvRead(QList<QStringList> &list)
{
    if (csvFile.isOpen()) {
        bool Quote = false;
        QList<QString> ItemList;
        QString item = "";
        QTextStream in(&csvFile);

        if (utf8Enc) {
            in.setCodec(QTextCodec::codecForName("UTF-8"));
        }
        else {
            in.setCodec(QTextCodec::codecForName("CP1251"));
        }

        while (!in.atEnd()) {
            QString line(in.readLine().simplified());
            int count = line.count();
            for (int i = 0; i < count; i++) {
                if (line[i] == QChar(34)) Quote = !Quote;
                if ((Quote != true) && (line[i] == separator)) {
                    ItemList.append(trimCsv(item));
                    item = "";
                }
                else {
                    item += line[i];
                }
                if ((count-1 == i) && (Quote != true)) {
                    item = trimCsv(item);
                    if (item != "") ItemList.append(item);
                    list.append(ItemList);
                    ItemList.clear();
                    item = "";
                }
            }
        }
    }
    fileClose();
}
