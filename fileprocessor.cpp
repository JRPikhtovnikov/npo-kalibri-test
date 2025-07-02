#include "fileprocessor.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QDateTime>

FileProcessor::FileProcessor(QObject *parent) : QObject(parent), stopRequested(false)
{
}

void FileProcessor::setSettings(const Settings &newSettings)
{
    settings = newSettings;
}

void FileProcessor::stopProcessing()
{
    stopRequested = true;
}

void FileProcessor::processFiles()
{
    qDebug() << "Начало обработки в директории:" << settings.inputPath;

    QDir inputDir(settings.inputPath);
    if (!inputDir.exists()) {
        qDebug() << "Директория не найдена:" << settings.inputPath;
        emit finished();
        return;
    }

    qDebug() << "Директория содержит:" << inputDir.entryList();

    QStringList filters;
    filters << settings.inputMask;
    qDebug() << "Использование фильтров:" << filters;

    QFileInfoList fileList = inputDir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    qDebug() << "Найденные файлы:" << fileList.size();

    int totalFiles = fileList.size();
    int processedFiles = 0;

    foreach (const QFileInfo &fileInfo, fileList) {
        if (stopRequested) {
            break;
        }

        if (processSingleFile(fileInfo)) {
            processedFiles++;
            emit fileProcessed(fileInfo.fileName());
        }

        int progress = static_cast<int>((processedFiles * 100) / totalFiles);
        emit progressUpdated(progress);
    }

    emit finished();
}

bool FileProcessor::processSingleFile(const QFileInfo &fileInfo)
{
    QFile inputFile(fileInfo.absoluteFilePath());
    if (!inputFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray fileData = inputFile.readAll();
    inputFile.close();

    bool ok;
    quint64 xorValue = settings.xorValue.toULongLong(&ok, 16);
    if (!ok) {
        return false;
    }

    QByteArray processedData = performXor(fileData, xorValue);

    QString outputFilePath = getOutputFileName(fileInfo.fileName());
    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        return false;
    }

    outputFile.write(processedData);
    outputFile.close();

    if (settings.deleteInput) {
        inputFile.remove();
    }

    return true;
}

QByteArray FileProcessor::performXor(const QByteArray &data, quint64 xorValue)
{
    QByteArray result;
    result.resize(data.size());

    const char *input = data.constData();
    char *output = result.data();
    int size = data.size();

    for (int i = 0; i < size; ++i) {
        int xorByte = (xorValue >> (8 * (i % 8))) & 0xFF;
        output[i] = input[i] ^ xorByte;
    }

    return result;
}

QString FileProcessor::getOutputFileName(const QString &inputFileName) const
{
    QString outputFilePath = QDir(settings.outputPath).filePath(inputFileName);

    if (settings.fileConflictResolution == FileConflictResolution::Overwrite) {
        return outputFilePath;
    }

    if (!QFile::exists(outputFilePath)) {
        return outputFilePath;
    }

    return resolveFileNameConflict(outputFilePath);
}

QString FileProcessor::resolveFileNameConflict(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.completeBaseName();
    QString suffix = fileInfo.suffix();
    QString path = fileInfo.path();

    int counter = 1;
    QString newFilePath;
    do {
        newFilePath = QString("%1/%2_%3.%4")
        .arg(path)
            .arg(baseName)
            .arg(counter)
            .arg(suffix);
        counter++;
    } while (QFile::exists(newFilePath));

    return newFilePath;
}
