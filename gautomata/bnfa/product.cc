/*
 * product.cc: 
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

#include "bnfa.h"
#include <iostream>
namespace gautomata
{

using namespace gbdd;       

static Domain first_vars(Domain vs)
{
	unsigned int n_vars = vs.size() / 2;
	Domain res;

	Domain::const_iterator i;
	unsigned int index = 0;
	for (i = vs.begin();n_vars > 0;++i)
	{
		res |= Domain(*i);

		n_vars--;
	}

	return res;
}

static Domain last_vars(Domain vs)
{
	unsigned int n_vars = vs.size() / 2;
	Domain res;

	Domain::const_iterator i;
	unsigned int index = 0;

	i = vs.begin();

	{
		int j;
		for(j = 0;j < n_vars;++j) ++i;
	}

	for (;n_vars > 0;++i)
	{
		res |= Domain(*i);

		n_vars--;
	}

	return res;
}


static Domain even_vars(Domain vs)
{
	Domain res;

	Domain::const_iterator i;
	unsigned int index = 0;
	for (i = vs.begin();i != vs.end();++i)
	{
		if ((index % 2) == 0) res |= Domain(*i);

		index++;
	}

	return res;
}

static Domain odd_vars(Domain vs)
{
	Domain res;

	Domain::const_iterator i;
	unsigned int index = 0;
	for (i = vs.begin();i != vs.end();++i)
	{
		if ((index % 2) == 1) res |= Domain(*i);

		index++;
	}

	return res;
}

		

BNfa BNfa::exist_paths(BNfa a1, BNfa a2, 
		       bool (*fn)(bool v1, bool v2))
{
	using gbdd::Bdd;
	using gbdd::Relation;
	typedef Bdd::Var Var;

	unsigned int a1_n_vars = Bdd::n_vars_needed(a1.n_states);
	unsigned int a2_n_vars = Bdd::n_vars_needed(a2.n_states);

	unsigned int max_n_vars = max(a1_n_vars, a2_n_vars);

	unsigned int even_n_states = 1 << max_n_vars;

	// Extend both automata to contain even_n_states;

	a1.increase_to_n_states(even_n_states, false, false);
	a2.increase_to_n_states(even_n_states, false, false);


	// Variables from state from a1 and a2 are interleaved to form
	// the variables in the product automaton. Thus, the states of
	// the product automaton are the cross product of the state
	// of a1 and a2

	BNfa product_a(a1._space);

	product_a.set_n_states(1 << (max_n_vars*2));

	Domain new_state_domain = Domain(0, 2 * max_n_vars);
	Domain old_state_domain = Domain(0, max_n_vars);

	product_a._starting = 
		Set(new_state_domain,
		    Bdd::bdd_product(
			    a1._starting.get_bdd(),
			    a2._starting.get_bdd().rename(old_state_domain, old_state_domain + max_n_vars),
			    Bdd::fn_and));

	product_a._accepting = 
		Set(new_state_domain,
		    Bdd::bdd_product(
			    a1._accepting.get_bdd(),
			    a2._accepting.get_bdd().rename(old_state_domain, old_state_domain + max_n_vars),
			    fn));

	Domains new_transitions_domains = product_a.get_transitions_domains();

	Domains new_domains_a1 = new_transitions_domains;
	new_domains_a1[0] = first_vars(new_domains_a1[0]);
	new_domains_a1[2] = first_vars(new_domains_a1[2]);

	Domains new_domains_a2 = new_transitions_domains;
	new_domains_a2[0] = last_vars(new_domains_a2[0]);
	new_domains_a2[2] = last_vars(new_domains_a2[2]);

	Bdd transitions_a1 = Relation(new_domains_a1, a1._transitions).get_bdd();
	Bdd transitions_a2 = Relation(new_domains_a2, a2._transitions).get_bdd();

	Bdd new_transitions = transitions_a1 & transitions_a2;

	product_a._transitions = Relation(new_transitions_domains,
					  new_transitions);

	product_a.is_necessarily_complete_deterministic = false;

	return product_a.filter_states_live();
}

static bool fn_is_monotonic_in(bool (*fn)(bool v1, bool v2),
			       bool v1, bool v2)
{
	return !fn(v1, v2) ||
		(fn(v1, true) &&
		 fn(true, v2) &&
		 fn(true, true));
}

static bool fn_is_monotonic(bool (*fn)(bool v1, bool v2))
{
	return 
		fn_is_monotonic_in(fn, false, false) &&
		fn_is_monotonic_in(fn, false, true) &&
		fn_is_monotonic_in(fn, true, false) &&
		fn_is_monotonic_in(fn, true, true);
}

BNfa BNfa::product(BNfa a1, BNfa a2,
		   bool (*fn)(bool v1, bool v2))
{

	if (fn_is_monotonic(fn))
	{
		// Make sure that there exist a path for all words in a1 and a2 by
		// adding nonproductive starting state and add transitions to
		// that state.

		{
			State q = a1.add_state(false, true);
			a1.add_edge(a1.states(), a1.alphabet(), Set(a1.states(), q));
		}

		{
			State q = a2.add_state(false, true);
			a2.add_edge(a2.states(), a2.alphabet(), Set(a2.states(), q));
		}
	}
	else
	{
		// Make deterministic

		a1 = a1.deterministic();
		a2 = a2.deterministic();
	}

	// Look for paths

	return exist_paths(a1, a2, fn);
}

BNfa operator&(const BNfa &a1, const BNfa &a2)
{
	return BNfa::product(a1, a2, Bdd::fn_and);
}

BNfa operator|(const BNfa &a1, const BNfa &a2)
{
	return BNfa::product(a1, a2, Bdd::fn_or);
}

BNfa operator-(const BNfa &a1, const BNfa &a2)
{
	return BNfa::product(a1, a2, Bdd::fn_minus);
}

}

