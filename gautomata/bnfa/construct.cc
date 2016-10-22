/*
 * construct.cc: 
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
 *    Marcus Nilsson (marcusn@docs.uu.se)
 */

#include "bnfa.h"

namespace gautomata
{

using namespace gbdd;

void BNfa::set_n_states(unsigned int new_n_states)
{
	unsigned int n_vars = Bdd::n_vars_needed(new_n_states);

	_states = Set(Domain(0, n_vars), Bdd(_space, true));

	{
		unsigned int i = new_n_states;

		for (;i < (1 << n_vars);++i)
		{
			_states &= !(Set(_states, i));
		}
	}

	n_states = new_n_states;

	is_necessarily_complete_deterministic = false;
}
	

/// Adds a new state to the automaton
/**
 * 
 *
 * @param accepting Whether the new state should be accepting
 * @param starting Whether the new state should be starting
 *
 * @return The new state added
 */

State BNfa::add_state(bool accepting, bool starting)
{
	unsigned int old_n_vars = Bdd::n_vars_needed(n_states);
	unsigned int new_n_vars = Bdd::n_vars_needed(n_states+1);

	State new_state = n_states;
	n_states++;

	if (new_n_vars > old_n_vars)
	{
		Domain new_domain = Domain(0, new_n_vars);
		Domains new_transitions_domains =
			get_transitions_domains();

		_starting = _starting.extend_domain(new_domain);
		_accepting = _accepting.extend_domain(new_domain);
		_states = _states.extend_domain(new_domain);

		_transitions = _transitions.extend_domain(0, new_transitions_domains[0]);
		_transitions = _transitions.extend_domain(2, new_transitions_domains[2]);
	}

	if (accepting) _accepting |= Set(_accepting, new_state);
	if (starting) _starting |= Set(_starting, new_state);

	_states |= Set(_states, new_state);

	is_necessarily_complete_deterministic = false;

	return new_state;
}

void BNfa::increase_to_n_states(unsigned int new_n_states,
			  bool starting,
			  bool accepting)
{
	unsigned int old_n_states = n_states;
	unsigned int old_n_vars = Bdd::n_vars_needed(old_n_states);
	unsigned int new_n_vars = Bdd::n_vars_needed(new_n_states);
	
	n_states = new_n_states;

	if (new_n_vars > old_n_vars)
	{
		Domain new_domain = Domain(0, new_n_vars);
		Domains new_transitions_domains =
			get_transitions_domains();

		_starting = _starting.extend_domain(new_domain);
		_accepting = _accepting.extend_domain(new_domain);
		_states = _states.extend_domain(new_domain);

		_transitions = _transitions.extend_domain(0, new_transitions_domains[0]);
		_transitions = _transitions.extend_domain(2, new_transitions_domains[2]);
	}

	Set new_states(_states, old_n_states, new_n_states - 1);

	_states |= new_states;
	if (accepting) _accepting |= new_states;
	if (starting) _starting |= new_states;

	is_necessarily_complete_deterministic = false;
}


void BNfa::add_edge(State from, SymbolSet on, State to)
{
	Domains doms = _transitions.get_domains();

	_transitions |= Relation(doms, Bdd::value(get_space(), doms[0], from) &
				       SymbolSet(doms[1], on).get_bdd() &
				       Bdd::value(get_space(), doms[2], to));

	is_necessarily_complete_deterministic = false;
}

void BNfa::add_edge(StateSet from, SymbolSet on, StateSet to)
{
	vector<BddBasedSet> tuple;
	tuple.push_back(from);
	tuple.push_back(on);
	tuple.push_back(to);

	_transitions |= BddBasedRelation::cross_product(_transitions.get_domains(), tuple);

	is_necessarily_complete_deterministic = false;
}

void BNfa::add_transitions(Relation new_transitions)
{
	_transitions |= new_transitions;

	is_necessarily_complete_deterministic = false;
}

}
