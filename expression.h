#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QString>
#include <memory>
#include <vector>

class Expression {
public:
    // Базовый класс для узлов выражения
    class Node {
    public:
        virtual ~Node() = default;
        virtual double evaluate(const std::vector<double>& values) const = 0;
    };
    std::unique_ptr<Expression> clone() const {
        return std::make_unique<Expression>(
            getExpression(),
            getUnits(),
            getFormat(),
            getMin(),
            getMax(),
            getStep()
            );
    }

    Expression() = default;
    Expression(const QString& expression, const QString& units,
               const QString& format, double min, double max, double step);

    double evaluate(const std::vector<double>& values) const;
    QString getExpression() const { return expression; }
    QString getUnits() const { return units; }
    QString getFormat() const { return format; }
    double getMin() const { return min; }
    double getMax() const { return max; }
    double getStep() const { return step; }

private:
    QString expression;
    QString units;
    QString format;
    double min = 0;
    double max = 0;
    double step = 0;
    std::unique_ptr<Node> root;

    // Вспомогательные функции для парсинга выражений
    std::unique_ptr<Node> parseExpression(const QString& expr);
    std::unique_ptr<Node> parseTerm(const QString& expr, int& pos);
    std::unique_ptr<Node> parseFactor(const QString& expr, int& pos);
};

// Конкретные классы узлов
class NumberNode : public Expression::Node {
public:
    NumberNode(double value) : value(value) {}
    double evaluate(const std::vector<double>& values) const override { return value; }
private:
    double value;
};

class VariableNode : public Expression::Node {
public:
    VariableNode(int index) : index(index) {}
    double evaluate(const std::vector<double>& values) const override { return values[index]; }
private:
    int index;
};

class BinaryOpNode : public Expression::Node {
public:
    BinaryOpNode(char op, std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}
    double evaluate(const std::vector<double>& values) const override {
        switch (op) {
        case '+': return left->evaluate(values) + right->evaluate(values);
        case '-': return left->evaluate(values) - right->evaluate(values);
        case '*': return left->evaluate(values) * right->evaluate(values);
        case '/': return left->evaluate(values) / right->evaluate(values);
        default: return 0;
        }
    }
private:
    char op;
    std::unique_ptr<Node> left, right;
};

#endif // EXPRESSION_H
