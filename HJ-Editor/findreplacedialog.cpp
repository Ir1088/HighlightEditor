#include "findreplacedialog.h"
#include <QLabel>
FindReplaceDialog::FindReplaceDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("查找/替换");

    findLineEdit = new QLineEdit(this);
    replaceLineEdit = new QLineEdit(this);
    caseSensitiveCheckBox = new QCheckBox("区分大小写", this);
    wholeWordsCheckBox = new QCheckBox("全词匹配", this);
    regexCheckBox = new QCheckBox("使用正则表达式", this);
    findButton = new QPushButton("查找", this);
    replaceButton = new QPushButton("替换", this);
    replaceAllButton = new QPushButton("全部替换", this);
    closeButton = new QPushButton("关闭", this);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *findLayout = new QHBoxLayout;
    QHBoxLayout *replaceLayout = new QHBoxLayout;
    QHBoxLayout *optionsLayout = new QHBoxLayout;
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    findLayout->addWidget(new QLabel("查找:", this));
    findLayout->addWidget(findLineEdit);
    replaceLayout->addWidget(new QLabel("替换为:", this));
    replaceLayout->addWidget(replaceLineEdit);
    optionsLayout->addWidget(caseSensitiveCheckBox);
    optionsLayout->addWidget(wholeWordsCheckBox);
    optionsLayout->addWidget(regexCheckBox);
    buttonLayout->addWidget(findButton);
    buttonLayout->addWidget(replaceButton);
    buttonLayout->addWidget(replaceAllButton);
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(findLayout);
    mainLayout->addLayout(replaceLayout);
    mainLayout->addLayout(optionsLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    connect(findButton, &QPushButton::clicked, this, &FindReplaceDialog::onFindClicked);
    connect(replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceClicked);
    connect(replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceAllClicked);
    connect(closeButton, &QPushButton::clicked, this, &FindReplaceDialog::close);
}

FindReplaceDialog::~FindReplaceDialog()
{
}

void FindReplaceDialog::onFindClicked()
{
    QString findText = findLineEdit->text();
    bool caseSensitive = caseSensitiveCheckBox->isChecked();
    bool wholeWords = wholeWordsCheckBox->isChecked();
    bool regex = regexCheckBox->isChecked();

    emit find(findText, caseSensitive, wholeWords, regex);
}

void FindReplaceDialog::onReplaceClicked()
{
    QString findText = findLineEdit->text();
    QString replaceText = replaceLineEdit->text();
    bool caseSensitive = caseSensitiveCheckBox->isChecked();
    bool wholeWords = wholeWordsCheckBox->isChecked();
    bool regex = regexCheckBox->isChecked();

    emit replace(findText, replaceText, caseSensitive, wholeWords, regex);
}

void FindReplaceDialog::onReplaceAllClicked()
{
    QString findText = findLineEdit->text();
    QString replaceText = replaceLineEdit->text();
    bool caseSensitive = caseSensitiveCheckBox->isChecked();
    bool wholeWords = wholeWordsCheckBox->isChecked();
    bool regex = regexCheckBox->isChecked();

    emit replaceAll(findText, replaceText, caseSensitive, wholeWords, regex);
}
