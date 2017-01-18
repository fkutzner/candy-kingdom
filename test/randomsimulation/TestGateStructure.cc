#include "TestGateStructure.h"

#include <unordered_set>
#include <cassert>

#include <gtest/gtest.h>

namespace randsim {
    void deleteClauses(Glucose::CNFProblem* formula);
    void assertContainsVariable(const std::unordered_set<Glucose::Var>& variables, const Glucose::Var forbidden);
    void assertDoesNotContainVariable(const std::unordered_set<Glucose::Var>& variables, const Glucose::Var forbidden);
    Glucose::Cl negatedLits(const Glucose::Cl& clause);
    void insertVariables(const std::vector<Glucose::Lit>& lits, std::unordered_set<Glucose::Var>& target);
    
    GateStructureBuilder::GateStructureBuilder() {
        
    }
    
    GateStructureBuilder::~GateStructureBuilder() {
        
    }
    
    class GateStructureBuilderImpl : public GateStructureBuilder {
    public:
        GateStructureBuilderImpl& withAnd(const std::vector<Glucose::Lit>& inputs, Glucose::Lit output) override;
        GateStructureBuilderImpl& withOr(const std::vector<Glucose::Lit>& inputs, Glucose::Lit output) override;
        GateStructureBuilderImpl& withXor(const std::vector<Glucose::Lit>& inputs, Glucose::Lit output) override;
        std::unique_ptr<Glucose::CNFProblem, void(*)(Glucose::CNFProblem*)> build() override;
        
        GateStructureBuilderImpl();
        virtual ~GateStructureBuilderImpl();
        GateStructureBuilderImpl(const GateStructureBuilderImpl& other) = delete;
        GateStructureBuilderImpl& operator=(const GateStructureBuilderImpl& other) = delete;
        
    private:
        std::unique_ptr<Glucose::CNFProblem> m_builtClauses;
        std::unordered_set<Glucose::Var> m_usedOutputs;
        std::unordered_set<Glucose::Var> m_gateVars;
        
        void addClause(Glucose::Cl it);
    };
    
    void GateStructureBuilderImpl::addClause(Glucose::Cl it) {
        m_builtClauses->readClause(it);
    }
    
    GateStructureBuilderImpl::GateStructureBuilderImpl() : GateStructureBuilder(), m_builtClauses(new Glucose::CNFProblem()),
    m_usedOutputs({}), m_gateVars({}) {
        addClause({Glucose::mkLit(0,1)});
        m_gateVars.insert(0);
    }
    
    GateStructureBuilderImpl::~GateStructureBuilderImpl() {
        
    }
    
    GateStructureBuilderImpl& GateStructureBuilderImpl::withAnd(const std::vector<Glucose::Lit>& inputs, Glucose::Lit output) {
        assertContainsVariable(m_gateVars, Glucose::var(output));
        assertDoesNotContainVariable(m_usedOutputs, Glucose::var(output));
        
        Glucose::Cl fwd = negatedLits(inputs);
        fwd.push_back(output);
        addClause(fwd);
        
        for (auto input : inputs) {
            addClause({input, ~output});
        }
        
        insertVariables(inputs, m_gateVars);
        m_usedOutputs.insert(Glucose::var(output));
        return *this;
    }
    
    GateStructureBuilderImpl& GateStructureBuilderImpl::withOr(const std::vector<Glucose::Lit>& inputs, Glucose::Lit output) {
        assertContainsVariable(m_gateVars, Glucose::var(output));
        assertDoesNotContainVariable(m_usedOutputs, Glucose::var(output));
        
        Glucose::Cl bwd(inputs);
        bwd.push_back(~output);
        addClause(bwd);
        
        for (auto input : inputs) {
            addClause({~input, output});
        }
        
        insertVariables(inputs, m_gateVars);
        m_usedOutputs.insert(Glucose::var(output));
        return *this;
    }
    
    GateStructureBuilderImpl& GateStructureBuilderImpl::withXor(const std::vector<Glucose::Lit>& inputs, Glucose::Lit output) {
        // TODO: implement this
        assert(false);
        return *this;
    }
    
    std::unique_ptr<Glucose::CNFProblem, void(*)(Glucose::CNFProblem*)> GateStructureBuilderImpl::build() {
        m_usedOutputs.clear();
        m_gateVars.clear();
        m_usedOutputs.clear();
        return std::unique_ptr<Glucose::CNFProblem, void(*)(Glucose::CNFProblem*)>(m_builtClauses.release(), deleteClauses);
    }
    
    std::unique_ptr<GateStructureBuilder> createGateStructureBuilder() {
        return std::make_unique<GateStructureBuilderImpl>();
    }
    
    void assertContainsVariable(const std::unordered_set<Glucose::Var>& variables, Glucose::Var forbidden) {
        assert(variables.find(forbidden) != variables.end()); // TODO refactor
    }
    
    void assertDoesNotContainVariable(const std::unordered_set<Glucose::Var>& variables, Glucose::Var forbidden) {
        assert(variables.find(forbidden) == variables.end()); // TODO refactor
    }
    
    void deleteClauses(Glucose::CNFProblem* formula) {
        
        // TODO: This is a weird workaround. CNFProblem ought to own the formula and take care of its destruction.
        
        if (formula == nullptr) {
            return;
        }
        
        for (auto clause : formula->getProblem()) {
            delete clause;
        }
        formula->getProblem().clear();
    }
    
    Glucose::Cl negatedLits(const Glucose::Cl& clause) {
        Glucose::Cl result;
        for (auto lit : clause) {
            result.push_back(~lit);
        }
        return result;
    }
    
    void insertVariables(const std::vector<Glucose::Lit>& lits, std::unordered_set<Glucose::Var>& target) {
        for (auto lit : lits) {
            target.insert(Glucose::var(lit));
        }
    }
    
}
