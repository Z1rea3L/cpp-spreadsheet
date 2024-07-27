#include "cell.h"
#include "sheet.h"

#include <iostream>

#include <deque>
#include <optional>

//Cell ===================
Cell::Cell(Sheet& sheet)
    :impl_(std::make_unique<EmptyImpl>()), sheet_(sheet){
}

Cell::~Cell(){
}

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> impl;
    
    if (text.empty()){
        impl = std::make_unique<EmptyImpl>();
        
    }else if (text[0] == FORMULA_SIGN && text.size() > 1){
        impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
        
    }else{
        impl = std::make_unique<TextImpl>(std::move(text));
    }
    
    if (IsCyclicDepend(*impl)){
        throw CircularDependencyException("Cyclic depend");
    }
    
    impl_ = std::move(impl);

    for (Cell* cell : cell_depend_to_) {
        cell->cell_depend_from_.erase(this);
    }

    cell_depend_to_.clear();
    
    for (const auto& pos : impl_->GetReferencedCells()) {
        Cell* cell = sheet_.GetConcreteCell(pos);
        if (!cell){
            sheet_.SetCell(pos, "");
            cell = sheet_.GetConcreteCell(pos);
        }
        
        cell_depend_to_.insert(cell);
        cell->cell_depend_from_.insert(this);
    }

    UpdateCache(true);
}
void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !cell_depend_from_.empty();
}

bool Cell::IsCyclicDepend(const Impl& impl) const {
    if (impl.GetReferencedCells().empty()){
        return false;
    }

    std::unordered_set<const Cell*> referenced_cells;
    for (const auto& pos : impl.GetReferencedCells()) {
        referenced_cells.insert(sheet_.GetConcreteCell(pos));
    }

    std::unordered_set<const Cell*> checked;
    std::deque<const Cell*> to_check;
    to_check.push_front(this);

    while (!to_check.empty()) {
        const Cell* current = to_check.front();
        
        to_check.pop_front();
        checked.insert(current);

        if (referenced_cells.find(current) != referenced_cells.end()){
            return true;
        }
        
        for (const Cell* cell : current->cell_depend_from_) {
            if (checked.find(cell) == checked.end()){
                to_check.push_front(cell);
            }
        }
    }

    return false;
}

void Cell::UpdateCache(bool flag) {
    if (impl_->IsCacheValid() || flag) {
        impl_->InvalidateCache();
        
        for (Cell* cell : cell_depend_from_) {
            cell->UpdateCache();
        }
    }
}

//EmptyImpl ===================
Cell::Value Cell::EmptyImpl::GetValue() const {
    return ""; 
}
std::string Cell::EmptyImpl::GetText() const {
    return "";
}

//TextImpl ===================
Cell::TextImpl::TextImpl(std::string text)
    :text_(std::move(text)){   
    if (text_.empty()) { 
        throw std::logic_error(""); 
    }
}

Cell::Value Cell::TextImpl::GetValue() const{
    if (text_.size() > 0 && text_[0] == '\'') {
        return text_.substr(1);
    }
    return text_;
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

//FormulaImpl ===================
Cell::FormulaImpl::FormulaImpl(std::string expression, const SheetInterface& sheet)
    :sheet_(sheet){
     if (expression.empty() || expression[0] != FORMULA_SIGN){
         throw std::logic_error("logic_error");
     }
    
     formula_ptr_ = ParseFormula(expression.substr(1));
 }

Cell::Value Cell::FormulaImpl::GetValue() const{
    if (!cache_){
        cache_ = formula_ptr_->Evaluate(sheet_);
    }
    
    auto value = formula_ptr_->Evaluate(sheet_);
    
    if (std::holds_alternative<double>(value)){
        return std::get<double>(value);
    }
    
    return std::get<FormulaError>(value);
}

std::string Cell::FormulaImpl::GetText() const{
    if (formula_ptr_) {
        return '=' + formula_ptr_->GetExpression();
    } else {
        return "";
    }
}

bool Cell::FormulaImpl::IsCacheValid() const {
    return cache_.has_value();
}

void Cell::FormulaImpl::InvalidateCache(){
    cache_.reset();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_ptr_->GetReferencedCells();
}

FormulaInterface::Value Cell::FormulaImpl::GetCachedValue() const {
    return cache_.value();
}
