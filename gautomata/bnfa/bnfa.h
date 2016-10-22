/*
 * bnfa.h: 
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
#ifndef GAUTOMATA_BNFA_H
#define GAUTOMATA_BNFA_H

#include <gautomata/nfa/nfa.h>

namespace gautomata
{
	using gbdd::Space;
	using gbdd::Domain;

	typedef gbdd::Space::VarMap VarMap;

	class BNfa : public Nfa
	{
		typedef gbdd::Bdd::Var Var;
		static const Var base_source = 64;
		static const Var base_dest = 128;
		static const Var base_alphabet = 192;
		static const Var base_extra = 256;

		Space *_space;

		unsigned int n_states;
		StateSet _states;

		StateSet _starting;
		StateSet _accepting;

		Relation _transitions;

		bool is_necessarily_complete_deterministic;

		Domain map_transitions_source(Domain vs) const;
		Domain map_transitions_alphabet(Domain vs) const;
		Domain map_transitions_dest(Domain vs) const;

		gbdd::Domains get_transitions_domains() const;


		static BNfa exist_paths(BNfa a1, BNfa a2, 
					bool (*fn)(bool v1, bool v2));

		void set_n_states(unsigned int new_n_states);

		void increase_to_n_states(unsigned int new_n_states,
					  bool starting,
					  bool accepting);

		static void automata_increase_to_n_states(vector<BNfa>& automata, unsigned int n_states);

		static vector<StateSet> automata_states_starting(vector<BNfa> automata);
		static vector<StateSet> automata_states_accepting(vector<BNfa> automata);
		static vector<Relation> automata_transitions(vector<BNfa> automata);

		gbdd::BinaryRelation bisim() const;
		vector<StateSet> find_powerstates(Domain dom_powerstate, 
						  gbdd::Bdd::Var dom_powerstate_begin,
						  vector<gbdd::Bdd>& powerstate_to_sym_powerstate) const;
		static Relation powerstates_transitions(BNfa orig,
							BNfa res,
							gbdd::Bdd::Var dom_powerstate_begin,
							vector<StateSet> powerstates, 
							vector<gbdd::Bdd> powerstate_to_sym_powerstate);
		static vector<SymbolSet> powerstate_successor_syms(BNfa old, StateSet from);
	public:
		class Factory : public Nfa::Factory
		{
			Space* _space;
		public:
			Factory(Space* space);
			~Factory() {}

			BNfa* ptr_empty() const;
			BNfa* ptr_universal() const;
			
			BNfa* ptr_epsilon() const;
			BNfa* ptr_symbol(SymbolSet sym) const;

		};

		Factory* ptr_factory() const;

		BNfa(Space* space);
		BNfa(Space* space, bool v);

		BNfa(const BNfa& bnfa);
		BNfa(const Nfa& nfa);

		~BNfa();
	      		
		BNfa* ptr_clone() const;

		SymbolSet alphabet() const;
		StateSet states() const;
		StateSet states_starting() const;
		Relation transitions() const;
		StateSet states_accepting() const;

		Space* get_space() const;
		unsigned int get_n_states() const;

		BNfa* ptr_product(const Nfa& a2,
				  bool (*fn)(bool v1, bool v2)) const;

		BNfa* ptr_concatenation(const Nfa& a2) const;

		

		// Unary operations
		
		BNfa* ptr_deterministic() const;
		BNfa* ptr_minimize() const;
		BNfa* ptr_project(Domain vs) const;
		BNfa* ptr_rename(VarMap map) const;
		BNfa* ptr_rename(Domain vs1, Domain vs2) const;

		BNfa* ptr_kleene() const;
		BNfa* ptr_reverse() const;
		BNfa* ptr_negate() const;

		BNfa* ptr_with_starting_accepting(StateSet starting,
						  StateSet accepting) const;


		BNfa* ptr_filter_states(StateSet s) const;
		BNfa* ptr_filter_states_live() const;
		BNfa* ptr_filter_states_reachable() const;
		BNfa* ptr_filter_states_productive() const;

		// Explicit Construction

		State add_state(bool accepting, bool starting = false);
		void add_edge(StateSet from, SymbolSet on, StateSet to);
		void add_edge(State from, SymbolSet on, State to);
		void add_transitions(Relation new_transitions);

		virtual SymbolSet edge_between(StateSet q, StateSet r) const;
		virtual SymbolSet edge_between(State q, State r) const;

		static BNfa epsilon(Space* space);
		static BNfa symbol(Space* space, SymbolSet sym);

		// Binary Operations

		static BNfa product(BNfa a1, BNfa a2, 
				    bool (*fn)(bool v1, bool v2));

		friend BNfa operator&(const BNfa &a1, const BNfa &a2);
		friend BNfa operator|(const BNfa &a1, const BNfa &a2);
		friend BNfa operator-(const BNfa &a1, const BNfa &a2);

		friend BNfa operator*(const BNfa& a1, const BNfa& a2);

		// Unary operations
		
		BNfa deterministic() const;
		BNfa minimize() const;
		BNfa project(Domain vs) const;
		BNfa rename(VarMap map) const;
		BNfa rename(Domain vs1, Domain vs2) const;

		BNfa kleene() const;
		BNfa reverse() const;
		BNfa negate() const;

		BNfa operator!() { return negate(); }

		static BNfa color(vector<BNfa> automata, vector<gbdd::Relation>& out_color_maps);

		BNfa with_starting_accepting(StateSet starting,
					     StateSet accepting) const;


		BNfa filter_states(StateSet s) const;
		BNfa filter_states_live() const;
		BNfa filter_states_reachable() const;
		BNfa filter_states_productive() const;

		// Predicates

		bool is_true() const;
		bool is_false() const;

		friend bool operator==(const BNfa &a1, const BNfa &a2);
		friend bool operator!=(const BNfa &a1, const BNfa &a2);
		friend bool operator<(const BNfa &a1, const BNfa &a2);
		friend bool operator<=(const BNfa &a1, const BNfa &a2);
		friend bool operator>(const BNfa &a1, const BNfa &a2);
		friend bool operator>=(const BNfa &a1, const BNfa &a2);
	};
}		


#endif /* GAUTOMATA_BNFA_H */
