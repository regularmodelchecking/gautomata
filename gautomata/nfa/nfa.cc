/*
 * nfa.cc:
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

#include "nfa.h"
#include <assert.h>
#include <string>
#include <set>
#include <map>
#include <queue>
#include <time.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <sstream>
#include <iostream>

namespace gautomata
{

using namespace gbdd;       

hash_map<State,State> Nfa::copy_states(const Nfa& a,
				       StateSet states_to_copy,
				       StateSet accepting,
				       StateSet starting)
{
	StateSet Q = states_to_copy;
	StateSet F = accepting;
	StateSet Init = starting;
	hash_map<State,State> state_map;

	for (StateSet::const_iterator i = Q.begin();i != Q.end();++i)
	{
		state_map[*i] = add_state(F.member(*i), Init.member(*i));
	}

	return state_map;
}

hash_map<State,State> Nfa::copy_states_and_transitions(const Nfa& a,
						       StateSet states_to_copy,
						       StateSet accepting,
						       StateSet starting)
{
	hash_map<State,State> state_map = copy_states(a, states_to_copy, accepting, starting);
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


Nfa::~Nfa()
{}

Nfa* Nfa::ptr_clone() const
{

	return ptr_with_starting_accepting(states_starting(), states_accepting());
}

StateSet Nfa::states_productive() const
{
	return reachable_predecessors(states_accepting(), alphabet());
}

StateSet Nfa::states_reachable() const
{
	return reachable_successors(states_starting(), alphabet());
}

StateSet Nfa::states_live() const
{
	return states_productive() & states_reachable();
}

Nfa* Nfa::ptr_product(const gbdd::BddBased& a2, bool (*fn)(bool v1, bool v2)) const
{
	return ptr_product(dynamic_cast<const Nfa&>(a2), fn);
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





Nfa* Nfa::ptr_product(const Nfa& in_a2,
		      bool (*fn)(bool v1, bool v2)) const
{
	const Nfa& in_a1 = *this;
	auto_ptr<Nfa> a1(in_a1.ptr_clone());
	auto_ptr<Nfa> a2(in_a2.ptr_clone());



	if (fn_is_monotonic(fn))
	{
		
		{
			
			State q = a1->add_state(false, true);

			a1->add_edge(a1->states(), a1->alphabet(), Set(a1->states(), q));
			
		}

		{
			State q = a2->add_state(false, true);

			a2->add_edge(a2->states(), a2->alphabet(), Set(a2->states(), q));
		}
	}



		
	else
	{
		a1.reset(a1->ptr_deterministic());
		a2.reset(a2->ptr_deterministic());
		

		
		
		}




	Nfa* res = a1->ptr_factory()->ptr_empty();
	map<pair<State,State>,State> state_map; 
	queue<pair<State,State> > to_explore;


	StateSet F1 = a1->states_accepting();
	StateSet F2 = a2->states_accepting();
	

	Space* space = a1->alphabet().get_space();

	

	StateSet s1 = a1->states_starting();
	StateSet s2 = a2->states_starting();
	
	for (StateSet::const_iterator i= s1.begin(); i!= s1.end(); ++i){
		for (StateSet::const_iterator j= s2.begin(); j!= s2.end(); ++j){

			state_map[make_pair(*i,*j)] = res->add_state(fn(F1.member(*i), F2.member(*j)), true);
			to_explore.push(make_pair(*i,*j));
		}
	}

	
	

	while (!to_explore.empty())
	{
		pair<State,State> q = to_explore.front();
		to_explore.pop();

		State q1 = q.first;
		State q2 = q.second;

		StateSet q1_succ = a1->successors(StateSet(space, q1), a1->alphabet());
		StateSet q2_succ = a2->successors(StateSet(space, q2), a2->alphabet());

		for (StateSet::const_iterator i1 = q1_succ.begin();i1 != q1_succ.end();++i1)
		{
			for (StateSet::const_iterator i2 = q2_succ.begin();i2 != q2_succ.end();++i2)
			{
				SymbolSet edge = a1->edge_between(q1, *i1) & a2->edge_between(q2, *i2);

				if (!(edge.is_empty()))
				{
					pair<State,State> r = make_pair(*i1, *i2);
					if (state_map.find(r) == state_map.end())
					{
						state_map[r] = res->add_state(fn(F1.member(*i1), F2.member(*i2)));
						to_explore.push(r);
					}
					
					res->add_edge(state_map[q], edge, state_map[r]);
				}
			}
		}
	}

	return res;
}
















Nfa* Nfa::ptr_product_and(const Nfa& a2) const
{
	return ptr_product(a2, Bdd::fn_and);
}

Nfa* Nfa::ptr_product_or(const Nfa& a2) const
{
	return ptr_product(a2, Bdd::fn_or);
}

Nfa* Nfa::ptr_product_minus(const Nfa& a2) const
{
	return ptr_product(a2, Bdd::fn_minus);
}

Nfa* Nfa::ptr_concatenation(const Nfa& a2) const
{
	const Nfa& a1 = *this;
	Nfa* res = auto_ptr<Factory>(ptr_factory())->ptr_empty();

	StateSet from_a1_accepting;
	if (!(a2.states_starting() & a2.states_accepting()).is_empty())
	{
		// epsilon is in the language of a2
		from_a1_accepting = a1.states_accepting();
	}
	else
	{
		// epsilon is not in the language of a2
		from_a1_accepting = StateSet::empty(a1.states());
	}

	hash_map<State,State> statemap_a1 = 
		res->copy_states_and_transitions(a1,
						 a1.states(),
						 from_a1_accepting,
						 a1.states_starting());

	hash_map<State,State> statemap_a2 = 
		res->copy_states_and_transitions(a2,
						 a2.states(),
						 a2.states_accepting(),
						 StateSet::empty(a2.states()));


	// For every transition q1 -> q2 from a starting state q1 in a2, add a transition
	// q3 -> q2 from every accepting state q3 in a1

	StateSet a2_starting = a2.states_starting();
	StateSet a1_accepting = a1.states_accepting();
	for (StateSet::const_iterator i_q1 = a2_starting.begin();i_q1 != a2_starting.end();++i_q1)
	{
		State q1 = statemap_a2[*i_q1];

		StateSet q1_succ = res->successors(StateSet(res->states(), q1), res->alphabet());

		for (StateSet::const_iterator i_q2 = q1_succ.begin();i_q2 != q1_succ.end();++i_q2)
		{
			State q2 = *i_q2;
			for (StateSet::const_iterator i_q3 = a1_accepting.begin();i_q3 != a1_accepting.end();++i_q3)
			{
				State q3 = statemap_a1[*i_q3];

				res->add_edge(q3, res->edge_between(q1, q2), q2);
			}
		}
	}

	return res;
}

			
Nfa::Var Nfa::highest_var() const
{
	assert(false);
}

Nfa::Var Nfa::lowest_var() const
{
	assert(false);
}

Nfa* Nfa::ptr_constrain_value(Var v, bool value) const
{
	Nfa* res = auto_ptr<Factory>(ptr_factory())->ptr_empty();

	hash_map<State,State> statemap = res->copy_states(*this,
							  states(),
							  states_accepting(),
							  states_starting());
	StateSet Q = states();


	for (StateSet::const_iterator i_q1 = Q.begin();i_q1 != Q.end();++i_q1)
	{
		State q1 = *i_q1;
		StateSet q1_succ = successors(StateSet(states(), q1), alphabet());

		for (StateSet::const_iterator i_q2 = q1_succ.begin();i_q2 != q1_succ.end();++i_q2)
		{
			State q2 = *i_q2;
			Bdd edge = SymbolSet(Domain::infinite(), edge_between(q1, q2)).get_bdd();
			SymbolSet constrained_edge(Domain::infinite(), dynamic_cast<Bdd&>(*auto_ptr<BddBased>(edge.ptr_constrain_value(v, value))));
			
			res->add_edge(statemap[q1], constrained_edge, statemap[q2]);
		}
	}

	return res;
}


Nfa* Nfa::ptr_rename(VarMap map) const
{
	Nfa* res = auto_ptr<Factory>(ptr_factory())->ptr_empty();
	StateSet Q = states();

	hash_map<State,State> state_map = res->copy_states(*this,
							   states(),
							   states_accepting(),
							   states_starting());

	for (StateSet::const_iterator i = Q.begin();i != Q.end();++i)
	{
		for (StateSet::const_iterator j = Q.begin();j != Q.end();++j)
		{
			Bdd bdd_edge = edge_between(*i, *j);

			res->add_edge(state_map[*i], bdd_edge.rename(map), state_map[*j]);
		}
	}

	return res;
}

Nfa* Nfa::ptr_project(Domain vs) const
{
	Nfa* res = auto_ptr<Factory>(ptr_factory())->ptr_empty();
	StateSet Q = states();

	hash_map<State,State> state_map = res->copy_states(*this,
							   states(),
							   states_accepting(),
							   states_starting());

	for (StateSet::const_iterator i = Q.begin();i != Q.end();++i)
	{
		for (StateSet::const_iterator j = Q.begin();j != Q.end();++j)
		{
			Bdd bdd_edge = edge_between(*i, *j);

			SymbolSet projected = bdd_edge.project(vs);

			res->add_edge(state_map[*i], projected, state_map[*j]);
		}
	}

	return res;
}
	

Nfa* Nfa::ptr_rename(Domain vs1, Domain vs2) const
{
	assert(vs1.size() == vs2.size());

	return ptr_rename(Domain::map_vars(vs1, vs2));
}

Nfa* Nfa::ptr_kleene() const
{
	Nfa* res = auto_ptr<Factory>(ptr_factory())->ptr_empty();

	hash_map<State,State> state_map = res->copy_states_and_transitions(*this,
									   states(),
									   states_accepting(),
									   states_starting());

	StateSet Init = states_starting();
	StateSet F = states_accepting();

	// From every transition q1 -> q2 where q1 is a starting state, add transitions
	// q3 -> q2 for every accepting state q3

	for (StateSet::const_iterator i_q1 = Init.begin();i_q1 != Init.end();++i_q1)
	{
		State q1 = *i_q1;
		StateSet q1_succ = successors(StateSet(states(), q1), alphabet());

		for (StateSet::const_iterator i_q2 = q1_succ.begin();i_q2 != q1_succ.end();++i_q2)
		{
			State q2 = *i_q2;
			for (StateSet::const_iterator i_q3 = F.begin();i_q3 != F.end();++i_q3)
			{
				State q3 = *i_q3;

				res->add_edge(state_map[q3], 
					      edge_between(q1, q2), 
					      state_map[q2]);
			}
		}
	}

	// Add state accepting epsilon

	State q_eps = res->add_state(true, true);

	return res;
}		

Nfa* Nfa::ptr_reverse() const
{
	Nfa* res = auto_ptr<Factory>(ptr_factory())->ptr_empty();

	hash_map<State,State> statemap = res->copy_states(*this,
							  states(),
							  states_starting(),
							  states_accepting());
	StateSet Q = states();


	for (StateSet::const_iterator i_q1 = Q.begin();i_q1 != Q.end();++i_q1)
	{
		State q1 = *i_q1;
		StateSet q1_succ = successors(StateSet(states(), q1), alphabet());

		for (StateSet::const_iterator i_q2 = q1_succ.begin();i_q2 != q1_succ.end();++i_q2)
		{
			State q2 = *i_q2;
			
			res->add_edge(statemap[q2], edge_between(q1, q2), statemap[q1]);
		}
	}

	return res;
}

Nfa* Nfa::ptr_negate() const
{
  Nfa* res =  auto_ptr<Nfa>(auto_ptr<Factory>(ptr_factory())->ptr_universal())->ptr_product_minus(*this);

  return res;
}


/// Change starting and accepting states of automaton
/**
 * 
 *
 * @param starting New set of starting states
 * @param accepting New set of accepting states
 * 
 * @return The same automaton but with \a starting and \a accepting as starting and accepting states
 */

