/*
 * refnfa.h: 
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
#ifndef GAUTOMATA_REFNFA_H
#define GAUTOMATA_REFNFA_H

#include <gautomata/nfa/nfa.h>

namespace gautomata
{
	using gbdd::Space;
	using gbdd::Domain;

	class RefNfa : public Nfa
	{
		auto_ptr<Nfa> ptr_nfa;

		static const Nfa& follow_if_refnfa(const Nfa& a);
	public:
		class Factory : public Nfa::Factory
		{
			auto_ptr<Nfa::Factory> ptr_factory;
		public:
			Factory(Nfa::Factory* factory);
			~Factory();

			Nfa* ptr_empty() const;
			Nfa* ptr_universal() const;
			
			Nfa* ptr_epsilon() const;
			Nfa* ptr_symbol(SymbolSet sym) const;

		};

		RefNfa(Nfa* nfa);
		RefNfa(const Nfa& a);
		RefNfa(const RefNfa& a);
		~RefNfa();

		RefNfa& operator= (const RefNfa& a);

		Nfa::Factory* ptr_factory() const;
		Nfa* ptr_clone() const;
		
		SymbolSet alphabet() const;
		StateSet states() const;
		StateSet states_starting() const;
		Relation transitions() const;
		StateSet states_accepting() const;

		SymbolSet edge_between(StateSet q, StateSet r) const;
		SymbolSet edge_between(State q, State r) const;


		StateSet successors(StateSet q, SymbolSet on) const;
		StateSet predecessors(StateSet q, SymbolSet on) const;


		StateSet reachable_successors(StateSet q,
						      SymbolSet on) const;
		StateSet reachable_predecessors(StateSet q,
							SymbolSet on) const;


		unsigned int n_states() const;


		Nfa* ptr_product(const Nfa& a2,
						  bool (*fn)(bool v1, bool v2)) const;

		Nfa* ptr_concatenation(const Nfa& a2) const;

		

		// Unary operations
		
		Nfa* ptr_deterministic() const;
		Nfa* ptr_minimize() const;
		Nfa* ptr_project(Domain vs) const;
		Nfa* ptr_rename(VarMap map) const;
		Nfa* ptr_rename(Domain vs1, Domain vs2) const;

		Nfa* ptr_kleene() const;
		Nfa* ptr_reverse() const;
		Nfa* ptr_negate() const;

		Nfa* ptr_with_starting_accepting(StateSet starting,
					     StateSet accepting) const;


		Nfa* ptr_filter_states(StateSet s) const;
		Nfa* ptr_filter_states_live() const;
		Nfa* ptr_filter_states_reachable() const;
		Nfa* ptr_filter_states_productive() const;

		// Explicit Construction

		State add_state(bool accepting, bool starting = false);
		void add_edge(StateSet from, SymbolSet on, StateSet to);
		void add_edge(State from, SymbolSet on, State to);
		void add_transitions(Relation new_transitions);

		bool is_true() const;
		bool is_false() const;

		// Value semantics versions of operations

		friend RefNfa operator&(const RefNfa &a1, const RefNfa &a2);
		friend RefNfa operator|(const RefNfa &a1, const RefNfa &a2);
		friend RefNfa operator-(const RefNfa &a1, const RefNfa &a2);

		friend RefNfa operator*(const RefNfa& a1, const RefNfa& a2);

		RefNfa deterministic() const;
		RefNfa minimize() const;
		RefNfa project(Domain vs) const;
		RefNfa rename(VarMap map) const;
		RefNfa rename(Domain vs1, Domain vs2) const;

		RefNfa kleene() const;
		RefNfa reverse() const;
		RefNfa negate() const;

		RefNfa operator!() const { return negate(); }

		RefNfa with_starting_accepting(StateSet starting,
					     StateSet accepting) const;


		RefNfa filter_states(StateSet s) const;
		RefNfa filter_states_live() const;
		RefNfa filter_states_reachable() const;
		RefNfa filter_states_productive() const;

	};
}

#endif /* GAUTOMATA_REFNFA_H */
