/*
 * automaton.cc: 
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

#include "automaton.h"

namespace gautomata
{

SymbolSet::SymbolSet()
{}

SymbolSet::SymbolSet(const Domain&d, Bdd sym_bdd):
	BddSet(d, sym_bdd)
{
}

SymbolSet::SymbolSet(const Domain&d, const BddSet& s):
	BddSet(d, s)
{
}
		
SymbolSet::SymbolSet(const BddSet& s):
	BddSet(s)
{}

SymbolSet::SymbolSet(const Bdd& b):
	BddSet(Domain::infinite(), b)
{}

Bdd SymbolSet::normalized_bdd() const
{
	return BddSet(Domain::infinite(), *this).get_bdd(); 
}

}
