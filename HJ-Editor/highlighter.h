#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

// 支持的语言类型
enum LanguageType {
    Cpp,
    Python,
    JSON
};

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument *parent = nullptr, LanguageType lang = Cpp);

    // 根据文件名或内容自动检测语言类型
    static LanguageType detectLanguage(const QString &fileName, const QString &content = "");

protected:
    void highlightBlock(const QString &text) override;

private:
    // 高亮规则结构体
    struct HighlightingRule {
        QRegularExpression pattern;  // 匹配正则表达式
        QTextCharFormat format;      // 格式（颜色、字体等）
    };

    QVector<HighlightingRule> highlightingRules;  // 规则列表

    // 多行注释相关
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QTextCharFormat multiLineCommentFormat;

    // 不同语言的格式定义
    QTextCharFormat keywordFormat;       // 关键字
    QTextCharFormat classFormat;         // 类名
    QTextCharFormat singleLineCommentFormat;  // 单行注释
    QTextCharFormat quotationFormat;     // 字符串/引号内容
    QTextCharFormat functionFormat;      // 函数名
    QTextCharFormat numberFormat;        // 数字（JSON/Python）
    QTextCharFormat jsonKeyFormat;       // JSON键
    QTextCharFormat jsonSeparatorFormat; // JSON分隔符

    // 初始化指定语言的规则
    void initCppRules();
    void initPythonRules();
    void initJsonRules();
};

#endif // HIGHLIGHTER_H
