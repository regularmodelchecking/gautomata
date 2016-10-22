/*
 * refnfta.hh: 
 *
 * Copyright (C) 2002 Marcus Nilsson (marcus@docs.uu.se)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Authors:
 *    Marcus Nilsson (marcus@docs.uu.se)
 *    Julien d'Orso  (juldor@it.uu.se)
 */

#ifndef TREEAUTOMATA_REFNFTA_HH
#define TREEAUTOMATA_REFNFTA_HH

#include <gautomata/nfta/nfta.h>

namespace gautomata
{
  using gbdd::Space;
  using gbdd::Domain;

  class RefNfta : public Nfta
  {
       Nfta* ptr_nfta;

    static const Nfta& follow_if_refnfta(const Nfta& a);
  public:
    class Factory : public Nfta::Factory
    {
      Nfta::Factory* ptr_factory;
    public:
      Factory(Nfta::Factory* factory);
      ~Factory();
      
      Nfta* ptr_empty(unsigned int max_arity) const;
      Nfta* ptr_universal(unsigned int max_arity) const;
      
      Nfta* ptr_symbol(unsigned int max_arity, SymbolSet sym) const;    
      Nfta* ptr_symbol_with_children(unsigned int max_arity,
					      SymbolSet sym,
					      unsigned int arity,
					      vector<const Nfta*> children) const;
      
     }; 

  RefNfta(Nfta* nfta);
    RefNfta(const RefNfta& ref_nfta);
    ~RefNfta();

    RefNfta& operator= (const RefNfta& a);

    Nfta::Factory* ptr_factory() const;
    Nfta* ptr_clone() const;

    Nfta* ptr_negate(const Nfta &a) const;
    //    static RefNfta negate(const RefNfta& a);

	
    unsigned int max_arity() const;	
    SymbolSet alphabet() const;
    StateSet states() const;
    Relation transitions(unsigned int arity) const;
    StateSet states_accepting() const;

    Nfta* ptr_product(const Nfta& a2,
			       bool (*fn)(bool v1, bool v2)) const;

    Nfta* ptr_concatenation(const Nfta& a2, SymbolSet sym) const;

		

    // Unary operations
	
    
    Nfta* ptr_complete() const;
    Nfta* ptr_determinize() const;
    Nfta* ptr_minimize() const;
    Nfta* ptr_project(Domain vs) const;
    Nfta* ptr_rename(VarMap map) const;
    Nfta* ptr_rename(Domain vs1, Domain vs2) const;

    Nfta* ptr_kleene(SymbolSet sym) const;
    

    Nfta* ptr_remove_leaves(SymbolSet leaves) const;
    

    //  auto_ptr<Nfta> ptr_reverse() const;
    //  auto_ptr<Nfta> ptr_negate() const;


    Nfta* ptr_filter_states(StateSet s) const;
    Nfta* ptr_filter_states_live() const;
    Nfta* ptr_filter_states_reachable() const;
    Nfta* ptr_filter_states_productive() const;

    Nfta* ptr_with_accepting(StateSet accepting) const;


    // Explicit Construction
    
    State add_state(bool accepting);
    void add_edge(unsigned int arity, vector<StateSet> from, SymbolSet on, StateSet to);
    void add_edge(unsigned int arity, vector<State> from, SymbolSet on, State to);
    void add_transitions(unsigned int arity, Relation new_transitions);

    bool is_true() const;
    bool is_false() const;

    // Value semantics versions of operations
    
    friend RefNfta operator&(const RefNfta &a1, const RefNfta &a2);
    friend RefNfta operator|(const RefNfta &a1, const RefNfta &a2);
    friend RefNfta operator-(const RefNfta &a1, const RefNfta &a2);
    
    // friend RefNfta operator*(const RefNfta& a1, const RefNfta& a2);
    
    RefNfta determinize() const;
    RefNfta minimize() const;
    RefNfta project(Domain vs) const;
    RefNfta rename(VarMap map) const;
    RefNfta rename(Domain vs1, Domain vs2) const;

    RefNfta kleene(SymbolSet sym) const;


    RefNfta remove_leaves(SymbolSet leaves) const;
    

//      RefNfta reverse() const;
//      RefNfta negate() const;

//      RefNfta operator!() const { return negate(); }
    
    RefNfta with_accepting(StateSet accepting) const;


    RefNfta filter_states(StateSet s) const;
    RefNfta filter_states_live() const;
    RefNfta filter_states_reachable() const;
    RefNfta filter_states_productive() const;
    
    };
}

#endif /* TREEAUTOMATA_REFNFTA_HH */
