#include <QtWidgets>
#include <QDebug>
#include "codeeditor.h"

// 构造函数：初始化代码编辑器
CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    // 创建行号显示区域
    lineNumberArea = new LineNumberArea(this);

    // 连接信号与槽：当文本块数量变化时更新行号区域宽度
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    // 当编辑器内容更新时更新行号区域
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    // 当光标位置变化时高亮当前行
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    // 当光标位置变化时显示代码补全窗口
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(showCompleteWidget()));

    // 初始化行号区域宽度
    updateLineNumberAreaWidth(0);
    // 高亮当前行
    highlightCurrentLine();

    // 设置编辑器颜色：行号区域颜色和编辑器背景色
    lineColor.setRgb(56,60,69);
    editorColor.setRgb(34,39,49);

    // 设置编辑器调色板：背景色和文本颜色
    QPalette p = this->palette();
    p.setColor(QPalette::Active, QPalette::Base, editorColor);
    p.setColor(QPalette::Inactive, QPalette::Base, editorColor);
    p.setColor(QPalette::Text, Qt::white);
    this->setPalette(p);

    // 初始化代码补全列表（添加C++关键字）
    setUpCompleteList();
    // 创建代码补全窗口
    completeWidget = new CompleteListWidget(this);
    completeWidget->hide();  // 默认隐藏
    // 设置补全窗口最大高度（最多显示5行）
    completeWidget->setMaximumHeight(fontMetrics().height()*5);
    completeState = CompleteState::Hide;  // 初始状态：隐藏补全窗口
}

// 计算行号区域的宽度（根据最大行号的位数动态调整）
int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;  // 位数初始化为1
    int max = qMax(1, blockCount());  // 获取最大行号

    // 计算最大行号的位数
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // 确保至少有3位宽，避免行数较少时行号区域过窄
    if (digits < 3) digits = 3;

    // 计算宽度：3像素边距 + 数字宽度 * 位数
    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

// 更新行号区域宽度（槽函数）
void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    // 设置编辑器左侧边距为行号区域宽度
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

// 更新行号区域（槽函数）：当编辑器内容滚动时调用
void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);  // 如果是垂直滚动，直接滚动行号区域
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());  // 否则只更新指定区域

    // 如果更新区域包含视口矩形，重新计算行号区域宽度
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

// 窗口大小变化事件处理
void CodeEditor::resizeEvent(QResizeEvent *e)
{
    // 调用基类的resizeEvent处理
    QPlainTextEdit::resizeEvent(e);

    // 获取内容区域矩形
    QRect cr = contentsRect();
    // 设置行号区域的几何位置和大小
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

// 高亮当前行
void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        // 设置高亮颜色（红色的浅色变体）
        QColor lineColor = QColor(Qt::red).lighter(160);

        // 设置选中格式：背景色和全宽选择
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);

        // 获取当前光标并清除其选择
        selection.cursor = textCursor();
        selection.cursor.clearSelection();

        // 添加到额外选择列表
        extraSelections.append(selection);
    }

    // 设置额外选择，实现高亮
    setExtraSelections(extraSelections);
}

// 绘制行号区域
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    // 创建绘制器
    QPainter painter(lineNumberArea);
    // 填充行号区域背景
    painter.fillRect(event->rect(), lineColor);

    // 获取第一个可见文本块
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();  // 块号（行号）
    // 计算文本块的顶部位置（考虑内容偏移）
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();  // 底部位置

    // 循环绘制所有可见行的行号
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            // 准备行号文本
            QString number = QString::number(blockNumber + 1);
            // 设置画笔颜色
            painter.setPen(Qt::lightGray);
            // 绘制行号文本（居中对齐）
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignCenter, number);
        }

        // 移动到下一个文本块
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

