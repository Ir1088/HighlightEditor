#include "highlighter.h"
#include <QFont>
#include <QColor>
#include <QFileInfo>

Highlighter::Highlighter(QTextDocument *parent, LanguageType lang)
    : QSyntaxHighlighter(parent)
{
    // 根据语言类型初始化规则
    switch (lang) {
        case Cpp: initCppRules(); break;
        case Python: initPythonRules(); break;
        case JSON: initJsonRules(); break;
    }
}

// 核心高亮逻辑
void Highlighter::highlightBlock(const QString &text)
{
    // 应用所有单行规则
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // 处理多行注释
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;

        if (endIndex == -1) {
            // 多行注释未结束，延续到下一行
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }

        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

// 初始化C++高亮规则
void Highlighter::initCppRules()
{
    // 关键字格式（加粗、粉红色）
    keywordFormat.setForeground(QColor(201, 81, 116));
    keywordFormat.setFontWeight(QFont::Bold);
    // C++关键字列表
    QStringList keywordPatterns = {
        "\\bchar\\b", "\\bclass\\b", "\\bconst\\b", "\\bdouble\\b", "\\benum\\b",
        "\\bexplicit\\b", "\\bfriend\\b", "\\binline\\b", "\\bint\\b", "\\blong\\b",
        "\\bnamespace\\b", "\\boperator\\b", "\\bprivate\\b", "\\bprotected\\b",
        "\\bpublic\\b", "\\bshort\\b", "\\bsignals\\b", "\\bsigned\\b", "\\bslots\\b",
        "\\bstatic\\b", "\\bstruct\\b", "\\btemplate\\b", "\\btypedef\\b", "\\btypename\\b",
        "\\bunion\\b", "\\bunsigned\\b", "\\bvirtual\\b", "\\bvoid\\b", "\\bvolatile\\b",
        "\\bbool\\b", "\\busing\\b", "\\bconstexpr\\b", "\\bsizeof\\b", "\\bif\\b",
        "\\bfor\\b", "\\bwhile\\b", "\\bdo\\b", "\\bcase\\b", "\\bbreak\\b", "\\bcontinue\\b",
        "\\bdelete\\b", "\\bnew\\b", "\\bdefault\\b", "\\btry\\b", "\\breturn\\b",
        "\\bthrow\\b", "\\bcatch\\b", "\\bgoto\\b", "\\belse\\b", "\\bthis\\b", "\\bswitch\\b"
    };
    // 添加关键字规则
    for (const QString &pattern : keywordPatterns) {
        highlightingRules.append({QRegularExpression(pattern), keywordFormat});
    }

    // 类名格式（深洋红色、加粗）
    classFormat.setForeground(Qt::darkMagenta);
    classFormat.setFontWeight(QFont::Bold);
    highlightingRules.append({QRegularExpression("(?<=class\\s)\\w+"), classFormat});

    // 单行注释格式（绿色）
    singleLineCommentFormat.setForeground(Qt::green);
    highlightingRules.append({QRegularExpression("//[^\n]*"), singleLineCommentFormat});

    // 多行注释格式（绿色）
    multiLineCommentFormat.setForeground(Qt::green);
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");

    // 字符串和头文件包含格式（深绿色）
    quotationFormat.setForeground(Qt::darkGreen);
    highlightingRules.append({QRegularExpression("\"[^\"]*\""), quotationFormat});  // 字符串
    highlightingRules.append({QRegularExpression("<[^\>]*>"), quotationFormat});   // 头文件
    highlightingRules.append({QRegularExpression("#include\\s+[<\"].*[>\"]"), quotationFormat});  // #include

    // 函数名格式（浅蓝色、斜体）
    functionFormat.setForeground(QColor(115, 182, 209));
    functionFormat.setFontItalic(true);
    highlightingRules.append({QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()"), functionFormat});
}

// 初始化Python高亮规则
void Highlighter::initPythonRules()
{
    // 关键字格式（蓝色、加粗）
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns = {
        "\\bdef\\b", "\\bclass\\b", "\\bif\\b", "\\belif\\b", "\\belse\\b",
        "\\bfor\\b", "\\bwhile\\b", "\\breturn\\b", "\\bimport\\b", "\\bfrom\\b",
        "\\bas\\b", "\\bpass\\b", "\\bbreak\\b", "\\bcontinue\\b", "\\bprint\\b",
        "\\bTrue\\b", "\\bFalse\\b", "\\bNone\\b", "\\btry\\b", "\\bexcept\\b",
        "\\braise\\b", "\\bfinally\\b", "\\bwith\\b", "\\blambda\\b"
    };
    for (const QString &pattern : keywordPatterns) {
        highlightingRules.append({QRegularExpression(pattern), keywordFormat});
    }

    // 单行注释格式（绿色）
    singleLineCommentFormat.setForeground(Qt::green);
    highlightingRules.append({QRegularExpression("#[^\n]*"), singleLineCommentFormat});

    // 字符串格式（深绿色，支持单引号和双引号）
    quotationFormat.setForeground(Qt::darkGreen);
    highlightingRules.append({QRegularExpression("'[^']*'"), quotationFormat});
    highlightingRules.append({QRegularExpression("\"[^\"]*\""), quotationFormat});

    // 函数名格式（深洋红色、斜体）
    functionFormat.setForeground(Qt::darkMagenta);
    functionFormat.setFontItalic(true);
    highlightingRules.append({QRegularExpression("(?<=def\\s)\\w+"), functionFormat});

    // 数字格式（红色）
    numberFormat.setForeground(Qt::red);
    highlightingRules.append({QRegularExpression("\\b\\d+\\b"), numberFormat});  // 整数
    highlightingRules.append({QRegularExpression("\\b\\d+\\.\\d+\\b"), numberFormat});  // 浮点数

    // Python无多行注释（用三引号实现，此处简化处理）
    commentStartExpression = QRegularExpression("'''");
    commentEndExpression = QRegularExpression("'''");
    multiLineCommentFormat.setForeground(Qt::green);
}

// 初始化JSON高亮规则
void Highlighter::initJsonRules()
{
    // JSON键格式（蓝色、加粗）
    jsonKeyFormat.setForeground(Qt::blue);
    jsonKeyFormat.setFontWeight(QFont::Bold);
    highlightingRules.append({QRegularExpression("\"[^\"]+\":"), jsonKeyFormat});

    // 字符串值格式（深绿色）
    quotationFormat.setForeground(Qt::darkGreen);
    highlightingRules.append({QRegularExpression("\"[^\"]*\""), quotationFormat});

    // 数字格式（红色）
    numberFormat.setForeground(Qt::red);
    highlightingRules.append({QRegularExpression("-?\\d+"), numberFormat});  // 整数
    highlightingRules.append({QRegularExpression("-?\\d+\\.\\d+"), numberFormat});  // 浮点数
    highlightingRules.append({QRegularExpression("-?\\d+[eE][+-]?\\d+"), numberFormat});  // 科学计数法

    // 布尔值和null（紫色）
    keywordFormat.setForeground(QColor(128, 0, 128));
    highlightingRules.append({QRegularExpression("\\btrue\\b"), keywordFormat});
    highlightingRules.append({QRegularExpression("\\bfalse\\b"), keywordFormat});
    highlightingRules.append({QRegularExpression("\\bnull\\b"), keywordFormat});

    // 分隔符格式（灰色）
    jsonSeparatorFormat.setForeground(Qt::gray);
    highlightingRules.append({QRegularExpression("[\\{\\}\\[\\],:]"), jsonSeparatorFormat});

    // JSON无注释，禁用多行注释处理
    commentStartExpression = QRegularExpression("");
    commentEndExpression = QRegularExpression("");
}

// 基于文件扩展名或内容识别语言类型
LanguageType Highlighter::detectLanguage(const QString &fileName, const QString &content)
{
    // 优先通过扩展名识别
    if (!fileName.isEmpty()) {
        QString suffix = QFileInfo(fileName).suffix().toLower();
        if (suffix == "cpp" || suffix == "c" || suffix == "h" || suffix == "cxx" || suffix == "hpp")
            return Cpp;
        if (suffix == "py")
            return Python;
        if (suffix == "json")
            return JSON;
    }

    // 若无扩展名或扩展名无法识别，通过代码特征识别
    if (!content.isEmpty()) {
        QString lowerContent = content.toLower();
        QString firstLine = content.split('\n').first().trimmed();

        // 检测JSON（严格检查，避免误判）
        bool hasBrace = lowerContent.contains("{") && lowerContent.contains("}");
        bool hasBracket = lowerContent.contains("[") && lowerContent.contains("]");
        bool hasColon = lowerContent.contains(":");
        if ((hasBrace || hasBracket) && hasColon) {
            return JSON;
        }

        // 检测Python（检查def、class、import等关键字，且无C++特征）
        bool hasPythonKeyword = lowerContent.contains("def ") || lowerContent.contains("class ")
                             || lowerContent.contains("import ") || lowerContent.contains("from ");
        bool hasCppFeature = lowerContent.contains("#include") || lowerContent.contains(";");
        if (hasPythonKeyword && !hasCppFeature) {
            return Python;
        }

        // 检测C++（检查#include、;、{等特征）
        if (lowerContent.contains("#include") || lowerContent.contains(";") || lowerContent.contains("class ")) {
            return Cpp;
        }
    }

    // 无法识别时默认返回C++
    return Cpp;
}
