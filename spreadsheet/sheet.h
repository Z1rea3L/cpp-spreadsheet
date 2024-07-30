#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>
#include <memory>
#include <iostream>

struct CellHasher {
    size_t operator()(const Position pos) const {
        return std::hash<std::string>()(pos.ToString());
    }
};

class Sheet : public SheetInterface {
public:
    ~Sheet();
    
    void SetCell(Position pos, std::string text) override;
    
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    
    const Cell* GetCellPtr(Position pos) const;
    Cell* GetCellPtr(Position pos);
    
    void ClearCell(Position pos) override;
    Size GetPrintableSize() const override;
    
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    int sheet_rows_ = 0;
    int sheet_cols_ = 0;
    void UpdateSheetSize();
    void PrintCellValue(std::ostream& output, Position pos)const;
    void PrintCellText(std::ostream& output, Position pos)const;
    
    std::unordered_map<Position, std::unique_ptr<Cell>, CellHasher> cells_;
};
