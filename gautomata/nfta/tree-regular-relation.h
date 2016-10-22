/*
 * tree-regular-relation.h: 
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

#ifndef TREE_REGULAR_RELATION_H
#define TREE_REGULAR_RELATION_H

#include <gautomata/refnfta/refnfta.h>

namespace gautomata
{
	class TreeRegularSet;
	class TreeRegularRelation;
	using gbdd::StructureRelation;
	using gbdd::Domains;

	typedef gbdd::StructureBinaryView<Nfta, TreeRegularRelation, TreeRegularSet> TreeBinaryRegularRelation;

	class TreeRegularRelation : public gbdd::SpecializedRelation<Nfta, TreeRegularRelation, TreeRegularSet>
	{
		typedef gbdd::SpecializedRelation<Nfta, TreeRegularRelation, TreeRegularSet> SpecT;
	public:
		TreeRegularRelation() {}
		TreeRegularRelation(const StructureRelation& r):SpecT(r) {};
		TreeRegularRelation(const Domains &ds, const Nfta& a);
		TreeRegularRelation(const Domains &ds, const TreeRegularRelation& r):SpecT(ds, r) {};
		
		operator const Nfta&() const;

		gbdd::Space* get_space() const;
		const Nfta& get_automaton() const;
	};

	class TreeRegularSet : public gbdd::StructureSetView<Nfta, TreeRegularRelation, TreeRegularSet>
	{
		typedef gbdd::StructureSetView<Nfta, TreeRegularRelation, TreeRegularSet> ViewT;
		TreeRegularSet(const TreeRegularRelation &r);
	public:
		TreeRegularSet() {}
		TreeRegularSet(const StructureRelation& r):ViewT(r) {};
		TreeRegularSet(const Domain &d, const Nfta& a);
		TreeRegularSet(const Domain &d, const TreeRegularSet& r):ViewT(d, r) {};

		static TreeRegularSet empty(const TreeRegularSet& with_domain_as);
		static TreeRegularSet universal(const TreeRegularSet& with_domain_as);
	};				    

	
		
}	

#endif /* REGULAR_RELATION_H */