// 按键事件处理：实现代码补全、括号匹配等功能
void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    // 自动补全括号：Shift + (
    if (event->modifiers() == Qt::ShiftModifier && event->key() == 40) {
        this->insertPlainText(tr("()"));
        this->moveCursor(QTextCursor::PreviousCharacter);  // 将光标移到括号中间
    }
    // 自动补全引号：Shift + "
    else if (event->modifiers() == Qt::ShiftModifier && event->key() == 34) {
        this->insertPlainText(tr("\"\""));
        this->moveCursor(QTextCursor::PreviousCharacter);  // 将光标移到引号中间
    }
    // 处理上下方向键：在补全窗口显示时用于选择补全项
    else if (event->key() == 16777235 && completeState == CompleteState::Showing) {
        if (completeWidget->currentRow() > 0)
            completeWidget->setCurrentRow(completeWidget->currentRow() - 1);
    }
    else if (event->key() == 16777237 && (completeState == CompleteState::Showing)) {
        if (completeWidget->currentRow() < completeWidget->count() - 1)
            completeWidget->setCurrentRow(completeWidget->currentRow() + 1);
    }
    // 处理回车键：在补全窗口显示时应用选中的补全项
    else if (event->key() == Qt::Key_Return && (completeState == CompleteState::Showing)) {
        QString insertText = completeWidget->currentItem()->text();
        QString word = this->getWordOfCursor();

        // 忽略当前光标变化事件，避免递归
        completeState = CompleteState::Ignore;

        // 删除当前单词（从光标位置向前删除）
        for (int i = 0; i < word.count(); ++i)
            this->textCursor().deletePreviousChar();

        // 插入选中的补全项
        this->insertPlainText(insertText);

        // 特殊处理：如果是#include，将光标定位到尖括号中间
        if (insertText.contains(tr("#include")))
            this->moveCursor(QTextCursor::PreviousCharacter);

        // 隐藏补全窗口
        completeState = CompleteState::Hide;
        completeWidget->hide();
    }
    // 处理回车键：实现代码自动缩进
    else if (event->key() == Qt::Key_Return) {
        // 获取当前行的文本
        QString temp = this->document()->findBlockByLineNumber(this->textCursor().blockNumber()).text();

        // 先执行默认的回车键行为（插入换行）
        QPlainTextEdit::keyPressEvent(event);

        // 如果当前行是空行，直接返回
        if (temp.count() <= 0) return;

        // 复制当前行的缩进（空格和制表符）到新行
        foreach (const QChar &c, temp) {
            if (c.isSpace()) this->insertPlainText(c);
            else break;
        }

        // 智能缩进：如果当前行以)结尾且包含特定关键字，添加一个制表符
        if (temp.at(temp.count() - 1) == ')' && (temp.contains(tr("for(")) || temp.contains(tr("while("))
                                          || temp.contains(tr("switch(")) || temp.contains(tr("if("))))
            this->insertPlainText(tr("\t"));

        // 智能缩进：如果当前行以{结尾，添加缩进并自动补全}
        if (temp.at(temp.count() - 1) == '{') {
            this->insertPlainText(tr("\t"));

            // 保存当前光标位置
            QTextCursor cursor = this->textCursor();
            int pos = this->textCursor().position();

            // 插入换行和闭合括号
            this->insertPlainText(tr("\n"));
            foreach (const QChar &c, temp) {
                if (c.isSpace()) this->insertPlainText(c);
            }
            this->insertPlainText(tr("}"));

            // 将光标移回中间行
            cursor.setPosition(pos);
            this->setTextCursor(cursor);
        }
    }
    // 处理退格键：智能删除匹配的括号、引号等
    else if (event->key() == Qt::Key_Backspace) {
        switch (this->document()->characterAt(this->textCursor().position() - 1).toLatin1()) {
            case '(':
                QPlainTextEdit::keyPressEvent(event);
                // 如果后面的字符是匹配的)，则同时删除
                if (this->document()->characterAt(this->textCursor().position()) == ')') {
                    this->textCursor().deleteChar();
                }
                break;
            case '\"':
                QPlainTextEdit::keyPressEvent(event);
                // 如果后面的字符是匹配的"，则同时删除
                if (this->document()->characterAt(this->textCursor().position()) == '\"') {
                    this->textCursor().deleteChar();
                }
                break;
            case '<':
                QPlainTextEdit::keyPressEvent(event);
                // 如果后面的字符是匹配的>，则同时删除
                if (this->document()->characterAt(this->textCursor().position()) == '>') {
                    this->textCursor().deleteChar();
                }
                break;
            default:
                QPlainTextEdit::keyPressEvent(event);
        }
    }
    // 其他按键：执行默认行为
    else {
        QPlainTextEdit::keyPressEvent(event);
        highlightMatchingParenthesis();


    }
}

