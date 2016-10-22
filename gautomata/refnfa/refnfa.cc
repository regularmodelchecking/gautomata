/*
 * refnfa.cc: 
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

#include "refnfa.h"
#include <iostream>

namespace gautomata
{

RefNfa::Factory::Factory(Nfa::Factory* factory):
	ptr_factory(factory)
{}

RefNfa::Factory::~Factory()
{}

const Nfa& RefNfa::follow_if_refnfa(const Nfa& a)
{
	try
	{
		return *(dynamic_cast<const RefNfa&>(a)).ptr_nfa;
	}
	catch (...)
	{
		return a;
	}
}

Nfa* RefNfa::Factory::ptr_empty() const
{
	return ptr_factory->ptr_empty();
}

Nfa* RefNfa::Factory::ptr_universal() const
{
	return ptr_factory->ptr_universal();
}
			
Nfa* RefNfa::Factory::ptr_epsilon() const
{
	return ptr_factory->ptr_epsilon();
}

Nfa* RefNfa::Factory::ptr_symbol(SymbolSet sym) const
{
	return ptr_factory->ptr_symbol(sym);
}

RefNfa::RefNfa(Nfa* nfa):
	ptr_nfa(nfa)
{}

RefNfa::RefNfa(const Nfa& a):
	ptr_nfa(a.ptr_clone())
{}

RefNfa::RefNfa(const RefNfa& a):
	ptr_nfa(a.ptr_clone())
{}

RefNfa& RefNfa::operator= (const RefNfa& a)
{
	ptr_nfa.reset(a.ptr_clone());

	return *this;
}


RefNfa::~RefNfa()
{}

Nfa* RefNfa::ptr_clone() const
{
	return ptr_nfa->ptr_clone();
}

Nfa::Factory* RefNfa::ptr_factory() const
{
	return ptr_nfa->ptr_factory();
}

SymbolSet RefNfa::alphabet() const
{
	return ptr_nfa->alphabet();
}
StateSet RefNfa::states() const
{
	return ptr_nfa->states();
}

StateSet RefNfa::states_starting() const
{
	return ptr_nfa->states_starting();
}

Relation RefNfa::transitions() const
{
	return ptr_nfa->transitions();
}

StateSet RefNfa::states_accepting() const
{
	return ptr_nfa->states_accepting();
}

SymbolSet RefNfa::edge_between(StateSet q, StateSet r) const
{
	return ptr_nfa->edge_between(q, r);
}

SymbolSet RefNfa::edge_between(State q, State r) const
{
	return ptr_nfa->edge_between(q, r);
}

StateSet RefNfa::successors(StateSet q, SymbolSet on) const
{
   return ptr_nfa->successors(q, on);
}

StateSet RefNfa::predecessors(StateSet q, SymbolSet on) const
{
  return ptr_nfa->predecessors(q, on);
}

StateSet RefNfa::reachable_successors(StateSet q, SymbolSet on) const
{
  return ptr_nfa->reachable_successors(q, on);
}

StateSet RefNfa::reachable_predecessors(StateSet q, SymbolSet on) const
{
  return ptr_nfa->reachable_predecessors(q, on);
}

unsigned int RefNfa::n_states() const
{
  return ptr_nfa->n_states();
}


Nfa* RefNfa::ptr_product(const Nfa& a2,
			  bool (*fn)(bool v1, bool v2)) const
{
	return ptr_nfa->ptr_product(follow_if_refnfa(a2), fn);
}

Nfa* RefNfa::ptr_concatenation(const Nfa& a2) const
{
	return ptr_nfa->ptr_concatenation(follow_if_refnfa(a2));
}

// Unary operations

Nfa* RefNfa::ptr_deterministic() const
{
	return ptr_nfa->ptr_deterministic();
}

Nfa* RefNfa::ptr_minimize() const
{
 	return ptr_nfa->ptr_minimize();
}
Nfa* RefNfa::ptr_project(Domain vs) const
{
	return ptr_nfa->ptr_project(vs);
}
Nfa* RefNfa::ptr_rename(VarMap map) const
{
	return ptr_nfa->ptr_rename(map);
}
Nfa* RefNfa::ptr_rename(Domain vs1, Domain vs2) const
{
	return ptr_nfa->ptr_rename(vs1,vs2);
}

Nfa* RefNfa::ptr_kleene() const
{
	return ptr_nfa->ptr_kleene();
}
Nfa* RefNfa::ptr_reverse() const
{
	return ptr_nfa->ptr_reverse();
}
Nfa* RefNfa::ptr_negate() const
{
	return ptr_nfa->ptr_negate();
}

Nfa* RefNfa::ptr_with_starting_accepting(StateSet starting,
					  StateSet accepting) const
{
	return ptr_nfa->ptr_with_starting_accepting(starting, accepting);
}


Nfa* RefNfa::ptr_filter_states(StateSet s) const
{
	return ptr_nfa->ptr_filter_states(s);
}
Nfa* RefNfa::ptr_filter_states_live() const
{
	return ptr_nfa->ptr_filter_states_live();
}
Nfa* RefNfa::ptr_filter_states_reachable() const
{
	return ptr_nfa->ptr_filter_states_reachable();
}
Nfa* RefNfa::ptr_filter_states_productive() const
{
	return ptr_nfa->ptr_filter_states_productive();
}

// Explicit Construction

State RefNfa::add_state(bool accepting, bool starting)
{
	return ptr_nfa->add_state(accepting, starting);
}

void RefNfa::add_edge(StateSet from, SymbolSet on, StateSet to)
{
	return ptr_nfa->add_edge(from, on, to);
}

void RefNfa::add_edge(State from, SymbolSet on, State to)
{
	return ptr_nfa->add_edge(from, on, to);
}

void RefNfa::add_transitions(Relation new_transitions)
{
	return ptr_nfa->add_transitions(new_transitions);
}

bool RefNfa::is_true() const
{
	return ptr_nfa->is_true();
}

bool RefNfa::is_false() const
{
	return ptr_nfa->is_false();
}

// Value semantics versions of operations

RefNfa operator&(const RefNfa &a1, const RefNfa &a2)
{
	return RefNfa(a1.ptr_nfa->ptr_product_and(*a2.ptr_nfa));
}

RefNfa operator|(const RefNfa &a1, const RefNfa &a2)
{
	return RefNfa(a1.ptr_nfa->ptr_product_or(*a2.ptr_nfa));
}

RefNfa operator-(const RefNfa &a1, const RefNfa &a2)
{
	return RefNfa(a1.ptr_nfa->ptr_product_minus(*a2.ptr_nfa));
}

RefNfa operator*(const RefNfa& a1, const RefNfa& a2)
{
	return RefNfa(a1.ptr_nfa->ptr_concatenation(*a2.ptr_nfa));
}

RefNfa RefNfa::deterministic() const
{
	return RefNfa(ptr_deterministic());
}

RefNfa RefNfa::minimize() const
{
	return RefNfa(ptr_minimize());
}

RefNfa RefNfa::project(Domain vs) const
{
	return RefNfa(ptr_project(vs));
}

RefNfa RefNfa::rename(VarMap map) const
{
	return RefNfa(ptr_rename(map));
}

RefNfa RefNfa::rename(Domain vs1, Domain vs2) const
{
	return RefNfa(ptr_rename(vs1, vs2));
}


RefNfa RefNfa::kleene() const
{
	return RefNfa(ptr_kleene());
}

RefNfa RefNfa::reverse() const
{
	return RefNfa(ptr_reverse());
}

RefNfa RefNfa::negate() const
{
	return RefNfa(ptr_negate());
}


RefNfa RefNfa::with_starting_accepting(StateSet starting,
				       StateSet accepting) const
{
	return RefNfa(ptr_with_starting_accepting(starting, accepting));
}



RefNfa RefNfa::filter_states(StateSet s) const
{
	return RefNfa(ptr_filter_states(s));
}

RefNfa RefNfa::filter_states_live() const
{
	return RefNfa(ptr_filter_states_live());
}

RefNfa RefNfa::filter_states_reachable() const
{
	return RefNfa(ptr_filter_states_reachable());
}

RefNfa RefNfa::filter_states_productive() const
{
	return RefNfa(ptr_filter_states_productive());
}


}
