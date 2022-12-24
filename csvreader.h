/**
 * file csvreader.h
 * Found on the web and edited
 **/
#ifndef CSVREADER_H
#define CSVREADER_H

#include <QObject>
#include <QFile>

class CsvReader: public QObject
{
    Q_OBJECT

private:
    QFile csvFile;
    QChar separator;
    bool utf8Enc;

    QString trimCsv(QString &item);

public:
    CsvReader(QObject *parent = 0,
              const QString &file_name = QString(""), bool encoding = false);

    ~CsvReader();

    void setFileName(const QString &name);
    bool fileOpen();
    void csvRead(QList<QStringList> &list);
    void fileClose();
    bool isOpen() const;
    void setSeparator(QChar sep);
    QChar getSeparator();
};
#endif // CSVREADER_H
