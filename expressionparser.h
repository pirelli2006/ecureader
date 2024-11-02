#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include "expression.h"
#include <memory>

class ExpressionParser {
public:
    static std::unique_ptr<Expression::Node> parse(const QString& expression);

private:
    class ValueNode : public Expression::Node {
    public:
        ValueNode(double value) : value(value) {}
        double evaluate(double) const override { return value; }
    private:
        double value;
    };

    class VariableNode : public Expression::Node {
    public:
        double evaluate(double x) const override { return x; }
    };

    class OperatorNode : public Expression::Node {
    public:
        OperatorNode(char op, std::unique_ptr<Expression::Node> left,
                     std::unique_ptr<Expression::Node> right)
            : op(op), left(std::move(left)), right(std::move(right)) {}
        double evaluate(double x) const override;
    private:
        char op;
        std::unique_ptr<Expression::Node> left, right;
    };

    static int getPrecedence(char op);
    static bool isOperator(char c);
};

#endif // EXPRESSIONPARSER_H
