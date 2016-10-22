/*
 * deterministic.cc: 
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

#define max(a, b) ((a > b) ? a : b)

namespace gautomata
{

using namespace gbdd;

static State add_powerstate(BNfa& res, 
			    BNfa old, 
			    hash_map<StateSet, State>& powerstate_to_state,
			    hash_map<State, StateSet>& state_to_powerstate,
			    StateSet new_state, 
			    bool starting)
{
	if (powerstate_to_state.find(new_state) == powerstate_to_state.end())
	{
		bool accepting = !((old.states_accepting() & new_state).is_false());

		powerstate_to_state[new_state] = res.add_state(accepting, starting);
		state_to_powerstate[powerstate_to_state[new_state]] = new_state;
	}

	return powerstate_to_state[new_state];
}

static Bdd redirect_powerstates(Bdd p, Bdd::Var v, hash_map<Bdd, Bdd>& powerstate_to_state)
{
	if(!p.bdd_is_leaf() && p.bdd_var() < v)
	{
		return Bdd::var_then_else(p.get_space(), p.bdd_var(), 
					  redirect_powerstates(p.bdd_then(), v, powerstate_to_state),
					  redirect_powerstates(p.bdd_else(), v, powerstate_to_state));
	}
	else
	{
		return powerstate_to_state[p];
	}
}

vector<SymbolSet> BNfa::powerstate_successor_syms(BNfa old, StateSet from)
{
	Bdd::VarPool pool;

	Relation transitions = old.transitions();

	Domain dom_q1 = transitions.get_domain(0);
	Domain dom_q2 = transitions.get_domain(2);

	pool.alloc(dom_q1|dom_q2);

	Domain dom_a1 = Domain::infinite(base_alphabet, 2);
	Domain dom_a2 = Domain::infinite(base_alphabet+1, 2);

	BinaryRelation exists_from_q1_on_a1(dom_a1, dom_q2,
					    Relation(dom_q1 * dom_a1 * dom_q2, transitions).restrict(0, from).project(0).get_bdd());

	BinaryRelation exists_from_q1_on_a2(dom_a2, dom_q2, exists_from_q1_on_a1);

	EquivalenceRelation a1_iff_a2(dom_a1, dom_a2, Bdd::bdd_product(exists_from_q1_on_a1.get_bdd(),
								       exists_from_q1_on_a2.get_bdd(),
								       Bdd::fn_iff).forall(dom_q2));


	vector<BddSet> bdd_sets = a1_iff_a2.quotient(old.alphabet());

	return vector<SymbolSet>(bdd_sets.begin(), bdd_sets.end());
}

/**
 * find_powerstates:
  * @dom_powerstate Domain to use for powerstates
 * @dom_powerstate_begin First variable for @dom_powerstate (even if @dom_powerstate is empty)
 * 
 * Finds powerstates in an automaton and records the transition relation from each powerstate
 * 
 * Returns: A vector of powerstates and the transition relations in @powerstate_to_sym_powerstate
 */

vector<StateSet> BNfa::find_powerstates(Domain dom_powerstate, 
					Bdd::Var dom_powerstate_begin,
					vector<Bdd>& powerstate_to_sym_powerstate) const
{
	// Create transitions that has powerstates as the last variables */
	Relation transitions;
	{
		transitions = Relation(_transitions.get_domain(0) *
				       Domain(base_alphabet, max(base_alphabet, _transitions.get_bdd().highest_var()) + 1 - base_alphabet) *
				       _transitions.get_domain(2),
				       _transitions.get_bdd());

		Domains domains = get_transitions_domains();
		domains[2] = dom_powerstate;
		transitions = Relation(domains, transitions);
	}


	// Apply subset construction, beginning with the set of starting states 
	vector<StateSet> powerstates;
	powerstates.push_back(StateSet(dom_powerstate, states_starting()));

	hash_set<Bdd> found_powerstates;
	found_powerstates.insert(powerstates[0].get_bdd());

	int current_state;
	for (current_state = 0;current_state < powerstates.size();++current_state)
	{
		StateSet power_from =  powerstates[current_state];

		Bdd sym_powerstate = transitions.restrict(0, power_from).project(0).get_bdd();

		/* Save sym_powerstate relation for this state */
		powerstate_to_sym_powerstate.push_back(sym_powerstate);

		/* Add successors */
		hash_set<Bdd> successors = sym_powerstate.with_geq_var(dom_powerstate_begin);
		hash_set<Bdd>::const_iterator i;
		for (i = successors.begin();i != successors.end();++i)
		{
			if (found_powerstates.find(*i) == found_powerstates.end())
			{
				powerstates.push_back(StateSet(dom_powerstate, *i));
				found_powerstates.insert(*i);
			}
		}
	}

	return powerstates;
}

