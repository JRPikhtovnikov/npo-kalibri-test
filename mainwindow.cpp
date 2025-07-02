#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    fileProcessor = new FileProcessor();
    processorThread = new QThread(this);
    fileProcessor->moveToThread(processorThread);
    processorThread->start();

    timer = new QTimer(this);

    connect(fileProcessor, &FileProcessor::progressUpdated, this, &MainWindow::updateProgress);
    connect(fileProcessor, &FileProcessor::finished, this, &MainWindow::processingFinished);
    connect(fileProcessor, &FileProcessor::fileProcessed, this, &MainWindow::fileProcessed);
    connect(timer, &QTimer::timeout, this, &MainWindow::startProcessing);

    loadSettings();
    updateUiFromSettings();
}

MainWindow::~MainWindow()
{
    processorThread->quit();
    processorThread->wait();
    delete fileProcessor;
    delete ui;
}

void MainWindow::loadSettings()
{
    QSettings qsettings("FileXorProcessor", "Settings");
    settings.inputMask = qsettings.value("inputMask", "*.txt").toString();
    settings.deleteInput = qsettings.value("deleteInput", false).toBool();
    settings.outputPath = qsettings.value("outputPath", QDir::homePath()).toString();
    settings.fileConflictResolution = static_cast<FileConflictResolution>(
        qsettings.value("fileConflictResolution", 0).toInt());
    settings.useTimer = qsettings.value("useTimer", false).toBool();
    settings.timerInterval = qsettings.value("timerInterval", 1000).toInt();
    settings.xorValue = qsettings.value("xorValue", "0000000000000000").toString();
}

void MainWindow::saveSettings()
{
    QSettings qsettings("FileXorProcessor", "Settings");
    qsettings.setValue("inputMask", settings.inputMask);
    qsettings.setValue("deleteInput", settings.deleteInput);
    qsettings.setValue("outputPath", settings.outputPath);
    qsettings.setValue("fileConflictResolution", static_cast<int>(settings.fileConflictResolution));
    qsettings.setValue("useTimer", settings.useTimer);
    qsettings.setValue("timerInterval", settings.timerInterval);
    qsettings.setValue("xorValue", settings.xorValue);
}

void MainWindow::updateUiFromSettings()
{
    ui->inputMaskEdit->setText(settings.inputMask);
    ui->deleteInputCheckBox->setChecked(settings.deleteInput);
    ui->outputPathEdit->setText(settings.outputPath);
    ui->conflictComboBox->setCurrentIndex(static_cast<int>(settings.fileConflictResolution));
    ui->timerCheckBox->setChecked(settings.useTimer);
    ui->timerIntervalSpinBox->setValue(settings.timerInterval);
    ui->xorValueEdit->setText(settings.xorValue);
    ui->timerIntervalSpinBox->setEnabled(settings.useTimer);
}

void MainWindow::updateSettingsFromUi()
{
    settings.inputPath = ui->inputPathEdit->text();
    settings.inputMask = ui->inputMaskEdit->text();
    settings.deleteInput = ui->deleteInputCheckBox->isChecked();
    settings.outputPath = ui->outputPathEdit->text();
    settings.fileConflictResolution = static_cast<FileConflictResolution>(
        ui->conflictComboBox->currentIndex());
    settings.useTimer = ui->timerCheckBox->isChecked();
    settings.timerInterval = ui->timerIntervalSpinBox->value();
    settings.xorValue = ui->xorValueEdit->text();
}

void MainWindow::on_startButton_clicked()
{
    updateSettingsFromUi();
    saveSettings();

    if (settings.useTimer) {
        timer->start(settings.timerInterval);
        ui->statusLabel->setText(tr("Начало обработки (режим таймера)"));
    } else {
        startProcessing();
    }
}

void MainWindow::on_stopButton_clicked()
{
    if (timer->isActive()) {
        timer->stop();
    }
    fileProcessor->stopProcessing();
    ui->statusLabel->setText(tr("Обработка остановлена"));
}

void MainWindow::startProcessing()
{
    ui->progressBar->setValue(0);
    ui->fileListWidget->clear();
    ui->statusLabel->setText(tr("Обработка..."));

    fileProcessor->setSettings(settings);
    QMetaObject::invokeMethod(fileProcessor, "processFiles", Qt::QueuedConnection);
}

void MainWindow::on_browseInputButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выберите входную дирректорию"),
                                                    QDir::homePath());
    if (!dir.isEmpty()) {
        ui->inputPathEdit->setText(dir);
    }
}

void MainWindow::on_browseOutputButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Выберите выходную дирректорию"),
                                                    QDir::homePath());
    if (!dir.isEmpty()) {
        ui->outputPathEdit->setText(dir);
    }
}

void MainWindow::updateProgress(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::processingFinished()
{
    ui->statusLabel->setText(tr("Обработка завершена"));
}

void MainWindow::fileProcessed(const QString &filename)
{
    ui->fileListWidget->addItem(filename);
}

void MainWindow::on_timerCheckBox_stateChanged(int arg1)
{
    ui->timerIntervalSpinBox->setEnabled(arg1 == Qt::Checked);
}
