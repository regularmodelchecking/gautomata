/*
 * nfa.h: 
 *
 * Copyright (C) 2000 Marcus Nilsson (marcusn@docs.uu.se)
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
 *    Marcus Nilsson (marcusn@docs.uu.se)
 */

#ifndef GAUTOMATA_NFA_H
#define GAUTOMATA_NFA_H

#include <gautomata/automaton/word-automaton.h>
#include <gautomata/automaton/simulation-graph.h>
#include <ostream>
#include <memory>
#include <string>

namespace gautomata
{
	using gbdd::Domain;
	using gbdd::BoolConstraint;

	typedef gbdd::Space::VarMap VarMap;

	class Nfa : public WordAutomaton
	{
	public:
		class Factory : public StructureConstraint::Factory
		{
		public:
			virtual ~Factory();

			virtual Nfa* ptr_empty() const = 0;
			virtual Nfa* ptr_universal() const;
			
			virtual Nfa* ptr_epsilon() const;
			virtual Nfa* ptr_symbol(SymbolSet sym) const;

			Nfa* ptr_random(unsigned int n_states,
						 unsigned int n_vars,
						 float edge_density = 0.5,
						 float symbol_density = 0.5) const;

			Nfa* ptr_clone(const Nfa& a) const;

			virtual StructureConstraint* ptr_forall(const BoolConstraint& c) const;

		};
		friend class Factory;
			
		virtual Factory* ptr_factory() const = 0;

		virtual ~Nfa();

		virtual Nfa* ptr_clone() const;

		// These methods are mandatory for every implentation
		// of Nfa. They are not necessary efficient but are
		// conceptually simple, derived directly from the
		// definition of nondeterminisic automata, and used as
		// a common interface for conversion between different
		// formats.

		virtual StateSet states_starting() const = 0;
		virtual StateSet states_accepting() const = 0;

		virtual StateSet states_reachable() const;
		virtual StateSet states_productive() const;
		virtual StateSet states_live() const;

		// Construction

		virtual State add_state(bool accepting, bool starting = false) = 0;
		
		hash_map<State,State> copy_states(const Nfa& a, StateSet states_to_copy, StateSet accepting, StateSet starting);
		hash_map<State,State> copy_states_and_transitions(const Nfa& a, StateSet states_to_copy, StateSet accepting, StateSet starting);

		// Binary Operations

		Nfa* ptr_product(const gbdd::BddBased& a2, bool (*fn)(bool v1, bool v2)) const;

		virtual Nfa* ptr_product(const Nfa& a2,
					 bool (*fn)(bool v1, bool v2)) const;

		virtual Nfa* ptr_product_and(const Nfa& a2) const;
		virtual Nfa* ptr_product_or(const Nfa& a2) const;
		virtual Nfa* ptr_product_minus(const Nfa& a2) const;
		virtual Nfa* ptr_concatenation(const Nfa& a2) const;

		

		// Unary operations
		
		virtual Var highest_var() const;
		virtual Var lowest_var() const;
		virtual Nfa* ptr_constrain_value(Var v, bool value) const;

		virtual Nfa* ptr_deterministic() const;
		virtual Nfa* ptr_minimize() const;
		virtual Nfa* ptr_project(Domain vs) const;
		virtual Nfa* ptr_rename(VarMap map) const;
		virtual Nfa* ptr_rename(Domain vs1, Domain vs2) const;

		virtual Nfa* ptr_kleene() const;
		virtual Nfa* ptr_reverse() const;
		virtual Nfa* ptr_negate() const;

		virtual Nfa* ptr_with_starting_accepting(StateSet starting,
								  StateSet accepting) const;


		virtual Nfa* ptr_filter_states(StateSet s) const;
		virtual Nfa* ptr_filter_states_live() const;
		virtual Nfa* ptr_filter_states_reachable() const;
		virtual Nfa* ptr_filter_states_productive() const;

		// Predicates

		virtual bool is_true() const;
		virtual bool is_false() const;

		virtual bool operator==(const BddBased &a2) const;
		virtual bool operator==(const Nfa &a2) const;
		virtual bool operator!=(const Nfa &a2) const;
		virtual bool operator<(const Nfa &a2) const;
		virtual bool operator<=(const Nfa &a2) const;
		virtual bool operator>(const Nfa &a2) const;
		virtual bool operator>=(const Nfa &a2) const;

		friend ostream& operator<<(ostream &s, const Nfa &nfa);

		class PrintStrategy
		{
		public:
			virtual string format_attributes() const;
			virtual string format_state(const Nfa& nfa, State q) const;
			virtual string format_edge(const Nfa& nfa, const SymbolSet& edge) const;
		};

		class ProxyPrintStrategy : public PrintStrategy
		{
			auto_ptr<PrintStrategy> next;
		public:
			ProxyPrintStrategy(PrintStrategy* next):
				next(next)
				{}

			virtual string format_attributes() const { return next->format_attributes(); }
			virtual string format_state(const Nfa& nfa, State q) const { return next->format_state(nfa, q); }
			virtual string format_edge(const Nfa& nfa, const SymbolSet& edge) const { return next->format_edge(nfa, edge); }
		};

		class EnumeratePrintStrategy : public PrintStrategy
		{
			gbdd::Domains ds;
			virtual string format_edge(const Nfa& nfa, const SymbolSet& edge) const;
		public:
			EnumeratePrintStrategy(gbdd::Domains ds);
			
			virtual string format_edge_symbol(const Nfa& nfa, unsigned int domain_i, unsigned int v) const;
			
		};

		ostream& print_dot(ostream& out, const PrintStrategy& ps) const { return print_dot(out, ps, states()); }
		ostream& print_dot(ostream& out, const PrintStrategy& ps, const StateSet& states) const;

		typedef pair<State,State> StatePair;

		set<StatePair> find_simulation_forward(const Nfa& a2, const StateSet& a1_states, const StateSet& a2_states) const;
		set<StatePair> find_simulation_backward(const Nfa& a2, const StateSet& a1_states, const StateSet& a2_states) const;

		set<StatePair> find_simulation_forward(const Nfa& a2) const;
		set<StatePair> find_simulation_backward(const Nfa& a2) const;
	};
}



#endif /* GAUTOMATA_NFA_H */











