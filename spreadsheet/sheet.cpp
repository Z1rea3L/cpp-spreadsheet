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
    return GetCellPtr(pos);
}

const Cell* Sheet::GetCellPtr(Position pos) const {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid position");
    }

    const auto cell = cells_.find(pos);
    if (cell == cells_.end()) {
        return nullptr;
    }

    return cell->second.get();
}

Cell* Sheet::GetCellPtr(Position pos) {
    return const_cast<Cell*>(static_cast<const Sheet&>(*this).GetCellPtr(pos));
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetCellPtr(pos);
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
            PrintCellValue(output, {row, col});
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
            PrintCellText(output, {row, col});
        }
        
        output << '\n';
    }
}

void Sheet::PrintCellValue(std::ostream& output, Position pos)const{
    const auto& cell = cells_.find(pos);
    if (cell != cells_.end() && cell->second != nullptr && !cell->second->GetText().empty()) {
        std::visit([&](const auto value) { output << value; }, cell->second->GetValue());
    }
}

void Sheet::PrintCellText(std::ostream& output, Position pos)const{
    const auto& cell = cells_.find(pos);
    if (cell != cells_.end() && cell->second != nullptr && !cell->second->GetText().empty()) {
        output << cell->second->GetText();
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