Relation BNfa::powerstates_transitions(BNfa orig,
				       BNfa res,
				       Bdd::Var dom_powerstate_begin,
				       vector<StateSet> powerstates, 
				       vector<Bdd> powerstate_to_sym_powerstate)
{
	Domains domains = res.get_transitions_domains();
	domains[1] = Domain(base_alphabet, max(base_alphabet, orig._transitions.get_bdd().highest_var()) + 1 - base_alphabet);
	domains[2] = Domain(dom_powerstate_begin, domains[2].size());

	Bdd transitions(res.get_space(), false);

	hash_map<Bdd, Bdd> powerstate_to_state;
	{
		int i;
		for (i = 0;i < powerstates.size();++i)
		{
			powerstate_to_state[powerstates[i].get_bdd()] = Bdd::value(res.get_space(), domains[2], i);

			transitions |= 	(powerstate_to_sym_powerstate[i] & 
					 Bdd::value(res.get_space(), domains[0], i));
			
		}
	}

	/* If the automaton was complete, then we get no empty powerstate. There may be that there
	 * is not an even (to the power of two) of states in the new automaton, therefore we have
	 * to make transitions from these bogus states. We replace with false to get no transitions
	 * at all.
	 *
	 * If the automaton is not complete, we get extra transitions to the empty powerstate. These
	 * are removed below.
	 *
	 */

	if (powerstate_to_state.find(Bdd(res.get_space(), false)) == powerstate_to_state.end())
	{
		powerstate_to_state[Bdd(res.get_space(), false)] = Bdd(res.get_space(), false);
	}

	/* We restrict to res.states() to get rid of bogus transitions from states that did not
	 * exist and therefore got transitions to the empty powerstate
	 */

	return Relation(domains, 
			redirect_powerstates(transitions, dom_powerstate_begin, powerstate_to_state)).
		restrict(0, res.states());
}

#undef DETERMINISTIC_VERSION_ORIGINAL
#define DETERMINISTIC_VERSION_SMART

/**
 * deterministic:
 * 
 * Applies the subset construction to obtain a deterministic automaton
 * 
 * Returns: A deterministic automaton equivalent to this automaton
 */

BNfa BNfa::deterministic() const
{
	if (is_necessarily_complete_deterministic) return *this;

#ifdef DETERMINISTIC_VERSION_SMART
	Domain dom_powerstate = Domain(base_extra, _transitions.get_domain(2).size());

	// Find powerstates
	vector<Bdd> powerstate_to_sym_powerstate;
	vector<StateSet> powerstates = find_powerstates(dom_powerstate, base_extra, powerstate_to_sym_powerstate);


	BNfa res(_space);
	res.set_n_states(powerstates.size());

	Relation tr = powerstates_transitions(*this, res, base_extra, powerstates, powerstate_to_sym_powerstate);
	res._transitions = Relation(res.get_transitions_domains(), tr);

	Relation powerstate_enum = Relation::enumeration(powerstates, res.states().get_domain());

	res._accepting = powerstate_enum.restrict(0, states_accepting()).project_on(1);
	res._starting = StateSet(res.states(), 0);

	res.is_necessarily_complete_deterministic = true;

	return res;
#endif /* DETERMINISTIC_VERSION_SMART */
#ifdef DETERMINISTIC_VERSION_ORIGINAL
	BNfa old = *this;

	BNfa res(_space);
	hash_map<StateSet, State> powerstate_to_state;
	hash_map<State, StateSet> state_to_powerstate;

	add_powerstate(res, old, powerstate_to_state, state_to_powerstate, old.states_starting(), true);

	State from = 0;
	while (from < res.n_states)
	{
		StateSet power_from =  state_to_powerstate[from];

		vector<SymbolSet> successor_syms = powerstate_successor_syms(old, power_from);

		vector<SymbolSet>::const_iterator j = successor_syms.begin();
		while (j != successor_syms.end())
		{
			SymbolSet on = *j;

			StateSet power_to = old.successors(power_from, on);

			State to = add_powerstate(res, old, powerstate_to_state, state_to_powerstate, power_to, false);
			res.add_edge(from, on, to);

			++j;
		}

		++from;
	}

	res.is_necessarily_complete_deterministic = true;

	return res;
#endif /* DETERMINISTIC_VERSION_ORIGINAL */
}


}