Nfa* Nfa::ptr_with_starting_accepting(StateSet starting, StateSet accepting) const
{
	Nfa* res = auto_ptr<Factory>(ptr_factory())->ptr_empty();

	res->copy_states_and_transitions(*this, states(), accepting, starting);

	return res;
}

Nfa* Nfa::ptr_filter_states(StateSet s) const
{
	Nfa* res = auto_ptr<Factory>(ptr_factory())->ptr_empty();

	res->copy_states_and_transitions(*this, s, states_accepting(), states_starting());

	return res;
}

Nfa* Nfa::ptr_filter_states_live() const
{
	return ptr_filter_states(states_live());
}

Nfa* Nfa::ptr_filter_states_reachable() const
{
	return ptr_filter_states(states_reachable());
}

Nfa* Nfa::ptr_filter_states_productive() const
{
	return ptr_filter_states(states_productive());
}

bool Nfa::is_true() const
{
	auto_ptr<Nfa> ptr_det(ptr_deterministic());

	return ptr_det->states_accepting() == ptr_det->states_reachable();
}
	
bool Nfa::is_false() const
{
	return states_live().is_empty();
}

bool Nfa::operator==(const BddBased &a2) const
{
	return *this == dynamic_cast<const Nfa&>(a2);
}

bool Nfa::operator==(const Nfa &a2) const
{
	return ptr_product(a2, Bdd::fn_iff)->is_true();
}

