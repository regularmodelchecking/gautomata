/*
 * automaton.h: 
 *
 * Copyright (C) 2004 Marcus Nilsson (marcusn@it.uu.se)
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
 *    Marcus Nilsson (marcusn@it.uu.se)
 */

#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <gbdd/gbdd.h>

namespace gautomata
{
	using namespace std;

	using gbdd::Relation;
	using gbdd::Set;
	using gbdd::Bdd;
	using gbdd::BddSet;
	using gbdd::Space;
	using gbdd::Domain;
	using gbdd::BddBased;

	typedef unsigned int State;
	typedef Set StateSet;


	class SymbolSet : public BddSet
	{
	public:
		SymbolSet();
		SymbolSet(const Domain&d, Bdd sym_bdd);
		SymbolSet(const Domain&d, const BddSet& s);
		SymbolSet(const BddSet& s);
		SymbolSet(const Bdd& b);

		Bdd normalized_bdd() const;
		operator Bdd() const { return normalized_bdd(); }
	};

	class Automaton : public gbdd::BddBased
	{
	public:
		virtual SymbolSet alphabet() const = 0;
	};
}


#endif /* AUTOMATON_H */
