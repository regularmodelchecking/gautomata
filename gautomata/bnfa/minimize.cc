/*
 * minimize.cc: 
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
 *    Marcus Nilsson (marcusn@docs.uu.se)
 */

#include "bnfa.h"
#include <iostream>

#define max(a, b) ((a > b) ? a : b)

#undef BISIM_EQREL

namespace gautomata
{

using namespace gbdd;

BinaryRelation BNfa::bisim() const
{
	BNfa a = *this;

#ifndef BISIM_EQREL
	// This algorithm maintains a current partitioning function p : Q -> P, where P is a set of partition names. In
	// each iteration, the function
	//
	// f(q) = {(p(q),a,p(q')):q in Q,a in Sigma,delta(q,a,q')}
	//
	// is calculated. Two states q,q' are placed in the same partition iff f(q) = f'(q)
	//
	// There is a problem when a state has no outgoing edges, then the set f(q) will always be empty. Thus,
	// we require all non-productive states to have outgoing edges. Otherwise, they will be mixed up with
	// states with the post-language being the empty string

	StateSet Dead = a.states() - a.states_productive();
	
	a.add_edge(Dead, a.alphabet(), Dead);

	vector<StateSet> partition;
	partition.push_back(a.states_accepting());
	partition.push_back(a.states() - a.states_accepting());

	vector<StateSet> prev_partition;

	/* We want to place the alphabet in the middle between the source states
	 * and the destination states. By taking the highest variable in the BDD
	 * representing the transition relation, we get a finite domain of the
	 * alphabet which is equivalent to the infinite domain.
	 */

	Relation transitions(a._transitions.get_domain(0) *
			     Domain(base_alphabet, max(base_alphabet, a._transitions.get_bdd().highest_var()) + 1 - base_alphabet) *
			     a._transitions.get_domain(2),
			     a._transitions.get_bdd());

	Bdd old_states = StateSet(transitions.get_domain(0), a.states()).get_bdd();

	do
	{
		prev_partition = partition;
		const Var base_pname1 = base_source + (base_dest - base_source) / 2;

		Domain dom_pname1(base_pname1, transitions.get_domain(0).size());
		Domain dom_pname2(base_extra, dom_pname1.size());

		BinaryRelation part_names1(transitions.get_domain(0), dom_pname1, BinaryRelation(Relation::enumeration(partition, dom_pname1)));
		BinaryRelation part_names2(transitions.get_domain(2), dom_pname2, part_names1);

		Bdd part_names = part_names1.get_bdd() & part_names2.get_bdd();

		/* Substitute successors with partition names */

		Bdd p = (transitions.get_bdd() & part_names).project(transitions.get_domain(2));

		Domains pr_doms;
		/* Create new partition */
		partition = vector<StateSet>();

		hash_set<Bdd> target_sets = p.with_geq_var(base_pname1);
		hash_set<Bdd>::const_iterator i;
		for(i = target_sets.begin();i != target_sets.end();++i)
		{
			StateSet part(transitions.get_domain(0),
				      p.with_image_geq_var(*i, base_pname1) & old_states);

			if(!(part.is_false())) partition.push_back(part);
		}
	} while(partition.size() != prev_partition.size());

	BinaryRelation renaming(Relation::enumeration(partition, Domain(base_alphabet, Bdd::n_vars_needed(partition.size()))));

	return renaming;
#else
	// Add dummy states to make n_states power of 2
	
	unsigned int n_vars_states = Bdd::n_vars_needed(a.n_states);
	unsigned int n_states = 1 << n_vars_states;

	a.increase_to_n_states(n_states, false, false);

	Bdd::VarPool pool;

	Domains dom_states_1 =
		pool.alloc_interleaved(n_vars_states, 2);
	Domains dom_states_2 =
		pool.alloc_interleaved(n_vars_states, 2);
	Domain dom_q1 = dom_states_1[0];
	Domain dom_q2 = dom_states_2[0];
	Domain dom_r1 = dom_states_1[1];
	Domain dom_r2 = dom_states_2[1];

	Domain dom_sym = map_transitions_alphabet(Domain::infinite());
//  	(void)pool.alloc(dom_sym);

	Relation not_trans1 =
		!Relation(dom_q1, dom_sym, dom_r1, a._transitions);

        Relation trans2 = Relation(dom_q2, dom_sym, dom_r2, a._transitions);


	StateSet F = a.states_accepting();
	StateSet notF = a.states() - F;

	EquivalenceRelation partition =
		BinaryRelation::cross_product(dom_r1, dom_r2, F, F) |
		BinaryRelation::cross_product(dom_r1, dom_r2, notF, notF);

	EquivalenceRelation prev_partition = partition;

	do
	{
		prev_partition = partition;

		// First we build simulates(q1, q2) which states that
		// q2 can simulate q1 in the sense that if q2 makes a
		// transition, then q1 can also make the transition such
		// that the target states are in the same partition.

		Bdd bdd_simulates =
			(Bdd)not_trans1 | ((Bdd)trans2 & (Bdd)partition);

		bdd_simulates = 
			!(bdd_simulates.project(dom_r2));

		bdd_simulates =
			!(bdd_simulates.project(dom_sym|dom_r1));


		BinaryRelation simulates(dom_q1, dom_q2, bdd_simulates);

		// Build bisimulates(q1, q2) by requiring both directions


		BinaryRelation bisimulates = simulates & simulates.inverse();

		
		// Build new partition and rename

		partition = BinaryRelation(dom_r1, dom_r2,
					   partition & bisimulates);
	} while (prev_partition != partition);

	vector<Set> state_quotient = partition.quotient(states());
	
	BinaryRelation renaming = Relation::enumeration(state_quotient);

	return renaming;
#endif
}
	

BNfa BNfa::minimize() const
{
	if (n_states == 0) return *this;

	BNfa res = *this;
	
	BinaryRelation renaming = res.bisim();

	res.set_n_states(renaming.image().size());

	Domain new_domain = res.states().get_domain();
	Domains new_transitions_domains =
		res.get_transitions_domains();

	res._starting = StateSet(new_domain, res._starting.compose(renaming));
	res._accepting = StateSet(new_domain, res._accepting.compose(renaming));
	res._transitions = Relation(new_transitions_domains, res._transitions.compose(0, renaming).compose(2, renaming));

	res.is_necessarily_complete_deterministic = is_necessarily_complete_deterministic;

	return res;
}

}