bool Nfa::operator!=(const Nfa &a2) const
{
	return !(*this == a2);
}

bool Nfa::operator<=(const Nfa &a2) const
{
	return ptr_product(a2, Bdd::fn_implies)->is_true();
}

bool Nfa::operator>=(const Nfa &a2) const
{
	return a2 <= *this;
}

bool Nfa::operator<(const Nfa &a2) const
{
	return *this <= a2 && *this != a2;
}

bool Nfa::operator>(const Nfa &a2) const
{
	return a2 >= a2 && *this != a2;
}

set<string> generate_paths(gbdd::Bdd p, unsigned int i, string prefix)
{
	if (p.bdd_is_leaf())
	{
		set<string> paths;

		if (p.bdd_leaf_value())
		{
			paths.insert(prefix);
		}

		return paths;
	}
	else
	{
		unsigned int index = p.bdd_var();

		if (i < index)
		{
			return generate_paths(p, i+1, prefix + "X");
		}
		else
		{
			assert(i == index);

			set<string> paths;

			set<string> paths1 = generate_paths(p.bdd_then(), i+1, prefix + "1");
			set<string> paths0 = generate_paths(p.bdd_else(), i+1, prefix + "0");

			paths = paths1;
			paths.insert(paths0.begin(), paths0.end());

			return paths;
		}
	}
}

