#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>
#include <variant>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();
    
    Value GetValue() const override;
    std::string GetText() const override;
    
    std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;

private:
    class Impl {
    public:
        virtual ~Impl() = default;
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const{
            return {};
        }
        virtual bool IsCacheValid() const {
            return true;
        }
        virtual void InvalidateCache(){
        }
    };

    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(){}
        TextImpl(std::string text);

        Value GetValue() const override;
        std::string GetText() const override;
        
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string expression, const SheetInterface& sheet);
        
        Value GetValue() const override;
        std::string GetText() const override;
        
        bool IsCacheValid() const override;
        void InvalidateCache() override;
        
        std::vector<Position> GetReferencedCells() const override;
        FormulaInterface::Value GetCachedValue() const;

    private:
        std::unique_ptr<FormulaInterface> formula_ptr_;
        const SheetInterface& sheet_;
        mutable std::optional<FormulaInterface::Value> cache_;
    };
    
    bool IsCyclicDepend(const Impl& impl) const;
    void UpdateCache();
    void CellDependesProcessing();

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    std::unordered_set<Cell*> cell_depend_from_;
    std::unordered_set<Cell*> cell_depend_to_;
};
