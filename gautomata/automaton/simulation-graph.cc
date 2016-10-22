/*
 * simulation-graph.cc: 
 *
 * Copyright (C) 2003 Marcus Nilsson (marcusn@docs.uu.se)
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

#include "simulation-graph.h"
#include <iostream>

namespace gautomata
{

SimulationGraph::Node::Node(State q1, State q2, Bdd on):
	q1(q1),
	q2(q2),
	on(on)
{}

unsigned int SimulationGraph::Node::hash() const
{
	Sgi::hash<Bdd> hfunc;
	return hfunc(on) * q1 * q2;
}

State SimulationGraph::Node::main_get_source() const
{
	assert(is_main());
	return q1;
}

State SimulationGraph::Node::main_get_destination() const
{
	assert(is_main());
	return q2;
}

State SimulationGraph::Node::challenge_get_source() const
{
	assert(is_challenge());
	return q1;
}

State SimulationGraph::Node::challenge_get_destination() const
{
	assert(is_challenge());
	return q2;
}

gbdd::Bdd SimulationGraph::Node::challenge_get_symbols() const
{
	assert(is_challenge());
	return on;
}

SimulationGraph::StatePair SimulationGraph::Node::simulation_pair() const
{
	assert(is_main());

	return StatePair(q1, q2);
}

bool SimulationGraph::Node::is_main() const
{
	return on.is_false();
}

bool SimulationGraph::Node::is_challenge() const
{
	return !is_main();
}

bool SimulationGraph::Node::operator==(const Node& n2) const
{
	return q1 == n2.q1 && q2 == n2.q2 && on == n2.on;
}

pair<SimulationGraph::NodeSet::const_iterator, bool> SimulationGraph::NodeSet::insert(const Node& n)
{
	return SetT::insert(n);
}

bool SimulationGraph::NodeSet::member(const Node& n) const
{
	return find(n) != end();
}

bool SimulationGraph::NodeSet::is_empty() const
{
	return begin() == end();
}

SimulationGraph::NodeSet SimulationGraph::NodeSet::operator-(const NodeSet& s2) const
{

	NodeSet res;

	for (const_iterator i = begin();i != end();++i)
	{
		if (!s2.member(*i)) res.insert(*i);
	}

	return res;
}

bool SimulationGraph::NodeSet::operator==(const NodeSet& s2) const
{
	const NodeSet& s1 = *this;

	return (s1-s2).is_empty() && (s2-s1).is_empty();
}

bool SimulationGraph::NodeSet::operator<=(const NodeSet& s2) const
{
	return (*this - s2).is_empty();
}

bool SimulationGraph::NodeSet::operator>=(const NodeSet& s2) const
{
	return (s2 - *this).is_empty();
}


set<SimulationGraph::StatePair> SimulationGraph::NodeSet::simulation() const
{
	set<StatePair> res;

	for (const_iterator i = begin();i != end();++i)
	{
		if (i->is_main()) res.insert(i->simulation_pair());
	}

	return res;
}

SimulationGraph::SimulationGraph(const WordAutomaton& a1,
				 const WordAutomaton& a2,
				 const StateSet& a1_states,
				 const StateSet& a2_states):
	symbol_domain(a1.alphabet().get_domain()),
	a1(a1),
	a2(a2),
	a1_states(a1_states),
	a2_states(a2_states)
{}

SimulationGraph::SimulationGraph(const WordAutomaton& a1,
				 const WordAutomaton& a2):
	symbol_domain(a1.alphabet().get_domain()),
	a1(a1),
	a2(a2),
	a1_states(a1.states()),
	a2_states(a2.states())
{}

			
vector<SimulationGraph::Successor> SimulationGraph::find_successors(Node n) const
{
	vector<Successor> res;
	StateSet Q1 = a1_states;
	StateSet Q2 = a2_states;


	if (n.is_main())
	{
		StateSet Succ = a1.successors(StateSet(Q1, n.main_get_source()), a1.alphabet()) & Q1;

		for (StateSet::const_iterator i = Succ.begin();i != Succ.end();++i)
		{
			SymbolSet edge(symbol_domain, a1.edge_between(n.main_get_source(), *i));

			res.push_back(Successor(edge.get_bdd(), Node(*i, n.main_get_destination(), edge.get_bdd())));
		}
	}
	else
	{
		assert(n.is_challenge());

		StateSet Succ = a2.successors(StateSet(Q2, n.challenge_get_destination()), a2.alphabet()) & Q2;

		for (StateSet::const_iterator i = Succ.begin();i != Succ.end();++i)
		{
			SymbolSet edge(symbol_domain, a2.edge_between(n.challenge_get_destination(), *i));

			res.push_back(Successor(edge.get_bdd(), Node(n.challenge_get_source(), *i, Bdd(a1.get_space(), false))));
		}
	}

	return res;
}

						
SimulationGraph::Node SimulationGraph::main_node(State q1, State q2) const
{
	return Node(q1, q2, Bdd(a1.get_space(), false));
}

SimulationGraph::NodeSet SimulationGraph::all_nodes() const
{
	NodeSet res;

	StateSet Q1 = a1_states;
	StateSet Q2 = a2_states;

	for (StateSet::const_iterator i = Q1.begin();i != Q1.end();++i)
	{
		for (StateSet::const_iterator j = Q2.begin();j != Q2.end();++j)
		{
			Node n(*i, *j, Bdd(a1.get_space(), false));
			res.insert(n);

			vector<Successor> n_succ = find_successors(n);

			for (vector<Successor>::const_iterator k = n_succ.begin();k != n_succ.end();++k)
			{
				res.insert(k->get_node());
			}
		}
	}

	return res;
}

SimulationGraph::NodeSet SimulationGraph::simulation_iteration(const NodeSet& prev) const
{
	NodeSet res;

	// Since prev is decreasing, we need only to iterate over prev and not all nodes

	for (NodeSet::const_iterator i = prev.begin();i != prev.end();++i)
	{
		Node n = *i;

		if (n.is_main())
		{
			vector<Successor> succ = find_successors(n);
			
			NodeSet succ_nodes;
			for (vector<Successor>::const_iterator j = succ.begin();j != succ.end();++j) succ_nodes.insert(j->get_node());

			if ((succ_nodes - prev).is_empty()) res.insert(n);
		}
		else
		{
			assert(n.is_challenge());

			vector<Successor> succ = find_successors(n);

			Bdd possibilities(a1.get_space(), false);

			for (vector<Successor>::const_iterator j = succ.begin();j != succ.end();++j)
			{
				if (prev.member(j->get_node()))
				{
					possibilities |= j->get_edge();
				}
			}

			if ((n.challenge_get_symbols() - possibilities).is_false()) res.insert(n);
		}
	}

	return res;
}

SimulationGraph::NodeSet SimulationGraph::simulation_fixpoint(const NodeSet& starting) const
{
	NodeSet current = starting;
	NodeSet prev;

	do
	{
		prev = current;
		current = simulation_iteration(current);
	} while (!(prev == current));

	return current;
}

SimulationGraph::NodeSet SimulationGraph::simulation_fixpoint() const
{
	return simulation_fixpoint(all_nodes());
}

ostream& operator<<(ostream &out, const SimulationGraph::Node &n)
{
	if (n.is_main())
	{
		
		out << "(" << n.main_get_source() << "," << n.main_get_destination() << ")";
	}
	else
	{
		out << "(" << n.challenge_get_source() << "," << n.challenge_get_destination() << "," << n.challenge_get_symbols() << ")";
	}

	return out;
}

ostream& operator<<(ostream &out, const SimulationGraph::NodeSet &s)
{
	out << "{";
	bool first = true;
	for (SimulationGraph::NodeSet::const_iterator i = s.begin();i != s.end();++i)
	{
		const SimulationGraph::Node& n = *i;

		if (first) first = false;
		else out << ",";

		out << n;
	}

	out << "}";

	return out;
}

				


}