void print_paths(ostream& out, set<string> paths)
{
	set<string>::const_iterator i;

	i = paths.begin();
	if (i == paths.end()) return;

	out << *i;
	++i;

	for(;i != paths.end();++i)
	{
		out << "\\n" << *i;
	}
}

ostream& operator<<(ostream &out, const Nfa &nfa)
{
	return nfa.print_dot(out, Nfa::PrintStrategy());
}

string Nfa::PrintStrategy::format_attributes() const
{
	return "";
}

string Nfa::PrintStrategy::format_state(const Nfa& nfa, State q) const
{
	ostringstream s;
	s << q;
	return s.str();
}

string Nfa::PrintStrategy::format_edge(const Nfa& nfa, const SymbolSet& edge) const
{
	gbdd::Domain edge_print_domain = gbdd::Domain::infinite();

	ostringstream out;
	print_paths(out, generate_paths(Set(edge_print_domain, edge).get_bdd(),
					0,
					""));

	return out.str();
}

Nfa::EnumeratePrintStrategy::EnumeratePrintStrategy(Domains ds):
	ds(ds)
{}

string Nfa::EnumeratePrintStrategy::format_edge(const Nfa& nfa, const SymbolSet& edge) const
{
	using gbdd::BddSet;
	using gbdd::BddBasedSet;
	using gbdd::BddRelation;
	using gbdd::BddBasedRelation;
	ostringstream out;

	// View edge as relation r
	gbdd::BddRelation r(ds, SymbolSet(gbdd::Domain::infinite(), edge).get_bdd());

	vector<BddSet> projected;
	{
		unsigned int i;
		for (i = 0;i < r.arity();++i)
		{
			projected.push_back(r.project_on(i));
		}
	}
	vector<BddSet::const_iterator> is;

	{
		unsigned int i;
		
		for(i = 0;i < r.arity();++i)
		{
			is.push_back(projected[i].begin());
		}
	}

	bool first = true;

	while(is.size() > 0 && is[0] != projected[0].end())
	{
		{
			vector<BddBasedSet> element;
			unsigned int i;

			for(i = 0; i < is.size();++i)
			{
				element.push_back(BddSet(projected[i], *(is[i])));
			}

			BddRelation prod = BddBasedRelation::cross_product(r.get_domains(), element);

			if (!(prod & r).is_false())
			{
				if (first) first = false;
				else out << "\\n";

				if (is.size() > 1) out << "(";

				out << format_edge_symbol(nfa, 0, *(is[0]));
				vector<BddSet::const_iterator>::iterator j = is.begin();

				++j;
				
				unsigned int domain_i = 1;

				while(j != is.end())
				{
					out << "," << format_edge_symbol(nfa, domain_i, **j);
					++j;
					++domain_i;
				}
				
				if (is.size() > 1) out << ")";
			}
		}

		// Increase to next

		unsigned int i = r.arity() - 1;

		++(is[i]);

		while (is[i] == projected[i].end() && i > 0)
		{
			is.pop_back();
		        --i;
			++(is[i]);
		}
		
		if(is[i] != projected[i].end())
		{
			++i;
			while (i < projected.size())
			{
				is.push_back(projected[i].begin());
				++i;
			}
		}

	}

	return out.str();
}

