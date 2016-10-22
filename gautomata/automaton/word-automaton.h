/*
 * word-automaton.h: 
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
 */

#ifndef WORD_AUTOMATON_H
#define WORD_AUTOMATON_H

#include <gautomata/automaton/automaton.h>

namespace gautomata
{
	class WordAutomaton : public Automaton
	{
	public:
		virtual ~WordAutomaton();

		// These methods are mandatory for every implentation
		// of WordAutomaton. They are not necessary efficient but are
		// conceptually simple, derived directly from the
		// definition of nondeterminisic automata, and used as
		// a common interface for conversion between different
		// formats.

		virtual StateSet states() const = 0;
		virtual SymbolSet alphabet() const;
		virtual Space* get_space() const;

		// At least one of these must be defined

		virtual SymbolSet edge_between(StateSet q, StateSet r) const;
		virtual SymbolSet edge_between(State q, State r) const;
		virtual Relation transitions() const;


		// Analysis of automaton. successors and predecessors have default
		// implementation using the transitions() method. The other methods
		// have default implementation using successors and predecessors.

		virtual StateSet successors(StateSet q, SymbolSet on) const;
		virtual StateSet predecessors(StateSet q, SymbolSet on) const;


		virtual StateSet reachable_successors(StateSet q,
						      SymbolSet on) const;
		virtual StateSet reachable_predecessors(StateSet q,
							SymbolSet on) const;


		virtual unsigned int n_states() const;

		// One of add_edges must be defined, the default implementation is
		// defined in terms of eachother.
		
		virtual void add_edge(StateSet from, SymbolSet on, StateSet to);
		virtual void add_edge(State from, SymbolSet on, State to);
		
		virtual void add_transitions(Relation new_transitions);

	};

	class ReverseWordAutomaton : public WordAutomaton
	{
		const WordAutomaton& a;
	public:
		ReverseWordAutomaton(const WordAutomaton& a);
		
		StateSet states() const;
		SymbolSet alphabet() const;
		Space* get_space() const;
		
		SymbolSet edge_between(StateSet q, StateSet r) const;
		SymbolSet edge_between(State q, State r) const;
		Relation transitions() const;
		
		
		StateSet successors(StateSet q, SymbolSet on) const;
		StateSet predecessors(StateSet q, SymbolSet on) const;
		
		
		StateSet reachable_successors(StateSet q,
					      SymbolSet on) const;
		StateSet reachable_predecessors(StateSet q,
						SymbolSet on) const;
		
		
		unsigned int n_states() const;
		
		void add_edge(StateSet from, SymbolSet on, StateSet to);
		void add_edge(State from, SymbolSet on, State to);
		
		void add_transitions(Relation new_transitions);

		virtual BddBased* ptr_rename(VarMap map) const { return a.ptr_rename(map); }
		virtual BddBased* ptr_project(gbdd::Domain vs) const  { return a.ptr_project(vs); };
		virtual BddBased* ptr_constrain_value(Var v, bool value) const  { return a.ptr_constrain_value(v, value); };
		
		virtual BddBased* ptr_product(const BddBased& b2, bool (*fn)(bool v1, bool v2)) const { return a.ptr_product(b2, fn); } ;
		virtual BddBased* ptr_negate() const { return a.ptr_negate(); } ;

		virtual BddBased* ptr_clone() const { return a.ptr_clone(); } ;

		virtual Var highest_var() const { return a.highest_var(); } ;
		virtual Var lowest_var() const { return a.lowest_var(); } ;

		virtual bool operator==(const BddBased& b2) const  { return a == b2; };

		Factory* ptr_factory() const;
	};
}


#endif /* WORD_AUTOMATON_H */
