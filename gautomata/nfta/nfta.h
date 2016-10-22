 /*
 * nfta.hh:
 *     A generic interface for (bottom-up)
non-deterministic finite tree automata.
 *
 * Authors:
 *    Julien d'Orso (julien.dorso@it.uu.se)
 *
 */

#ifndef TREE_AUTOMATA_NFTA_H
#define TREE_AUTOMATA_NFTA_H

#include <gbdd/gbdd.h>
#include <gautomata/automaton/automaton.h>
#include <ostream>
#include <memory>

namespace gautomata
{
  typedef gbdd::Space::VarMap VarMap;
  using gbdd::BoolConstraint;

  class Nfta : public Automaton
  {
    hash_map<State, State> copy_states(const Nfta& a,StateSet accepting);
    hash_map<State, State> copy_states_and_transitions(const Nfta& a, StateSet accepting);

  public:

    static vector< vector<State> > combine_states(vector<StateSet> V);
    static vector<StateSet> combinations(unsigned int a, StateSet s);

    class Factory : public StructureConstraint::Factory {
    public:
      virtual ~Factory() {}

      virtual Nfta* ptr_empty(unsigned int max_arity) const =0;
      virtual Nfta* ptr_universal(unsigned int max_arity) const =0;
      

      virtual Nfta* ptr_symbol(unsigned int max_arity, SymbolSet sym) const =0;
      virtual Nfta* ptr_symbol_with_children(unsigned int max_arity,
						      SymbolSet sym,
						      unsigned int arity,
						      vector<const Nfta*> children) const =0;
      virtual Nfta* ptr_forall(const BoolConstraint& c) const;

    
    
    
    
    
      virtual Nfta* ptr_random(unsigned int max_arity,
					unsigned int n_states,
					unsigned int n_vars,
					float edge_density =0.5,
					float symbol_density =0.5) const;

    };

    virtual Factory* ptr_factory() const =0;

    virtual ~Nfta() {}

    virtual Nfta* ptr_clone() const =0;
    BddBased* ptr_negate() const;
    virtual Nfta* ptr_negate(const Nfta &a) const =0;
    virtual unsigned int max_arity() const =0;
    virtual SymbolSet alphabet() const =0;
    virtual StateSet states() const =0;
    virtual StateSet states_accepting() const =0;
    virtual Space* get_space() const;

    // In the file Nfta.cc, there is a default implementation for
    // these three functions. However, this definition is circular:
    // edge() -> edge(Set) -> trans -> edge()
    // Thus, an implementation should redefine only one of these three.

    virtual SymbolSet edge_between(unsigned int arity, vector<StateSet> q, StateSet r) const;
    virtual SymbolSet edge_between(unsigned int arity, vector<State> q, State r) const;

    virtual Relation transitions(unsigned int arity) const;

    // Analysis of the automaton.

    // Default implementation of these two uses the transitions() method.

    virtual StateSet successors(unsigned int arity, vector<StateSet> q, SymbolSet on) const;

    virtual StateSet predecessors(unsigned int arity, StateSet q, SymbolSet on) const;

    // Default implementations of the following functions uses the two methods above.
    virtual StateSet reachable_successors(StateSet q, SymbolSet on) const;
    virtual StateSet reachable_predecessors(StateSet q, SymbolSet on) const;
    virtual StateSet states_reachable() const;
    virtual StateSet states_productive() const;
    virtual StateSet states_live() const;

    // Construction methods

    virtual State add_state(bool accepting) =0;

    // Definition for these two is circular. An implementation
    // needs to define one of these.
    virtual void add_edge(unsigned int arity, vector<State> from, SymbolSet on, State to);
    virtual void add_edge(unsigned int arity, vector<StateSet> from, SymbolSet on, StateSet to);

    virtual void add_transitions(unsigned int arity, Relation new_transitions);


    // Binary operations....
    Nfta* ptr_product(const gbdd::BddBased& a2, bool (*fn)(bool v1, bool v2)) const;
    virtual Nfta* ptr_product(const Nfta& a2, bool (*fn)(bool v1, bool v2)) const =0;





    virtual Nfta* ptr_product_and(const Nfta& a2)const;
    virtual Nfta* ptr_product_or(const Nfta& a2)const;
    virtual Nfta* ptr_product_minus(const Nfta& a2)const;

    virtual Nfta* ptr_concatenation(const Nfta& a2,SymbolSet sym) const =0;

    // Unary operations
    virtual Var highest_var() const;
    virtual Var lowest_var() const;
    virtual Nfta* ptr_constrain_value(Var v, bool value) const;
    virtual Nfta* ptr_complete() const =0;
    virtual Nfta* ptr_determinize() const =0;
    virtual Nfta* ptr_minimize() const =0;
    virtual Nfta* ptr_project(Domain vs) const;
    virtual Nfta* ptr_rename(VarMap map) const;
    virtual Nfta* ptr_rename(Domain vs1, Domain vs2) const;

    virtual Nfta* ptr_filter_states(StateSet s) const=0;
    virtual Nfta* ptr_filter_states_live() const;
    virtual Nfta* ptr_filter_states_reachable() const;
    virtual Nfta* ptr_filter_states_productive()const;

    virtual Nfta* ptr_with_accepting(StateSet accepting) const =0;

    virtual Nfta* ptr_kleene(SymbolSet sym) const =0;


    virtual Nfta* ptr_remove_leaves(SymbolSet leaves) const =0;


    // Predicates

    virtual bool is_true() const;
    virtual bool is_false() const;

    bool operator == (const BddBased &a2) const;
    bool operator != (const BddBased &a2) const;
    bool operator >= (const BddBased &a2) const;
    bool operator >  (const BddBased &a2) const;
    bool operator <= (const BddBased &a2) const;
    bool operator <  (const BddBased &a2) const;

    virtual bool operator == (const Nfta& a2) const;    
    virtual bool operator != (const Nfta& a2) const;   
    virtual bool operator >= (const Nfta& a2) const;   
    virtual bool operator >  (const Nfta& a2) const;   
    virtual bool operator <= (const Nfta& a2) const;   
    virtual bool operator <  (const Nfta& a2) const;

    // Printing an automaton to a stream:

    friend ostream& operator << (ostream& s, const Nfta& nfta);
  };

}


#endif /* TREE_AUTOMATA_NFTA_H */
