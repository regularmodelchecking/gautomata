/*
 * tree-regular-relation.cc: 
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

#include "tree-regular-relation.h"

namespace gautomata
{

TreeRegularRelation::TreeRegularRelation(const Domains &ds, const Nfta& a):
	SpecT(ds, a)
{};

TreeRegularRelation::operator const Nfta&() const
{
	return get_automaton();
}

gbdd::Space* TreeRegularRelation::get_space() const
{
	return get_automaton().get_space();
}

const Nfta& TreeRegularRelation::get_automaton() const
{
	return dynamic_cast<const Nfta&>(get_bdd_based());
}

TreeRegularSet::TreeRegularSet(const Domain &d, const Nfta& a):
	ViewT(d, a)
{};

TreeRegularSet TreeRegularSet::empty(const TreeRegularSet& with_domain_as)
{
	auto_ptr<Nfta::Factory> factory(with_domain_as.get_automaton().ptr_factory());

	return TreeRegularSet(with_domain_as.get_domain(), *auto_ptr<Nfta>(factory->ptr_universal(with_domain_as.get_automaton().max_arity())));
}

TreeRegularSet TreeRegularSet::universal(const TreeRegularSet& with_domain_as)
{
	auto_ptr<Nfta::Factory> factory(with_domain_as.get_automaton().ptr_factory());

	return TreeRegularSet(with_domain_as.get_domain(), *auto_ptr<Nfta>(factory->ptr_empty(with_domain_as.get_automaton().max_arity())));
}

}
