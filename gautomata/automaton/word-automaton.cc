/*
 * word-automaton.cc: 
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

#include "word-automaton.h"
#include <iostream>

namespace gautomata
{

using namespace gbdd;       

WordAutomaton::~WordAutomaton()
{}

SymbolSet WordAutomaton::alphabet() const
{
	return SymbolSet(Domain::infinite(), Bdd(get_space(), true));
}

Space* WordAutomaton::get_space() const
{
	return states().get_space();
}

SymbolSet WordAutomaton::edge_between(State q, State r) const
{
	return edge_between(StateSet(states(), q),
			    StateSet(states(), r));
}

SymbolSet WordAutomaton::edge_between(StateSet q, StateSet r) const
{
	return transitions().restrict(0, q).restrict(2, r).project_on(1);
}

Relation WordAutomaton::transitions() const
{
	StateSet Q = states();

	unsigned int states_vars = Q.get_domain().size();

	// Make sure we have room for states (note that alphabet can be infinite)
	Domain dom_alphabet = alphabet().get_domain() + 2 * states_vars;
	Domain dom_from(0, states_vars);
	Domain dom_to = dom_from + states_vars;

	Domains tr_doms = dom_from * dom_alphabet * dom_to;

	Relation tr(tr_doms, Bdd(Q.get_space(), false));

	for (StateSet::const_iterator i = Q.begin();i != Q.end();++i)
	{
		for (StateSet::const_iterator j = Q.begin();j != Q.end();++j)
		{
			tr |= Relation(tr_doms,
				       (Bdd::value(tr.get_space(), dom_from, *i) &
					Bdd::value(tr.get_space(), dom_to, *j) &
					BddSet(dom_alphabet, edge_between(*i, *j)).get_bdd()));
		}
	}

	return tr;
}

StateSet WordAutomaton::successors(StateSet q, SymbolSet on) const
{
	return transitions().restrict(0, q).restrict(1, on).project_on(2);
}

StateSet WordAutomaton::predecessors(StateSet q, SymbolSet on) const
{
	return transitions().restrict(2, q).restrict(1, on).project_on(0);
}

StateSet WordAutomaton::reachable_successors(StateSet q, SymbolSet on) const
{
	StateSet q_prev(q);
	do
	{
		q_prev = q;
		q = q | successors(q, on);
	} while(!(q_prev == q));

	return q;
}

StateSet WordAutomaton::reachable_predecessors(StateSet q, SymbolSet on) const
{
	StateSet q_prev(q);
	do
	{
		q_prev = q;
		q = q | predecessors(q, on);
	} while(!(q_prev == q));

	return q;
}

unsigned int WordAutomaton::n_states() const
{
	return states().size();
}

/// Adds an edge in the automaton
/**
 * For all states q in \a from, all symbols a in \a on, and for all states q' in
 * \a to, adds an edge (q, a, q') to the automaton.
 *
 * @param from Set of states where the edge should originate
 * @param on Set on symbol on the new edge
 * @param to Set of states where the edge should be destined
 * 
 */

void WordAutomaton::add_edge(StateSet from, SymbolSet on, StateSet to)
{
	for (StateSet::const_iterator i = from.begin();i != from.end();++i)
	{
		for (StateSet::const_iterator j = to.begin();j != to.end();++j)
		{
			add_edge(*i, on, *j);
		}
	}
}

/// Adds an edge in the automaton
/**
 * For all symbols a in \a on, adds an edge (from, a, to)
 * to the automaton.
 *
 * @param from State where the edge should originate
 * @param on Set on symbol on the new edge
 * @param to State where the edge should be destined
 * 
 */

void WordAutomaton::add_edge(State from, SymbolSet on, State to)
{
	add_edge(StateSet(states(), from),
		 on,
		 StateSet(states(), to));
}

/// Adds transitions to the automaton
/**
 * 
 *
 * @param new_transitions Transitions to add
 */

void WordAutomaton::add_transitions(Relation new_transitions)
{
	StateSet Q = new_transitions.project_on(0);

	for (StateSet::const_iterator i = Q.begin();i != Q.end();++i)
	{
		Relation Q_transitions = new_transitions.restrict(0, StateSet(Q, *i));
		StateSet R = Q_transitions.project_on(2);

		for (StateSet::const_iterator j = R.begin();j != R.end();++j)
		{
			Relation QR_transitions = Q_transitions.restrict(2, StateSet(R, *j));
			SymbolSet edge = QR_transitions.project_on(1);

			add_edge(*i, edge, *j);
		}
	}
}

ReverseWordAutomaton::ReverseWordAutomaton(const WordAutomaton& a):
	a(a)
{}

StateSet ReverseWordAutomaton::states() const
{
	return a.states();
}

SymbolSet ReverseWordAutomaton::alphabet() const
{
	return a.alphabet();
}

Space* ReverseWordAutomaton::get_space() const
{
	return a.get_space();
}

SymbolSet ReverseWordAutomaton::edge_between(StateSet q, StateSet r) const
{
	return a.edge_between(r, q);
}

SymbolSet ReverseWordAutomaton::edge_between(State q, State r) const
{
	return a.edge_between(r, q);
}

Relation ReverseWordAutomaton::transitions() const
{
	assert(false);
}

StateSet ReverseWordAutomaton::successors(StateSet q, SymbolSet on) const
{
	return a.predecessors(q, on);
}

StateSet ReverseWordAutomaton::predecessors(StateSet q, SymbolSet on) const
{
	return a.successors(q, on);
}


StateSet ReverseWordAutomaton::reachable_successors(StateSet q,
			      SymbolSet on) const
{
	return a.reachable_predecessors(q, on);
}

StateSet ReverseWordAutomaton::reachable_predecessors(StateSet q,
				SymbolSet on) const
{
	return a.reachable_successors(q, on);
}

unsigned int ReverseWordAutomaton::n_states() const
{
	return a.n_states();
}

void ReverseWordAutomaton::add_edge(StateSet from, SymbolSet on, StateSet to)
{
	assert(false);
}

void ReverseWordAutomaton::add_edge(State from, SymbolSet on, State to)
{
	assert(false);
}

void ReverseWordAutomaton::add_transitions(Relation new_transitions)
{
	assert(false);
}

WordAutomaton::Factory* ReverseWordAutomaton::ptr_factory() const
{
	return a.ptr_factory();
}


}
