/*
 * bnfa.cc: 
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
v * 
 * Authors:
 *    Marcus Nilsson (marcusn@docs.uu.se)
 */

#include "bnfa.h"
#include <assert.h>
#include <iostream>

#define max(a, b) ((a > b) ? a : b)

namespace gautomata
{

using namespace gbdd;       

BNfa::Factory::Factory(Space* space):
	_space(space)
{}

BNfa* BNfa::Factory::ptr_empty() const
{
	return new BNfa(_space, false);
}

BNfa* BNfa::Factory::ptr_universal() const
{
	return new BNfa(_space, true);
}

BNfa* BNfa::Factory::ptr_epsilon() const
{
	return new BNfa(BNfa::epsilon(_space));
}

BNfa* BNfa::Factory::ptr_symbol(SymbolSet sym) const
{
	return new BNfa(BNfa::symbol(_space, sym));
}
	

BNfa::Factory* BNfa::ptr_factory() const
{
	return new Factory(_space);
}

Domain BNfa::map_transitions_source(Domain vs) const
{
	return vs + base_source;
}
		
Domain BNfa::map_transitions_alphabet(Domain vs) const
{
	return vs + base_alphabet;
}

Domain BNfa::map_transitions_dest(Domain vs) const
{
	return vs + base_dest;
}

Domains BNfa::get_transitions_domains() const
{
	Domain vars_states = Domain(0, Bdd::n_vars_needed(n_states));
	Domain vars_syms = Domain::infinite();

	return 
		map_transitions_source(vars_states) *
		map_transitions_alphabet(vars_syms) *
		map_transitions_dest(vars_states);
}	

/// Creates a new automaton.
/**
 * 
 *
 * @param space The BDD space which will be used for the representation
 */
BNfa::BNfa(Space* space):
	_space(space),
	n_states(0),
	_starting(Domain(), Bdd(space, false)),
	_accepting(Domain(), Bdd(space, false)),
	_states(Domain(), Bdd(space, false)),
	_transitions(Domains(3), Bdd(space, false)),
	is_necessarily_complete_deterministic(false)
{
	_transitions = Relation(get_transitions_domains(), Bdd(space, false));
}


BNfa::BNfa(const BNfa &bnfa):
	_space(bnfa._space),
	n_states(bnfa.n_states),
	_starting(bnfa._starting),
	_accepting(bnfa._accepting),
	_states(bnfa._states),
	_transitions(bnfa._transitions),
	is_necessarily_complete_deterministic(bnfa.is_necessarily_complete_deterministic)
{}

BNfa::BNfa(const Nfa &nfa):
	_starting(nfa.states_starting()),
	_accepting(nfa.states_accepting()),
	_states(nfa.states()),
	is_necessarily_complete_deterministic(false)
{
	_space = _starting.get_space();
	n_states = _states.size();

	_transitions = Relation(get_transitions_domains(), nfa.transitions());
}

BNfa::BNfa(Space* space, bool v):
	_space(space),
	n_states(0),
	_starting(Domain(), Bdd(space, false)),
	_accepting(Domain(), Bdd(space, false)),
	_states(Domain(), Bdd(space, false)),
	_transitions(Domains(3), Bdd(space, false)),
	is_necessarily_complete_deterministic(false)
{
	_transitions = Relation(get_transitions_domains(), Bdd(space, false));

	if (v)
	{
		State q = add_state(true, true);

		add_edge(q, alphabet(), q);
	}
}

BNfa::~BNfa()
{}

BNfa* BNfa::ptr_clone() const
{
	return new BNfa(*this);
}

Space* BNfa::get_space() const
{
	return _space;
}

unsigned int BNfa::get_n_states() const
{
	return n_states;
}

BNfa BNfa::epsilon(Space* space)
{
	BNfa nfa(space);

	nfa.add_state(true, true);

	nfa.is_necessarily_complete_deterministic = false;

	return nfa;
}

BNfa BNfa::symbol(Space* space, SymbolSet sym)
{
	BNfa nfa(space);
	
	State q = nfa.add_state(false, true);
	State r = nfa.add_state(true);

	nfa.add_edge(q, sym, r);

	nfa.is_necessarily_complete_deterministic = false;

	return nfa;
}

SymbolSet BNfa::alphabet() const
{
	return Set(map_transitions_alphabet(Domain::infinite()), Bdd(_space, true));
}

StateSet BNfa::states() const
{
	return _states;
}

StateSet BNfa::states_starting() const
{
	return _starting;
}

Relation BNfa::transitions() const
{
	return _transitions;
}

StateSet BNfa::states_accepting() const
{
	return _accepting;
}

SymbolSet BNfa::edge_between(StateSet q, StateSet r) const
{
	return Nfa::edge_between(q, r);
}

SymbolSet BNfa::edge_between(State q, State r) const
{
	Domains doms = _transitions.get_domains();

	return SymbolSet(doms[1],
			 _transitions.get_bdd().value_follow(doms[0], q).value_follow(doms[2], r));
}

	

static gbdd::Bdd::Var expand_even(gbdd::Bdd::Var v) { return 2 * v; }
static gbdd::Bdd::Var expand_odd(gbdd::Bdd::Var v) { return 2 * v + 1; }


BNfa BNfa::filter_states(StateSet s) const
{
	BNfa res = *this;

	if (s.size() == 0) return BNfa(_space);

	res._starting &= s;
	res._accepting &= s;

	Relation compressmap = s.compress();

	res.set_n_states(s.size());

	Domain new_domain = res.states().get_domain();
	Domains new_transitions_domains =
		res.get_transitions_domains();

	res._starting = StateSet(new_domain, res._starting.compose(compressmap));
	res._accepting = StateSet(new_domain, res._accepting.compose(compressmap));

	res._transitions = Relation(new_transitions_domains, res._transitions.compose(0, compressmap).compose(2, compressmap));

	res.is_necessarily_complete_deterministic = false;

	return res;
}

BNfa BNfa::filter_states_live() const
{
	return filter_states(states_live());
}

BNfa BNfa::filter_states_reachable() const
{
	return filter_states(states_reachable());
}

BNfa BNfa::filter_states_productive() const
{
	return filter_states(states_productive());
}



BNfa BNfa::project(Domain vs) const
{
	BNfa res(*this);

	res._transitions = 
		Relation(res._transitions.get_domains(),
			 (res._transitions.get_bdd()).project(map_transitions_alphabet(vs)));

	res.is_necessarily_complete_deterministic = false;

	return res;
}

BNfa BNfa::rename(VarMap map) const
{
	VarMap new_map;

	VarMap::const_iterator i;
	for (i = map.begin();i != map.end();++i)
	{
		Domain source = map_transitions_alphabet(Domain(i->first, 1));
		Domain dest = map_transitions_alphabet(Domain(i->second, 1));

		new_map[*(source.begin())] = *(dest.begin());
	}

	BNfa res(*this);

	res._transitions =
		Relation(res._transitions.get_domains(),
			 res._transitions.get_bdd().rename(new_map));

	return res;
}

BNfa BNfa::rename(Domain vs1, Domain vs2) const
{
	assert(vs1.size() == vs2.size());

	return rename(Domain::map_vars(vs1, vs2));
}

BNfa operator*(const BNfa &a1, const BNfa &a2)
{
	unsigned int n_vars_a1 = Bdd::n_vars_needed(a1.n_states);
	unsigned int n_vars_a2 = Bdd::n_vars_needed(a2.n_states);

	unsigned max_n_vars = max(n_vars_a1, n_vars_a2);

	BNfa res(a1._space);
	res.set_n_states(1 << (max_n_vars+1));

	Domain new_state_domain = res.states().get_domain();
	Domains new_transitions_domains =
		res.get_transitions_domains();

	// Color states from a1 and a2

	Set a1_starting = a1._starting.extend_domain(new_state_domain, false);
	Set a1_accepting = a1._accepting.extend_domain(new_state_domain, 
						       false);
	Relation a1_transitions =
		a1._transitions.
		extend_domain(0, new_transitions_domains[0], false).
		extend_domain(2, new_transitions_domains[2], false);
		

	Set a2_starting = a2._starting.extend_domain(new_state_domain, true);
	Set a2_accepting = a2._accepting.extend_domain(new_state_domain,
						       true);
	Relation a2_transitions =
		a2._transitions.
		extend_domain(0, new_transitions_domains[0], true).
		extend_domain(2, new_transitions_domains[2], true);


	// Initialize new automaton

	res._starting = a1_starting;
	res._accepting = a2_accepting;
	if (!((a2_starting & a2_accepting).is_false()))
		res._accepting |= a1_accepting;

	Relation from_a2_starting =
		a2_transitions.restrict(0, a2_starting).project(0);

	res._transitions = 
		a1_transitions | 
		a2_transitions |
		from_a2_starting.restrict(0, a1_accepting);

	res.is_necessarily_complete_deterministic = false;

	return res.filter_states_live();		
}

BNfa BNfa::kleene() const
{
	BNfa res = *this;

	State qstart = res.add_state(true, true);

	Relation from_starting = 
		res._transitions.restrict(0, res.states_starting()).project(0);

	res.add_transitions(from_starting.restrict(0, StateSet(res.states(),
							       qstart)));

	res.add_transitions(from_starting.restrict(0, res.states_accepting()));

	res._starting = Set(res.states(), qstart);

	res.is_necessarily_complete_deterministic = false;

	return res.filter_states_live();
}

BNfa BNfa::reverse() const
{
	BNfa res = *this;

	res._accepting = _starting;
	res._starting = _accepting;
	res._transitions =
		Relation(_transitions.get_domains(),
			 Relation(_transitions.get_domain(2) *
				  _transitions.get_domain(1) *
				  _transitions.get_domain(0),
				  _transitions).get_bdd());

	res.is_necessarily_complete_deterministic = false;

	return res;
}

BNfa BNfa::negate() const
{
	if (is_necessarily_complete_deterministic)
	{
		BNfa res = with_starting_accepting(states_starting(), !states_accepting());

		res.is_necessarily_complete_deterministic = true;

		return res;
	}
	else
	{
		return BNfa(_space, true) - *this;
	}
}

static unsigned int automata_max_n_states(vector<BNfa> automata)
{
	vector<BNfa>::const_iterator i = automata.begin();

	if (i == automata.end()) return 0;

	unsigned int max_n_states = i->get_n_states();
	++i;

	for(;i != automata.end();++i)
	{
		max_n_states = max(max_n_states, i->get_n_states());
	}

	return max_n_states;
}

void BNfa::automata_increase_to_n_states(vector<BNfa>& automata, unsigned int n_states)
{
	vector<BNfa>::iterator i;
	for(i = automata.begin();i != automata.end();++i)
	{
		i->increase_to_n_states(n_states, false, false);
	}

	return;
}

vector<StateSet> BNfa::automata_states_starting(vector<BNfa> automata)
{
	vector<StateSet> res;

	vector<BNfa>::const_iterator i;
	for(i = automata.begin();i != automata.end();++i)
	{
		res.push_back(i->states_starting());
	}

	return res;
}

vector<StateSet> BNfa::automata_states_accepting(vector<BNfa> automata)
{
	vector<StateSet> res;

	vector<BNfa>::const_iterator i;
	for(i = automata.begin();i != automata.end();++i)
	{
		res.push_back(i->states_accepting());
	}

	return res;
}

vector<Relation> BNfa::automata_transitions(vector<BNfa> automata)
{
	vector<Relation> res;

	vector<BNfa>::const_iterator i;
	for(i = automata.begin();i != automata.end();++i)
	{
		res.push_back(i->transitions());
	}

	return res;
}

static Domain last_n_vars(Domain vs, unsigned int n_vars)
{
	Domain res;
	unsigned skip_n_vars = vs.size() - n_vars;

	Domain::const_iterator i;
	unsigned int index = 0;

	i = vs.begin();

	{
		int j;
		for(j = 0;j < skip_n_vars;++j) ++i;
	}

	for (;n_vars > 0;++i)
	{
		res |= Domain(*i);

		n_vars--;
	}

	return res;
}

static Domain first_n_vars(Domain vs, unsigned int n_vars)
{
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

StateSet states_union(vector<StateSet> states)
{
	StateSet res = *(states.begin());

	vector<StateSet>::const_iterator i = states.begin();
	++i;
	
	for(;i != states.end();++i)
	{
		res |= *i;
	}

	return res;
}

Relation relations_union(vector<Relation> relations)
{
	Relation res = *(relations.begin());

	vector<Relation>::const_iterator i = relations.begin();
	++i;
	
	for(;i != relations.end();++i)
	{
		res |= *i;
	}

	return res;
}

	

BNfa BNfa::color(vector<BNfa> automata, vector<Relation>& out_color_maps)
{
	assert(automata.size() > 0);

	vector<BNfa> orig_automata = automata;

	unsigned int n_state_vars = Bdd::n_vars_needed(automata_max_n_states(automata));
	unsigned int n_color_vars = Bdd::n_vars_needed(automata.size());

	automata_increase_to_n_states(automata, 1 << n_state_vars);

	BNfa res((automata.begin())->get_space());

	res.increase_to_n_states(1 << n_state_vars + n_color_vars, false, false);

	res._starting = states_union(Set::color(last_n_vars(res._starting.get_domain(), n_color_vars),
						automata_states_starting(automata)));

	res._accepting = states_union(Set::color(last_n_vars(res._starting.get_domain(), n_color_vars),
						 automata_states_accepting(automata)));

	Domain dom_color_source = last_n_vars(res._transitions.get_domain(0), n_color_vars);
	Domain dom_color_dest = last_n_vars(res._transitions.get_domain(2), n_color_vars);

	res._transitions = relations_union(Relation::color(2, dom_color_dest, 
							   Relation::color(0, dom_color_source, automata_transitions(automata))));

	{
		Bdd::VarPool pool;
		
		Domain dom_source = pool.alloc(n_state_vars);
		Domain dom_dest = pool.alloc(n_state_vars);

		Domain dom_color = pool.alloc(n_color_vars);

		out_color_maps = vector<Relation>();
		{
			vector<BNfa>::const_iterator i;
			for(i = orig_automata.begin();i != orig_automata.end();++i)
			{
				unsigned int n_states_vars_i = Bdd::n_vars_needed(i->get_n_states());

				Relation id = EquivalenceRelation::identity(res.get_space(), 
									    first_n_vars(dom_source, n_states_vars_i),
									    first_n_vars(dom_dest, n_states_vars_i));
				id = id.extend_domain(1, dom_dest);

				out_color_maps.push_back(id);
			}
		}

		out_color_maps = Relation::color(1, dom_color, out_color_maps);
	}

	return res;
}


BNfa BNfa::with_starting_accepting(StateSet starting,
				   StateSet accepting) const
{
	BNfa res = *this;

	res._starting = StateSet(res._starting.get_domain(), starting);
	res._accepting = StateSet(res._accepting.get_domain(), accepting);

	res.is_necessarily_complete_deterministic = false;

	return res;
}

bool BNfa::is_true() const
{
	BNfa det = deterministic();

	return det.states_accepting() == det.states_reachable();
}
	
bool BNfa::is_false() const
{
	return states_live().is_empty();
}

bool operator==(const BNfa &a1, const BNfa &a2)
{
	return BNfa::product(a1, a2, Bdd::fn_iff).is_true();
}

bool operator!=(const BNfa &a1, const BNfa &a2)
{
	return !(a1 == a2);
}

bool operator<=(const BNfa &a1, const BNfa &a2)
{
	return BNfa::product(a1, a2, Bdd::fn_implies).is_true();
}

bool operator>=(const BNfa &a1, const BNfa &a2)
{
	return a2 <= a1;
}

bool operator<(const BNfa &a1, const BNfa &a2)
{
	return a1 <= a2 && a1 != a2;
}

bool operator>(const BNfa &a1, const BNfa &a2)
{
	return a2 >= a1 && a1 != a2;
}



BNfa* BNfa::ptr_product(const Nfa& a2,
			  bool (*fn)(bool v1, bool v2)) const
{
	return new BNfa(product(*this, BNfa(a2), fn));
}

BNfa* BNfa::ptr_concatenation(const Nfa& a2) const
{
	return new BNfa(*this * BNfa(a2));
}

BNfa* BNfa::ptr_deterministic() const
{
	return new BNfa(deterministic());
}

BNfa* BNfa::ptr_minimize() const
{
	return new BNfa(minimize());
}

BNfa* BNfa::ptr_project(Domain vs) const
{
	return new BNfa(project(vs));
}

BNfa* BNfa::ptr_rename(VarMap map) const
{
	return new BNfa(rename(map));
}
BNfa* BNfa::ptr_rename(Domain vs1, Domain vs2) const
{
	return new BNfa(rename(vs1, vs2));
}

BNfa* BNfa::ptr_kleene() const
{
	return new BNfa(kleene());
}
BNfa* BNfa::ptr_reverse() const
{
	return new BNfa(reverse());
}
BNfa* BNfa::ptr_negate() const
{
	return new BNfa(negate());
}

BNfa* BNfa::ptr_with_starting_accepting(StateSet starting,
					  StateSet accepting) const
{
	return new BNfa(with_starting_accepting(starting, accepting));
}


BNfa* BNfa::ptr_filter_states(StateSet s) const
{
	return new BNfa(filter_states(s));
}
BNfa* BNfa::ptr_filter_states_live() const
{
	return new BNfa(filter_states_live());
}
BNfa* BNfa::ptr_filter_states_reachable() const
{
	return new BNfa(filter_states_reachable());
}
BNfa* BNfa::ptr_filter_states_productive() const
{
	return new BNfa(filter_states_productive());
}


}

