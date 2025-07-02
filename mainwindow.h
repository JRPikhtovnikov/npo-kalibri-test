#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "fileprocessor.h"
#include "settings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_browseInputButton_clicked();
    void on_browseOutputButton_clicked();
    void updateProgress(int value);
    void processingFinished();
    void fileProcessed(const QString &filename);
    void on_timerCheckBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    FileProcessor *fileProcessor;
    QThread *processorThread;
    QTimer *timer;
    Settings settings;

    void loadSettings();
    void saveSettings();
    void updateUiFromSettings();
    void updateSettingsFromUi();
    void startProcessing();
};
#endif // MAINWINDOW_H
