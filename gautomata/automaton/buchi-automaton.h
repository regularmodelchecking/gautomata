/*
 * buchi-automaton.h: 
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

#ifndef BUCHI_AUTOMATON_H
#define BUCHI_AUTOMATON_H

#include <gautomata/automaton/word-automaton.h>

namespace gautomata
{
	class BuchiAutomaton : public WordAutomaton
	{
		hash_map<State,State> copy_states(const BuchiAutomaton& a, StateSet states_to_copy, StateSet starting, vector<StateSet> accepting);
		hash_map<State,State> copy_states_and_transitions(const BuchiAutomaton& a, StateSet states_to_copy, StateSet starting, vector<StateSet> accepting);
	public:
		class Factory : public StructureConstraint::Factory
		{
		public:
			virtual ~Factory();

			virtual BuchiAutomaton* ptr_empty() const = 0;
			virtual BuchiAutomaton* ptr_universal() const;
		};
			
		virtual Factory* ptr_factory() const = 0;

		virtual ~BuchiAutomaton();

		virtual BuchiAutomaton* ptr_clone() const;

		virtual StateSet states_starting() const = 0;
		virtual vector<StateSet> states_accepting() const = 0;
		
		virtual StateSet states_reachable() const;
		
		virtual State add_state(bool starting, vector<bool> accepting = vector<bool>()) = 0;

	};
}

#endif /* BUCHI_AUTOMATON_H */
