/*
 * test-relations.cc: 
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

#include <gautomata/gautomata.h>
#include <iostream>

using namespace gautomata;
using gbdd::Bdd;
using gbdd::Space;
using gbdd::Relation;
using gbdd::Set;

static Space* space = Space::create_default();


static bool test_rename()
{
	Domain dom(0, 4);
	Set v0 = Set(dom, Bdd::var_true(space, 0));
	Set v1 = Set(dom, Bdd::var_true(space, 1));
	Set v2 = Set(dom, Bdd::var_true(space, 2));
	Set v3 = Set(dom, Bdd::var_true(space, 3));

	BNfa nfa0(space);
	{
		State q0 = nfa0.add_state(false, true);
		State q1 = nfa0.add_state(true);
		
		nfa0.add_edge(q0, v0, q1);
	}

	BNfa nfa2(space);
	{
		State q0 = nfa2.add_state(false, true);
		State q1 = nfa2.add_state(true);
		
		nfa2.add_edge(q0, v2, q1);
	}

	Domains domains0 = Domain(0, 1) * Domain(1, 1);
	Domains domains2 = Domain(2, 1) * Domain(3, 1);

	RegularRelation rel0(domains0, nfa0);
	RegularRelation rel2(domains2, nfa2);


	return
		(nfa0 != nfa2) &&
		(rel0 == rel2) &&
		RegularRelation(domains2, rel0).get_automaton() == static_cast<const Nfa&>(nfa2);
}	

static bool test_composition()
{
	Domain dom(0, 4);

	Domain dom1(0,2);
	Domain dom2(2,2);

	Bdd dom1_v0 = Bdd::value(space, dom1, 0);
	Bdd dom1_v1 = Bdd::value(space, dom1, 1);
	Bdd dom1_v2 = Bdd::value(space, dom1, 2);
	Bdd dom1_v3 = Bdd::value(space, dom1, 3);

	Bdd dom2_v0 = Bdd::value(space, dom2, 0);
	Bdd dom2_v1 = Bdd::value(space, dom2, 1);
	Bdd dom2_v2 = Bdd::value(space, dom2, 2);
	Bdd dom2_v3 = Bdd::value(space, dom2, 3);

	BNfa nfa01(space);
	{
		State q0 = nfa01.add_state(false, true);
		State q1 = nfa01.add_state(true);
		
		nfa01.add_edge(q0, Set(dom, dom1_v0 & dom2_v1), q1);
	}

	BNfa nfa12(space);
	{
		State q0 = nfa12.add_state(false, true);
		State q1 = nfa12.add_state(true);
		
		nfa12.add_edge(q0, Set(dom, dom1_v1 & dom2_v2), q1);
	}

	BNfa nfa02(space);
	{
		State q0 = nfa02.add_state(false, true);
		State q1 = nfa02.add_state(true);
		
		nfa02.add_edge(q0, Set(dom, dom1_v0 & dom2_v2), q1);
	}

	Domains domains = dom1 * dom2;

	RegularRelation rel01(domains, nfa01);
	RegularRelation rel12(domains, nfa12);

	return nfa02 == BNfa(rel01.compose(1, rel12).get_automaton());
}	


int main(int argc, char **argv)
{
	struct
	{
		const char *name;
		bool (*test_f)(void);
	}
	tests[] =
	{
		{"Renaming", test_rename},
		{"Composition", test_composition}
	};

	int i;

	for (i = 0;i < sizeof(tests) / sizeof(tests[0]);i++)
	{
		cout << tests[i].name << "...";
		cout.flush();

		cout << (tests[i].test_f() ? "Ok" : "Fail");

		cout << endl;
	}

	return 0;
}
