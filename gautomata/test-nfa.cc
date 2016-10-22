/*
 * test-bnfa.cc: 
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
using gbdd::Domain;

static Space* space = Space::create_default();

static bool test_negate(Nfa::Factory& factory)
{
        Set alphabet = Set(Domain(0, 2), Bdd(space, true));

	SymbolSet s_0 = Set(alphabet, 0);
	SymbolSet s_1 = Set(alphabet, 1);
	SymbolSet s_2 = Set(alphabet, 2);
	SymbolSet s_3 = Set(alphabet, 3);

	RefNfa nfa0(factory.ptr_empty());
	{
		State q0 = nfa0.add_state(false, true);
		State q1 = nfa0.add_state(true);
		
		nfa0.add_edge(q0, s_0, q1);
	}

	RefNfa nfa1(factory.ptr_empty());
	{
		State q0 = nfa1.add_state(false, true);
		State q1 = nfa1.add_state(true);
		
		nfa1.add_edge(q0, s_1, q1);
	}

	RefNfa nfa01 = (nfa0 * nfa1);
	
	return (!nfa01 == (RefNfa(factory.ptr_universal()) - nfa01)) &&
		(!(nfa01.deterministic()) == (RefNfa(factory.ptr_universal()) - nfa01));
}

static bool test_ops(Nfa::Factory& factory)
{
        Set alphabet = Set(Domain(0, 2), Bdd(space, true));

	SymbolSet s_0 = Set(alphabet, 0);
	SymbolSet s_1 = Set(alphabet, 1);
	SymbolSet s_2 = Set(alphabet, 2);
	SymbolSet s_3 = Set(alphabet, 3);

	RefNfa nfa0(factory.ptr_empty());
	{
		State q0 = nfa0.add_state(false, true);
		State q1 = nfa0.add_state(true,false);
		
		nfa0.add_edge(q0, s_0, q1);
	}

	RefNfa nfa1(factory.ptr_empty());
	{
		State q0 = nfa1.add_state(false, true);
		State q1 = nfa1.add_state(true,false);
		
		nfa1.add_edge(q0, s_1, q1);
	}

	RefNfa nfa2(factory.ptr_empty());
	{
		State q0 = nfa2.add_state(false, true);
		State q1 = nfa2.add_state(true,false);
		
		nfa2.add_edge(q0, s_2, q1);
	}

	RefNfa nfa3(factory.ptr_empty());
	{
		State q0 = nfa3.add_state(false, true);
		State q1 = nfa3.add_state(true,false);
		
		nfa3.add_edge(q0, s_3, q1);
	}

	RefNfa nfa01 = nfa0 * nfa1;

	RefNfa nfa23 = nfa2 * nfa3;

	
	return 
		(nfa01.kleene() | nfa23.kleene()).kleene()
		==
		(nfa01 | nfa23).kleene();
}

static bool test_rename(Nfa::Factory& factory)
{
	Domain dom(0, 4);
	Set v0 = Set(dom, Bdd::var_true(space, 0));
	Set v1 = Set(dom, Bdd::var_true(space, 1));
	Set v2 = Set(dom, Bdd::var_true(space, 2));
	Set v3 = Set(dom, Bdd::var_true(space, 3));

	RefNfa nfa0(factory.ptr_empty());
	{
		State q0 = nfa0.add_state(false, true);
		State q1 = nfa0.add_state(true);
		
		nfa0.add_edge(q0, v0, q1);
	}

	RefNfa nfa3(factory.ptr_empty());
	{
		State q0 = nfa3.add_state(false, true);
		State q1 = nfa3.add_state(true, false);
		
		nfa3.add_edge(q0, v3, q1);
	}

	return nfa0.rename(Domain(0,1), Domain(3, 1)) == nfa3;
}	

bool test_minimization(Nfa::Factory& factory)
{
	Domain dom(0, 4);
	Set v0 = Set(dom, Bdd::var_true(space, 0));
	Set v1 = Set(dom, Bdd::var_true(space, 1));
	Set v2 = Set(dom, Bdd::var_true(space, 2));
	Set v3 = Set(dom, Bdd::var_true(space, 3));

	RefNfa nfa0(factory.ptr_empty());
	{
		State q0 = nfa0.add_state(false, true);
		State q1 = nfa0.add_state(false);
		State q2 = nfa0.add_state(true);
		State q3 = nfa0.add_state(true);
		State q4 = nfa0.add_state(true);
		
		nfa0.add_edge(q0, v0, q1);
		nfa0.add_edge(q1, v1, q2);
		nfa0.add_edge(q2, v3, q3);
		nfa0.add_edge(q3, v3, q4);
		nfa0.add_edge(q4, v3, q2);
	}

	RefNfa nfa1(factory.ptr_empty());
	{
		State q0 = nfa1.add_state(false, true);
		State q1 = nfa1.add_state(false);
		State q2 = nfa1.add_state(true);
		
		nfa1.add_edge(q0, v0, q1);
		nfa1.add_edge(q1, v1, q2);
		nfa1.add_edge(q2, v3, q2);
	}

	RefNfa nfa0_min = nfa0.minimize();

	return nfa0_min == nfa1;
}
	
bool test_random(Nfa::Factory& factory)
{
	RefNfa nfa0(factory.ptr_random(10, 5));

	return true;
}

bool test_simulation(Nfa::Factory& factory)
{
        Set alphabet = Set(Domain(0, 2), Bdd(space, true));

	SymbolSet s_0 = Set(alphabet, 0);
	SymbolSet s_1 = Set(alphabet, 1);
	SymbolSet s_2 = Set(alphabet, 2);
	SymbolSet s_3 = Set(alphabet, 3);

	RefNfa nfa0(factory.ptr_empty());
	State nfa0_q0 = nfa0.add_state(false, true);
	State nfa0_q1 = nfa0.add_state(false);
	State nfa0_q2 = nfa0.add_state(false);
	State nfa0_q3 = nfa0.add_state(true);
		
	nfa0.add_edge(nfa0_q0, s_0, nfa0_q1);
	nfa0.add_edge(nfa0_q0, s_0, nfa0_q2);
	nfa0.add_edge(nfa0_q1, s_1, nfa0_q3);
	nfa0.add_edge(nfa0_q2, s_2, nfa0_q3);

	RefNfa nfa1(factory.ptr_empty());
	State nfa1_q0 = nfa1.add_state(false, false);
	State nfa1_q1 = nfa1.add_state(false);
	State nfa1_q2 = nfa1.add_state(true);
		
	nfa1.add_edge(nfa1_q0, s_0, nfa1_q1);
	nfa1.add_edge(nfa1_q1, s_1|s_2, nfa1_q2);

	set<Nfa::StatePair> forward_sim = nfa0.find_simulation_forward(nfa1);
#if 0
	for (set<Nfa::StatePair>::const_iterator i = forward_sim.begin();i != forward_sim.end();++i)
	{
		cout << "(" << i->first << "," << i->second << ")";
	}
#endif

	set<Nfa::StatePair> answer;
	answer.insert(Nfa::StatePair(nfa0_q0, nfa1_q0));
	answer.insert(Nfa::StatePair(nfa0_q1, nfa1_q1));
	answer.insert(Nfa::StatePair(nfa0_q2, nfa1_q1));
	answer.insert(Nfa::StatePair(nfa0_q3, nfa1_q2));

	return forward_sim == answer;
}



int main(int argc, char **argv)
{
	struct
	{
		const char *name;
		bool (*test_f)(Nfa::Factory& factory);
	}
	tests[] =
	{
		{"Operations", test_ops},
		{"Negate", test_negate},
		{"Renaming", test_rename},
		{"Minimization", test_minimization},
		{"Random", test_random},
		{"Simulation", test_simulation}
	};

	int i;
	auto_ptr<Nfa::Factory> ptr_factory(new MNfa::Factory(space));

	for (i = 0;i < sizeof(tests) / sizeof(tests[0]);i++)
	{
		cout << tests[i].name << "...";
		cout.flush();

		cout << (tests[i].test_f(*ptr_factory) ? "Ok" : "Fail");

		cout << endl;
	}



	return 0;



}
