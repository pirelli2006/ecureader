#include "expression.h"
#include <cctype>
#include <stdexcept>

Expression::Expression(const QString& expression, const QString& units,
                       const QString& format, double min, double max, double step)
    : expression(expression), units(units), format(format), min(min), max(max), step(step)
{
    root = parseExpression(expression);
}

double Expression::evaluate(const std::vector<double>& values) const
{
    if (!root) return 0;
    double result = root->evaluate(values);
    return qBound(min, result, max);
}

std::unique_ptr<Expression::Node> Expression::parseExpression(const QString& expr)
{
    int pos = 0;
    auto result = parseTerm(expr, pos);
    while (pos < expr.length()) {
        char op = expr[pos].toLatin1();
        if (op != '+' && op != '-') break;
        pos++;
        auto right = parseTerm(expr, pos);
        BinaryOpNode::Operator binOp = (op == '+') ?
                                           BinaryOpNode::Operator::Add : BinaryOpNode::Operator::Subtract;
        result = std::make_unique<BinaryOpNode>(binOp, std::move(result), std::move(right));
    }
    return result;
}

std::unique_ptr<Expression::Node> Expression::parseTerm(const QString& expr, int& pos)
{
    auto result = parseFactor(expr, pos);
    while (pos < expr.length()) {
        char op = expr[pos].toLatin1();
        if (op != '*' && op != '/') break;
        pos++;
        auto right = parseFactor(expr, pos);
        BinaryOpNode::Operator binOp = (op == '*') ?
                                           BinaryOpNode::Operator::Multiply : BinaryOpNode::Operator::Divide;
        result = std::make_unique<BinaryOpNode>(binOp, std::move(result), std::move(right));
    }
    return result;
}

std::unique_ptr<Expression::Node> Expression::parseFactor(const QString& expr, int& pos)
{
    if (pos >= expr.length()) throw std::runtime_error("Unexpected end of expression");

    // Пропускаем пробелы
    while (pos < expr.length() && expr[pos].isSpace()) pos++;

    if (expr[pos] == '(') {
        pos++;
        auto result = parseExpression(expr.mid(pos));
        if (pos >= expr.length() || expr[pos] != ')')
            throw std::runtime_error("Mismatched parentheses");
        pos++;
        return result;
    }

    if (std::isalpha(expr[pos].toLatin1())) {
        int index = expr[pos].toLatin1() - 'A';
        pos++;
        return std::make_unique<VariableNode>(index);
    }

    QString numStr;
    while (pos < expr.length() && (std::isdigit(expr[pos].toLatin1()) || expr[pos] == '.')) {
        numStr += expr[pos];
        pos++;
    }
    if (numStr.isEmpty()) throw std::runtime_error("Invalid expression");
    bool ok;
    double value = numStr.toDouble(&ok);
    if (!ok) throw std::runtime_error("Invalid number format");
    return std::make_unique<NumberNode>(value);
}
