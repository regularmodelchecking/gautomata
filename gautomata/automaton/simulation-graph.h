/*
 * simulation-graph.h: 
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

#ifndef GAUTOMATA_SIMULATION_GRAPH_H
#define GAUTOMATA_SIMULATION_GRAPH_H

#include <gautomata/automaton/word-automaton.h>
#include <gbdd/gbdd.h>

namespace gautomata
{
	using namespace std;

	class SimulationGraph
	{
		typedef gbdd::Bdd Bdd;
		typedef pair<State, State> StatePair;
	public:
		class Node
		{
			State q1;
			State q2;
			Bdd on;

			friend class SimulationGraph;
			
			Node(State q1, State q2, Bdd on);

			State main_get_source() const;
			State main_get_destination() const;
			State challenge_get_source() const;
			State challenge_get_destination() const;
			Bdd challenge_get_symbols() const;
		public:
			bool is_main() const;
			bool is_challenge() const;


			bool operator==(const Node& n2) const;

			StatePair simulation_pair() const;

			unsigned int hash() const;

			friend ostream& operator<<(ostream &s, const Node &n);
		};
		
		struct node_hash
		{
			size_t operator()(const Node& n) const
				{
					return n.hash();
				}
		};

		class NodeSet : private hash_set<Node, node_hash>
		{

			typedef hash_set<Node, node_hash> SetT;
		public:

			typedef SetT::const_iterator const_iterator;

		        const_iterator begin() const { return SetT::begin(); }
		        const_iterator end() const { return SetT::end(); }

			pair<const_iterator, bool> insert(const Node& n);
			void erase(const Node& n) { SetT::erase(n); }
			bool member(const Node& n) const;
			bool is_empty() const;

			set<StatePair> simulation() const;

			NodeSet operator-(const NodeSet& s2) const;

			bool operator==(const NodeSet& s2) const;
			bool operator<=(const NodeSet& s2) const;
			bool operator>=(const NodeSet& s2) const;

			friend ostream& operator<<(ostream &s, const NodeSet &s);
		};

	private:
/**
 * Common domain for all symbolset in nodes, needed for hashing
 * 
 */

		gbdd::Domain symbol_domain;

/**
 * We keep a reference to the automata to represent the graph
 * 
 */

		const WordAutomaton& a1;
		const WordAutomaton& a2;

		StateSet a1_states;
		StateSet a2_states;

	public:
		SimulationGraph(const WordAutomaton& a1,
				const WordAutomaton& a2);

		SimulationGraph(const WordAutomaton& a1,
				const WordAutomaton& a2,
				const StateSet& a1_states,
				const StateSet& a2_states);

		class Successor
		{
			Bdd edge;
			Node node;
		public:
			Successor(Bdd edge, Node node) : edge(edge), node(node) {}

			const Bdd& get_edge() const { return edge; }
			const Node& get_node() const { return node; }
		};


		vector<Successor> find_successors(Node n) const;

		Node main_node(State q1, State q2) const;
		
		NodeSet all_nodes() const;

		NodeSet simulation_iteration(const NodeSet& s) const;

		NodeSet simulation_fixpoint(const NodeSet& starting) const;
		NodeSet simulation_fixpoint() const;

	};
}




#endif /* GAUTOMATA_SIMULATION_GRAPH_H */