// 初始化代码补全列表（添加C++关键字和常用代码片段）
void CodeEditor::setUpCompleteList()
{
    completeList << "char" << "class" << "const"
                << "double" << "enum" << "explicit"
                << "friend" << "inline" << "int"
                << "long" << "namespace" << "operator"
                << "private" << "protected" << "public"
                << "short" << "signals" << "signed"
                << "slots" << "static" << "struct"
                << "template" << "typedef" << "typename"
                << "union" << "unsigned" << "virtual"
                << "void" << "volatile" << "bool" << "using" << "constexpr"
                << "sizeof" << "if" << "for" << "foreach" << "while" << "do" << "case"
                << "break" << "continue" << "template" << "delete" << "new"
                << "default" << "try" << "return" << "throw" << "catch" << "goto" << "else"
                << "extren" << "this" << "switch" << "#include <>" << "#include \"\"" << "#define" << "iostream";
}

// 获取光标前的单词（用于代码补全）
QString CodeEditor::getWordOfCursor()
{
    int pos = this->textCursor().position() - 1;  // 当前光标位置前一个字符
    QVector<QChar> words;  // 存储组成单词的字符
    QString result;  // 最终结果

    // 检查光标后一个字符是否为数字、字母或空格
    QChar ch = this->document()->characterAt(pos + 1);
    if (ch.isDigit() || ch.isLetter() || ch == ' ') return result;

    // 获取光标前的字符
    ch = this->document()->characterAt(pos);
    if (ch == ' ') return result;

    // 向前查找，直到遇到非单词字符（字母、数字、下划线、#）
    while (ch.isDigit() || ch.isLetter() || ch == '_' || ch == '#') {
        words.append(ch);
        pos--;
        ch = this->document()->characterAt(pos);
    }

    // 反转字符顺序（因为是从后向前收集的）
    for (int i = words.size() - 1; i >= 0; i--)
        result += words[i];

    return result;
}

// 显示代码补全窗口
void CodeEditor::showCompleteWidget()
{
    // 如果处于忽略状态（例如正在处理补全插入），则返回
    if (completeState == CompleteState::Ignore) return;

    // 隐藏补全窗口并设置状态为隐藏
    completeWidget->hide();
    completeState = CompleteState::Hide;

    // 获取光标前的单词
    QString word = this->getWordOfCursor();
    completeWidget->clear();  // 清空补全列表

    if (!word.isEmpty()) {  // 如果有单词需要补全
        int maxSize = 0;  // 记录最长补全项的长度
        QMap<QString, int> distance;  // 存储补全项与当前单词的编辑距离
        vector<QString> itemList;  // 存储匹配的补全项

        // 查找所有包含当前单词的补全项
        foreach (const QString &temp, completeList) {
            if (temp.contains(word)) {
                itemList.push_back(temp);
                // 计算莱文斯坦距离（编辑距离）
                distance[temp] = CompleteListWidget::ldistance(temp.toStdString(), word.toStdString());
                if (temp.length() > maxSize) maxSize = temp.length();  // 更新最大长度
            }
        }

        // 如果有匹配的补全项
        if (itemList.size() > 0) {
            // 按编辑距离排序（距离越小越靠前）
            sort(itemList.begin(), itemList.end(), [&](const QString &s1, const QString &s2)
                { return distance[s1] < distance[s2]; });

            // 添加到补全窗口
            foreach (const QString &item, itemList) {
                completeWidget->addItem(new QListWidgetItem(item));
            }

            // 计算补全窗口的X坐标
            int x = this->getCompleteWidgetX();
            // 计算补全窗口的Y坐标（光标下方）
            int y = this->cursorRect().y() + fontMetrics().height();

            // 移动补全窗口到指定位置
            completeWidget->move(x, y);

            // 设置补全窗口大小
            if (completeWidget->count() > 5)
                completeWidget->setFixedHeight(fontMetrics().height() * 6);
            else
                completeWidget->setFixedHeight(fontMetrics().height() * (completeWidget->count() + 1));

            // 设置宽度（考虑最长补全项的长度）
            completeWidget->setFixedWidth((fontMetrics().width(QLatin1Char('9')) + 6) * maxSize);

            // 显示补全窗口并设置状态为显示
            completeWidget->show();
            completeState = CompleteState::Showing;

            // 选中第一个补全项
            completeWidget->setCurrentRow(0, QItemSelectionModel::Select);
        }
    }
}

