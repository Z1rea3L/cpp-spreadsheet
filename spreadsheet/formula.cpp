#include "formula.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <variant>

using namespace std::literals;

//FormulaError =================
FormulaError::FormulaError(Category category) 
    : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

  std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Ref:
            return "#REF!";
        case Category::Value:
            return "#VALUE!";
        case Category::Arithmetic:
            return "#ARITHM!";
        }
    return "";
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

//Formula =================
FormulaInterface::Value Formula::Evaluate(const SheetInterface& sheet) const{ //idk (sad emoji)
    const auto cell_check = [&sheet](Position pos)->double{
        if (!pos.IsValid()){
            throw FormulaError(FormulaError::Category::Ref);
        }

        const auto* cell = sheet.GetCell(pos);
        if (!cell){
            return 0;
        }

        if (std::holds_alternative<double>(cell->GetValue())){
            return std::get<double>(cell->GetValue());
        }

        if (std::holds_alternative<std::string>(cell->GetValue())) {
            auto value = std::get<std::string>(cell->GetValue());
            double result = 0;
            if (!value.empty()) {
                std::istringstream in(value);
                
                if (!(in >> result) || !in.eof()){
                    throw FormulaError(FormulaError::Category::Value);
                }
            }
            return result;
        }
        throw FormulaError(std::get<FormulaError>(cell->GetValue()));
    };

    try {
        return ast_.Execute(cell_check);
    }
    catch (FormulaError& e) {
        return FormulaError(e.GetCategory());
    }
}

std::vector<Position> Formula::GetReferencedCells() const{
    std::vector<Position> cells;
    for (auto cell : ast_.GetCells()) {
        if (cell.IsValid()){
            cells.push_back(cell);
        }
    }
    
    cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());
    return cells;
}

std::string Formula::GetExpression() const{
    std::ostringstream stream;
    ast_.PrintFormula(stream);
    return stream.str();
}

//Other ================
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
   try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("");
    }
}
