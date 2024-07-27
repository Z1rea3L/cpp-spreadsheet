#include "sheet.h"
#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

Sheet::~Sheet(){
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    
    std::unique_ptr<Cell>& cell = cells_[pos];
    if (!cell) {
        cell = std::make_unique<Cell>(*this);
    }
    
    cell->Set(std::move(text));
    UpdateSheetSize();
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetConcreteCell(pos);
}

const Cell* Sheet::GetConcreteCell(Position pos) const {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid position");
    }

    const auto cell = cells_.find(pos);
    if (cell == cells_.end()) {
        return nullptr;
    }

    return cells_.at(pos).get();
}

Cell* Sheet::GetConcreteCell(Position pos) {
    return const_cast<Cell*>(static_cast<const Sheet&>(*this).GetConcreteCell(pos));
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetConcreteCell(pos);
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    const auto& cell = cells_.find(pos);
    if (cell != cells_.end() && cell->second != nullptr) {
        cell->second->Clear();
        if (!cell->second->IsReferenced()) {
            cell->second.reset();
        }
    }
    
    UpdateSheetSize();
}

Size Sheet::GetPrintableSize() const {
    Size size;
    for (const auto& [pos, cell] : cells_) {
        if (cell != nullptr) {
            size.rows = std::max(size.rows, pos.row + 1);
            size.cols = std::max(size.cols, pos.col + 1);
        }
    }
    
    return size;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < sheet_rows_; ++row) {
        for (int col = 0; col < sheet_cols_; ++col) {
            if (col > 0){
                output << '\t';
            }
            
            const auto& cell = cells_.find({ row, col });
            if (cell != cells_.end() && cell->second != nullptr && !cell->second->GetText().empty()) {
                std::visit([&](const auto value) { output << value; }, cell->second->GetValue());
            }
        }
        
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < sheet_rows_; ++row) {
        for (int col = 0; col < sheet_cols_; ++col) {
            if (col > 0){
                output << '\t';
            }
            
            const auto& cell = cells_.find({ row, col });
            if (cell != cells_.end() && cell->second != nullptr && !cell->second->GetText().empty()) {
                output << cell->second->GetText();
            }
        }
        
        output << '\n';
    }
}

void Sheet::UpdateSheetSize(){
    Size size = GetPrintableSize();
    sheet_rows_ = size.rows;
    sheet_cols_ = size.cols;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
