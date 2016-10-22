/*
 * bnfta.hh:
 *     An implementation of NFTA's using BDD's to encode the transition relation...
 *
 * Authors:
 *    Julien d'Orso (julien.dorso@it.uu.se)
 *
 */

#ifndef TREE_AUTOMATA_BNFTA_H
#define TREE_AUTOMATA_BNFTA_H

#include <gautomata/nfta/nfta.h>


namespace gautomata
{
  using gbdd::Space;
  using gbdd::Domain;

  //  typedef gbdd::Space::VarMap VarMap;
  typedef vector<Relation> Relations;

  class Bnfta : public Nfta
  {
    Space *_space;

    unsigned int n_states;
    StateSet _states;

    StateSet _accepting;

    vector<Relation> _transitions;

    bool is_complete;
    bool is_deterministic;

    unsigned int _max_arity;
  
    Domain map_transitions_source(Domain vs, unsigned int a) const;
    Domain map_transitions_alphabet(Domain vs, unsigned int a) const;
    Domain map_transitions_dest(Domain vs, unsigned int a) const;

    gbdd::Domains get_transitions_domains(unsigned int arity) const;

    static Bnfta actual_product(Bnfta a1, Bnfta a2,bool (*fn)(bool v1, bool v2));

    void set_n_states(unsigned int new_n_states);
    void increase_to_n_states(unsigned int new_n_states, bool accepting);

    gbdd::BinaryRelation bisim() const;


    static vector<StateSet> find_powerstates(const Bnfta& old,
					     vector<vector<vector<StateSet> > >& tuples,
					     vector<vector<gbdd::Bdd> >& p2p_sym);

    static vector<Relation> powerstates_transitions(const Bnfta& old,
						    const Bnfta& res_auto,
						    const vector<StateSet>& powerstates,
						    const vector<vector<vector<StateSet> > >& tuples,
						    const vector<vector<gbdd::Bdd> >& p2p_sym);


  public: 

    unsigned int max_arity() const;

    SymbolSet alphabet() const;
    Relation transitions(unsigned int arity) const;
    StateSet states() const;
    StateSet states_accepting() const;

    Space* get_space() const;
    unsigned int get_n_states() const;

    
    class Factory : public Nfta::Factory
      {
	Space* _space;
    
      public:
	Factory(Space* space);
	~Factory() {}
    
	Nfta* ptr_empty(unsigned int max_arity) const;
	Nfta* ptr_universal(unsigned int max_arity) const;
 
	Nfta* ptr_symbol(unsigned int max_arity, SymbolSet sym) const;    
	Nfta* ptr_symbol_with_children(unsigned int max_arity,
						SymbolSet sym,
						unsigned int arity,
						vector<const Nfta*> children) const;

    };
    
    Nfta::Factory* ptr_factory() const;

    Bnfta(Space* space, unsigned int max_arity);
    Bnfta(Space* space, unsigned int max_arity, bool v);

    Bnfta(const Bnfta& bnfta);
    Bnfta(const Nfta& nfta);

    ~Bnfta() {}

    Nfta* ptr_clone() const;
    Nfta* ptr_negate(const Nfta &a) const;
    Nfta* ptr_product(const Nfta &a2, bool (*fn)(bool v1, bool v2)) const; 
    Bnfta* ptr_product(const Bnfta& a2, bool (*fn)(bool v1, bool v2)) const;

    Nfta* ptr_concatenation(const Nfta& a2, SymbolSet sym) const;


    // unary operations

    Nfta* ptr_complete() const;
    Nfta* ptr_determinize() const;
    Nfta* ptr_minimize() const;
    Nfta* ptr_project(Domain vs) const;
    Nfta* ptr_rename(VarMap map) const;
    Nfta* ptr_rename(Domain vs1, Domain vs2) const;

    Nfta* ptr_filter_states(StateSet s) const;
    Nfta* ptr_filter_states_live() const;
    Nfta* ptr_filter_states_reachable() const;
    Nfta* ptr_filter_states_productive() const;

    Nfta* ptr_with_accepting(StateSet accepting) const;

    Nfta* ptr_kleene(SymbolSet sym) const;

    Nfta* ptr_remove_leaves(SymbolSet leaves) const;
    
    
    // Explicit construction methods:

    State add_state(bool accepting);

    void add_edge(unsigned int arity, vector<State> from, SymbolSet on, State to);
    void add_edge(unsigned int arity, vector<StateSet> from, SymbolSet on, StateSet to);

    void add_transitions(unsigned int arity, Relation new_transitions);
    
    static Bnfta symbol(Space* space, unsigned int max_arity, SymbolSet sym);

    static Bnfta symbol_with_children(Space* space, unsigned int max_arity,
				      SymbolSet sym,
				      unsigned int arity,
				      vector<const Nfta*> children);


    // simplifying an automaton

    Bnfta filter_states(StateSet s) const;

    Bnfta filter_states_reachable() const;
    Bnfta filter_states_productive() const;
    Bnfta filter_states_live() const;

    // Unary operations:

    Bnfta complete() const;
    Bnfta minimize() const;
    Bnfta determinize() const;
    Bnfta project(Domain vs) const;
    Bnfta rename(VarMap map) const;
    Bnfta rename(Domain vs1, Domain vs2) const;

    Bnfta with_accepting(StateSet accepting) const;

    Bnfta kleene(SymbolSet sym) const;

    Bnfta remove_leaves(SymbolSet leaves) const;
    
    // Boolean operations:

    
    static Bnfta product(const Bnfta& a1, const Bnfta& a2, bool (*fn)(bool v1, bool v2));


    static Bnfta concatenation(const Bnfta& a1, const Bnfta& a2, SymbolSet sym);
    
    static Bnfta negate(const Bnfta& a);


  
    friend Bnfta operator & (const Bnfta& a1, const Bnfta& a2);
    friend Bnfta operator | (const Bnfta& a1, const Bnfta& a2);
    friend Bnfta operator - (const Bnfta& a1, const Bnfta& a2);


    // Predicates:

    bool is_true() const; // does accept all ground terms ?
    bool is_false() const; // is language empty ?

    friend bool operator == (const Bnfta& a1, const Bnfta& a2);
    friend bool operator != (const Bnfta& a1, const Bnfta& a2);
    friend bool operator <= (const Bnfta& a1, const Bnfta& a2);
    friend bool operator < (const Bnfta& a1, const Bnfta& a2);
    friend bool operator >= (const Bnfta& a1, const Bnfta& a2);
    friend bool operator > (const Bnfta& a1, const Bnfta& a2);

  };

}


#endif /* TREE_AUTOMATA_BNFTA_H */
