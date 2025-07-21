#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "highlighter.h"
#include <QRegularExpression>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include "findreplacedialog.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QIcon runIcon;
    QIcon stopIcon;
    Ui::MainWindow *ui;
    Highlighter *highlighter;
    QProcess process;
    void setUpHighlighter();
    //---------记录文件信息----------
    QString fileName;
    QString filePath;
    bool fileSaved;
    bool isRunning;
    //bool fileEdited;
    void initFileData();
    bool firstLoad;
    //-----------------------------

    //---------code running data---
    QString output;
    QString error;
    //-----------------------------
    FindReplaceDialog *findReplaceDialog;

public slots:
    void changeSaveState();
    //---------工具栏响应函数---------
    void newFile();
    void saveFile();
    void openFile();
    void undo();
    void redo();
    void run();
    void applyLightTheme();
    void applyDarkTheme();
    //------------------------------
    void runFinished(int code);
    void updateOutput();
    void updateError();
    void about();
    void openFindReplaceDialog();
    void findText(const QString &text, bool caseSensitive, bool wholeWords, bool regex);
    void replaceText(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool regex);
    void replaceAllText(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool regex);

public:
    void inputData(QString data);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
};

#endif // MAINWINDOW_H
