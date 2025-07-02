#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

enum class FileConflictResolution {
    Overwrite,
    Rename
};

struct Settings {
    QString inputPath;
    QString inputMask;
    bool deleteInput;
    QString outputPath;
    FileConflictResolution fileConflictResolution;
    bool useTimer;
    int timerInterval;
    QString xorValue;
};

#endif // SETTINGS_H
