/*
 * buchi-automaton.cc: 
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

#include "buchi-automaton.h"
#include <algorithm>


namespace gautomata
{

using namespace gbdd;       

BuchiAutomaton* BuchiAutomaton::Factory::ptr_universal() const
{
	BuchiAutomaton* res = ptr_empty();

	State q = res->add_state(true, vector<bool>(1, true));
	res->add_edge(q, res->alphabet(), q);

	return res;
}


struct is_member : unary_function<Set, bool>
{
	unsigned int v;
public:
	is_member(unsigned int v): v(v) {}
	
	bool operator()(Set s) { return s.member(v); }
};


hash_map<State,State> BuchiAutomaton::copy_states(const BuchiAutomaton& a,
						  StateSet states_to_copy, 
						  StateSet starting,
						  vector<StateSet> accepting)
{
	StateSet Q = states_to_copy;
	StateSet Init = starting;
	hash_map<State,State> state_map;

	for (StateSet::const_iterator i = Q.begin();i != Q.end();++i)
	{
		vector<bool> new_accepting(accepting.size());
		transform(accepting.begin(), accepting.end(), new_accepting.begin(), is_member(*i));

		state_map[*i] = add_state(Init.member(*i), new_accepting);
	}

	return state_map;
}
	
hash_map<State,State> BuchiAutomaton::copy_states_and_transitions(const BuchiAutomaton& a,
								  StateSet states_to_copy,
								  StateSet starting,
								  vector<StateSet> accepting)
{
	hash_map<State,State> state_map = copy_states(a, states_to_copy, starting, accepting);
	StateSet Q = states_to_copy;

	for (StateSet::const_iterator i = Q.begin();i != Q.end();++i)
	{
		for (StateSet::const_iterator j = Q.begin();j != Q.end();++j)
		{
			add_edge(state_map[*i], 
				 a.edge_between(*i, *j),
				 state_map[*j]);
		}
	}

	return state_map;
}

BuchiAutomaton::~BuchiAutomaton()
{}

StateSet BuchiAutomaton::states_reachable() const
{
	return reachable_successors(states_starting(), alphabet());
}

BuchiAutomaton* BuchiAutomaton::ptr_clone() const
{
	BuchiAutomaton* res = auto_ptr<Factory>(ptr_factory())->ptr_empty();

	res->copy_states_and_transitions(*this, states(), states_starting(), states_accepting());

	return res;
}


}