// 计算补全窗口的X坐标（即单词开始位置的X坐标）
int CodeEditor::getCompleteWidgetX()
{
    QTextCursor cursor = this->textCursor();
    int pos = cursor.position() - 1;  // 当前光标位置前一个字符
    int origianlPos = pos + 1;  // 原始位置

    QChar ch;
    // 向前查找，直到找到单词的开始位置
    ch = this->document()->characterAt(pos);
    while ((ch.isDigit() || ch.isLetter() || ch == '_' || ch == '#') && pos > 0) {
        pos--;
        ch = this->document()->characterAt(pos);
    }
    pos++;  // 调整到单词的第一个字符

    // 临时忽略光标变化事件，避免触发新的补全
    completeState = CompleteState::Ignore;

    // 将光标移动到单词开始位置
    cursor.setPosition(pos);
    this->setTextCursor(cursor);

    // 获取该位置的X坐标并加上一些边距
    int x = this->cursorRect().x() + 2 * fontMetrics().width(QLatin1Char('9'));

    // 恢复原始光标位置
    cursor.setPosition(origianlPos);
    this->setTextCursor(cursor);

    // 恢复状态为隐藏
    completeState = CompleteState::Hide;

    return x;
}

// 高亮匹配括号
void CodeEditor::highlightMatchingParenthesis()
{
    QTextCursor cursor = this->textCursor();
    int position = cursor.position();
    QChar ch = document()->characterAt(position);

    if (ch == '(' || ch == '{' || ch == '[') {
        // 根据当前字符类型确定匹配括号的类型
        char closingChar = (ch == '(') ? ')' : (ch == '{' ? '}' : ']');
        QTextCursor matchingCursor = findMatchingBracket(position, closingChar);

        if (matchingCursor.isNull()) {
            return;
        }

        // 创建高亮显示的选项
        QList<QTextEdit::ExtraSelection> extraSelections;
        QTextEdit::ExtraSelection selection;

        // 高亮匹配的括号
        selection.cursor = matchingCursor;
        selection.format.setBackground(QColor(0, 255, 0, 50)); // 绿色背景
        extraSelections.append(selection);

        // 高亮当前括号
        selection.cursor = cursor;
        selection.format.setBackground(QColor(255, 0, 0, 50)); // 红色背景
        extraSelections.append(selection);

        // 设置额外的选择以显示高亮
        setExtraSelections(extraSelections);
    }
}

// 查找匹配的括号
QTextCursor CodeEditor::findMatchingBracket(int position, char closingChar)
{
    QTextCursor cursor = this->textCursor();
    int bracketCount = 0;
    bool insideString = false;

    // 向前或向后查找匹配的括号
    while (!cursor.isNull()) {
        cursor.movePosition(QTextCursor::NextCharacter);
        QChar currentChar = cursor.document()->characterAt(cursor.position());

        // 如果在字符串内，跳过括号匹配
        if (currentChar == '"') {
            insideString = !insideString;
        }

        if (!insideString) {
            if (currentChar == closingChar) {
                return cursor;  // 找到匹配的括号
            }
        }
    }

    return QTextCursor();  // 如果未找到匹配的括号
}
