#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QFileInfo>
#include <QVector>
#include "settings.h"

class FileProcessor : public QObject
{
    Q_OBJECT

public:
    explicit FileProcessor(QObject *parent = nullptr);
    void setSettings(const Settings &newSettings);
    void stopProcessing();

public slots:
    void processFiles();

signals:
    void progressUpdated(int value);
    void finished();
    void fileProcessed(const QString &filename);

private:
    Settings settings;
    bool stopRequested;

    bool processSingleFile(const QFileInfo &fileInfo);
    QByteArray performXor(const QByteArray &data, quint64 xorValue);
    QString getOutputFileName(const QString &inputFileName) const;
    QString resolveFileNameConflict(const QString &filePath) const;
};

#endif // FILEPROCESSOR_H
