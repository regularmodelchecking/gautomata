/*
 * regular-relation.h: 
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

#ifndef REGULAR_RELATION_H
#define REGULAR_RELATION_H

#include <gautomata/refnfa/refnfa.h>

namespace gautomata
{
	using gbdd::StructureRelation;
	using gbdd::BddRelation;
	using gbdd::Domains;

	class RegularSet;
	class RegularRelation;
	typedef gbdd::StructureBinaryView<Nfa, RegularRelation, RegularSet> BinaryRegularRelation;

	class RegularRelation : public gbdd::SpecializedRelation<Nfa, RegularRelation, RegularSet>
	{
		typedef gbdd::SpecializedRelation<Nfa, RegularRelation, RegularSet> SpecT;
	public:
		RegularRelation() {}
		RegularRelation(const StructureRelation& r):SpecT(r) {};
		RegularRelation(const Domains &ds, const Nfa& a);
		RegularRelation(const Domains &ds, const RegularRelation& r):SpecT(ds, r) {};
		
		gbdd::Space* get_space() const;
		const Nfa& get_automaton() const;

		BddRelation edge_between(StateSet q, StateSet r) const;
		BddRelation edge_between(State q, State r) const;

		static BinaryRegularRelation copy(const Domain& d1,
						  const Domain& d2,
						  const RegularSet& s);

		static RegularRelation empty(const RegularRelation& with_domain_as);
		static RegularRelation universal(const RegularRelation& with_domain_as);
	};

	class RegularSet : public gbdd::StructureSetView<gautomata::Nfa, gautomata::RegularRelation, gautomata::RegularSet>
	{
		typedef gbdd::StructureSetView<Nfa, RegularRelation, RegularSet> ViewT;
		RegularSet(const RegularRelation &r);
	public:
		RegularSet() {}
		RegularSet(const StructureRelation& r):ViewT(r) {};
		RegularSet(const Domain &d, const Nfa& a);
		RegularSet(const Domain &d, const RegularSet& r):ViewT(d, r) {};

		static RegularSet empty(const RegularSet& with_domain_as);
		static RegularSet universal(const RegularSet& with_domain_as);
	};				    
}	

#endif /* REGULAR_RELATION_H */
