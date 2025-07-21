#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    FindReplaceDialog(QWidget *parent = nullptr);
    ~FindReplaceDialog();

signals:
    void find(const QString &text, bool caseSensitive, bool wholeWords, bool regex);
    void replace(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool regex);
    void replaceAll(const QString &findText, const QString &replaceText, bool caseSensitive, bool wholeWords, bool regex);

private slots:
    void onFindClicked();
    void onReplaceClicked();
    void onReplaceAllClicked();

private:
    QLineEdit *findLineEdit;
    QLineEdit *replaceLineEdit;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *wholeWordsCheckBox;
    QCheckBox *regexCheckBox;
    QPushButton *findButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
    QPushButton *closeButton;
};

#endif // FINDREPLACEDIALOG_H