string Nfa::EnumeratePrintStrategy::format_edge_symbol(const Nfa& nfa, unsigned int domain_i, unsigned int v) const
{
	ostringstream s;
	s << v;
	return s.str();
}


ostream& Nfa::print_dot(ostream& out, const PrintStrategy& ps, const StateSet& states) const
{
	const Nfa& nfa = *this;

	StateSet starting = nfa.states_starting() & states;
	StateSet accepting = nfa.states_accepting() & states;
	StateSet not_accepting = states - accepting;

	out << "digraph NFA {" << endl;
	out << "rankdir = LR;" << endl;
	out << "center = true;" << endl;
	out << "edge [fontname = Courier];" << endl;
	out << "node [height = .5, width = .5];" << endl;
	
	out << "node [shape = doublecircle];";
	out << ps.format_attributes() << endl;

	{
		StateSet::const_iterator i = accepting.begin();
		while (i != accepting.end())
		{
			out << " " << ps.format_state(nfa, *i);
			++i;
		}
	}

	if (!(accepting.is_empty())) out << ";\n";

	out << "node [shape = circle];";

	{
		StateSet::const_iterator i = not_accepting.begin();

		while (i != not_accepting.end())
		{
			out << " " << ps.format_state(nfa, *i);
			++i;
		}
	}

	if (!(not_accepting.is_empty())) out << ";\n";

	{
		StateSet::const_iterator i = starting.begin();

		while (i != starting.end())
		{
			out << "init" << *i << " [shape = plaintext, label = \"\"];" << endl;
			out << "init" << *i << " -> " << ps.format_state(nfa, *i) << ";" << endl;

			++i;
		}
	}
	
	gbdd::Domain edge_print_domain = gbdd::Domain::infinite();

	StateSet::const_iterator i = states.begin();

	while (i != states.end())
	{
		StateSet destinations = 
			nfa.successors(Set(states, *i), nfa.alphabet()) & states;

		StateSet::const_iterator j = destinations.begin();

		while(j != destinations.end())
		{
			out << ps.format_state(nfa, *i) << " -> " << ps.format_state(nfa, *j) <<
				" [label=\"";
			SymbolSet edge = 
				nfa.edge_between(*i, *j);

		        out << ps.format_edge(nfa, edge);

			out << "\"];" << endl;

			++j;
		}

		++i;
	}
	
  out << "}" << endl;

  return out;
}

// Factory methods
Nfa::Factory::~Factory()
{}

Nfa* Nfa::Factory::ptr_universal() const
{
	Nfa* nfa = ptr_empty();

	State q = nfa->add_state(true, true);
	nfa->add_edge(q, nfa->alphabet(), q);

	return nfa;
}
			
Nfa* Nfa::Factory::ptr_epsilon() const
{
	Nfa* nfa = ptr_empty();

	nfa->add_state(true, true);

	return nfa;
}
	
Nfa* Nfa::Factory::ptr_symbol(SymbolSet sym) const
{
	Nfa* nfa = ptr_empty();

	State q = nfa->add_state(false, true);
	State r = nfa->add_state(true);

	nfa->add_edge(q, sym, r);

	return nfa;
}

static bool with_probability(float p)
{
	return rand() < RAND_MAX * p;
}

static SymbolSet edge_with_density(Space* space, 
				   unsigned int n_vars,
				   float density)
{
	Bdd p(space, true);

	for (int i = 0;i < n_vars;++i)
	{
		if (with_probability(density))
		{
			if (with_probability(0.5))
			{
				p &= Bdd::var_true(space, i);
			}
			else
			{
				p &= Bdd::var_false(space, i);
			}
		}
	}

	return SymbolSet(Domain::infinite(), p);
}

	

