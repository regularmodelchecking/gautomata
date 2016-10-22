/*
 * measure-nfa.cc: 
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

#include <gautomata/gautomata.h>
#include <time.h>
#include <iostream>

using namespace gautomata;
using gbdd::Space;

static Space* space = Space::create_default();
static clock_t timer;

static void measure_begin()
{
	timer = clock();
}

static float measure_end()
{
	return float(clock() - timer) / CLOCKS_PER_SEC;
}

static vector<RefNfa> create_automata(const Nfa::Factory& factory,
				      unsigned int n_automata,
				      unsigned int n_states, 
				      unsigned int n_vars,
				      float edge_density,
				      float symbol_density)
{
	vector<RefNfa> res;

	for (unsigned int i = 0;i < n_automata;++i)
	{
		res.push_back(factory.ptr_random(n_states, n_vars, edge_density, symbol_density));
	}
	
	return res;
}
		


static void measure(const Nfa::Factory& factory, const vector<RefNfa>& automata)
{
	RefNfa nfa1 = factory.ptr_clone(automata[0]);
	RefNfa nfa2 = factory.ptr_clone(automata[1]);

	measure_begin();
	RefNfa res = nfa1 & nfa2;
	float time_product = measure_end();

	measure_begin();
	RefNfa nfa1_det = nfa1.deterministic();
	float time_deterministic = measure_end();

	measure_begin();
	RefNfa nfa1_det_min = nfa1_det.minimize();
	float time_minimize = measure_end();

	cout << time_product << "(" << res.states().size() << ") ";
	cout << time_deterministic << "(" << nfa1_det.states().size() << ") ";
	cout << time_minimize << "(" << nfa1_det_min.states().size() << ") ";
	cout << endl;
}
		

int main(int argc, char **argv)
{
	vector<Nfa::Factory*> factories;

	factories.push_back(new BNfa::Factory(space));
	factories.push_back(new BNfa::Factory(space));
	//	factories.push_back(new BlockNfa::Factory(space));
	//	factories.push_back(new BlockNfa::Factory(space));


	for (int n_states = 5;n_states < 100;n_states += 2)
	{
		vector<RefNfa> automata = create_automata(*(factories[0]), 2, n_states, 10, 0.2, 0.2);

		for (vector<Nfa::Factory*>::const_iterator i = factories.begin();i != factories.end();++i)
		{
			cout << n_states << ": ";
			measure(**i, automata);
		}
	}

	return 0;
}
