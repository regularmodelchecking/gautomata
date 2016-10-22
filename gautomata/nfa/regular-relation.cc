/*
 * regular-relation.cc: 
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

#include "regular-relation.h"

namespace gautomata
{

RegularRelation RegularRelation::empty(const RegularRelation& with_domain_as)
{
	auto_ptr<Nfa::Factory> factory(with_domain_as.get_automaton().ptr_factory());

	return RegularRelation(with_domain_as.get_domains(), *auto_ptr<Nfa>(factory->ptr_empty()));
}

RegularRelation RegularRelation::universal(const RegularRelation& with_domain_as)
{
	auto_ptr<Nfa::Factory> factory(with_domain_as.get_automaton().ptr_factory());

	return RegularRelation(with_domain_as.get_domains(), *auto_ptr<Nfa>(factory->ptr_universal()));
}

RegularRelation::RegularRelation(const Domains &ds, const Nfa& a):
	SpecT(ds, a)
{}

gbdd::Space* RegularRelation::get_space() const
{
	return get_automaton().get_space();
}

const Nfa& RegularRelation::get_automaton() const
{
	return dynamic_cast<const Nfa&>(get_bdd_based());
}

BddRelation RegularRelation::edge_between(StateSet q, StateSet r) const
{
	return BddRelation(get_domains(), get_automaton().edge_between(q, r).normalized_bdd());
}

BddRelation RegularRelation::edge_between(State q, State r) const
{
	return BddRelation(get_domains(), get_automaton().edge_between(q, r).normalized_bdd());
}


RegularSet::RegularSet(const Domain &d, const Nfa& a):
	ViewT(d, a)
{};

RegularSet RegularSet::empty(const RegularSet& with_domain_as)
{
	auto_ptr<Nfa::Factory> factory(with_domain_as.get_automaton().ptr_factory());

	return RegularSet(with_domain_as.get_domain(), *auto_ptr<Nfa>(factory->ptr_empty()));
}

RegularSet RegularSet::universal(const RegularSet& with_domain_as)
{
	auto_ptr<Nfa::Factory> factory(with_domain_as.get_automaton().ptr_factory());

	return RegularSet(with_domain_as.get_domain(), *auto_ptr<Nfa>(factory->ptr_universal()));
}

BinaryRegularRelation RegularRelation::copy(const Domain& d1,
					    const Domain& d2,
					    const RegularSet& s)
{
	gbdd::BddEquivalenceRelation id = 
		gbdd::BddEquivalenceRelation::identity(s.get_space(), d1, d2);	

	auto_ptr<Nfa::Factory> factory(s.get_automaton().ptr_factory());
	auto_ptr<Nfa> copy_nfa(factory->ptr_empty());

	{
		gautomata::State q = copy_nfa->add_state(true, true);
		
		copy_nfa->add_edge(q, Set(Domain::infinite(), id.get_bdd()), q);
	}

	BinaryRegularRelation copy_all(d1, d2, *copy_nfa);

	return BinaryRegularRelation(d1, d2, copy_all.restrict_range(s));
}

}