/// Generate random automaton
/**
 * 
 *
 * @param n_states Number of states in generated automaton
 * @param n_vars Number of BDD variables used in alphabet
 * @param edge_density Probability of existing an edge between two states
 * @param symbol_density Probability of each variable being constrained in an edge
 * 
 * @return Automaton with \a n_states number of states and possible
 *         using up to \a n_vars number of variables on edges.
 */
Nfa* Nfa::Factory::ptr_random(unsigned int n_states,
				       unsigned int n_vars,
				       float edge_density,
				       float symbol_density) const
{
	struct timeb tb;
	ftime(&tb);
	
	//srand(time(NULL) + tb.millitm);
	srand(2345823);

	Nfa* nfa = ptr_empty();
	Space* space = nfa->alphabet().get_space();


	for (int i = 0;i < n_states;++i)
	{
		nfa->add_state(with_probability(0.5),
			       with_probability(0.5));
	}

	StateSet Q = nfa->states();

	for (StateSet::const_iterator i_q1 = Q.begin();i_q1 != Q.end();++i_q1)
	{
		State q1 = *i_q1;
		for (StateSet::const_iterator i_q2 = Q.begin();i_q2 != Q.end();++i_q2)
		{
			State q2 = *i_q2;

			if (with_probability(edge_density))
			{
				nfa->add_edge(q1, edge_with_density(space, n_vars, symbol_density), q2);
			}
		}
	}

	return nfa;
}

Nfa* Nfa::Factory::ptr_clone(const Nfa& a) const
{
	Nfa* res = ptr_empty();

	res->copy_states_and_transitions(a, a.states(), a.states_accepting(), a.states_starting());

	return res;
}

StructureConstraint* Nfa::Factory::ptr_forall(const BoolConstraint& c) const
{
	const Bdd& p = dynamic_cast<const Bdd&>(c); // Only supporting this for now, use ptr_convert later

	auto_ptr<Nfa> sym (ptr_symbol(p));

	return sym->ptr_kleene();
}

set<Nfa::StatePair> Nfa::find_simulation_forward(const Nfa& a2, const StateSet& a1_states, const StateSet& a2_states) const
{
	const Nfa& a1 = *this;

	SimulationGraph g(a1, a2, a1_states, a2_states);

	SimulationGraph::NodeSet initial = g.all_nodes();

	// Ensure accepting1 => accepting2, false if accepting1 and not accepting2

	StateSet F1 = a1.states_accepting();
	StateSet Not_F2 = a2.states() - a2.states_accepting();

	for (StateSet::const_iterator i = F1.begin();i != F1.end();++i)
	{
		for (StateSet::const_iterator j = Not_F2.begin();j != Not_F2.end();++j)
		{
			initial.erase(g.main_node(*i, *j));
		}
	}

	return g.simulation_fixpoint(initial).simulation();
}

set<Nfa::StatePair> Nfa::find_simulation_backward(const Nfa& a2, const StateSet& a1_states, const StateSet& a2_states) const
{
	const Nfa& a1 = *this;

	ReverseWordAutomaton a1_reverse(a1);
	ReverseWordAutomaton a2_reverse(a2);

	SimulationGraph g(a1_reverse, a2_reverse, a1_states, a2_states);

	SimulationGraph::NodeSet initial = g.all_nodes();

	// Ensure starting1 => starting2, false if starting1 and not starting2

	StateSet I1 = a1.states_starting();
	StateSet Not_I2 = a2.states() - a2.states_starting();

	for (StateSet::const_iterator i = I1.begin();i != I1.end();++i)
	{
		for (StateSet::const_iterator j = Not_I2.begin();j != Not_I2.end();++j)
		{
			initial.erase(g.main_node(*i, *j));
		}
	}

	return g.simulation_fixpoint(initial).simulation();
}

set<Nfa::StatePair> Nfa::find_simulation_forward(const Nfa& a2) const
{
	return find_simulation_forward(a2, states(), a2.states());
}

set<Nfa::StatePair> Nfa::find_simulation_backward(const Nfa& a2) const
{
	return find_simulation_backward(a2, states(), a2.states());
}

}
