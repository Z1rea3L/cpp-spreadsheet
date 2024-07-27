#pragma once
#include "FormulaAST.h"
#include "common.h"

#include <memory>
#include <vector>
class FormulaInterface {
public:
    using Value = std::variant<double, FormulaError>;

    virtual ~FormulaInterface() = default;

    virtual Value Evaluate(const SheetInterface& sheet) const = 0;
    
    virtual std::string GetExpression() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

class Formula : public FormulaInterface {
public:
     explicit Formula(std::string expression) try : ast_(ParseFormulaAST(std::move(expression))) {}
       catch (...) {
            throw FormulaException("Incorrect formula syntax");
        }

    Value Evaluate(const SheetInterface& sheet) const override;
    std::vector<Position> GetReferencedCells() const override;
    virtual std::string GetExpression() const override;

private:
    const FormulaAST ast_; 
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression);
