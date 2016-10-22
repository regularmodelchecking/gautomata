/*
 * bnfta-minimize.cc: 
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
 *    Julien d'Orso  (julodr@it.uu.se)
 */

#include "bnfta.h"

#define max(a, b) ((a > b) ? a : b)


namespace gautomata
{

  using namespace gbdd;

  BinaryRelation Bnfta::bisim() const
  {
    Bnfta a = *this;
    unsigned int max_a = a.max_arity();

    // Initialize partitions to hold accepting and not accepting states
    vector<StateSet> partition;
    partition.push_back(a.states_accepting());
    partition.push_back(a.states() - a.states_accepting());
    
    vector<StateSet> prev_partition;

    /* We want to place the alphabet in the middle between the source states
     * and the destination states. By taking the highest variable in the BDD
     * representing the transition relation, we get a finite domain of the
     * alphabet which is equivalent to the infinite domain.
     */

    vector<Relation> transitions(max_a+1);

    for(unsigned int k = 0; k <= max_a; ++k)
      {
	Domains my_domains = a._transitions[k].get_domains();
	my_domains[k] =  Domain(128, max(128+32*max_a, a._transitions[k].get_bdd().highest_var()) - (127+32*max_a));

	transitions[k] =  Relation(my_domains,
				   a._transitions[k].get_bdd());
      }

    
    do
      {
	prev_partition = partition;
	
	for(unsigned int k = 0; k <= max_a; ++k)
	  { // loop through the different arities.
	    for(unsigned  int j = 0; j < k; ++j)
	      { // we check on predecessors of rank j.
		Bdd old_states = StateSet(transitions[k].get_domain(j), a.states()).get_bdd();

		Domain dom_pname1(80+32*j, transitions[k].get_domain(j).size());
		Domain dom_pname2(256+32*max_a, dom_pname1.size());
	
		BinaryRelation part_names1(transitions[k].get_domain(j), dom_pname1, BinaryRelation(Relation::enumeration(partition, dom_pname1)));
		BinaryRelation part_names2(transitions[k].get_domain(k+1), dom_pname2, part_names1);
	
		Bdd part_names = part_names1.get_bdd() & part_names2.get_bdd();
		
		/* Substitute successors with partition names */
	
		Bdd p = (transitions[k].get_bdd() & part_names).project(transitions[k].get_domain(k+1));
		for(unsigned int x = 0; x < k; ++x) if(x != j) p = p.project(transitions[k].get_domain(x));		


		Domains pr_doms;
		/* Create new partition */
		partition = vector<StateSet>();
		
		hash_set<Bdd> target_sets = p.with_geq_var(80+32*j);
		hash_set<Bdd>::const_iterator i;
		for(i = target_sets.begin();i != target_sets.end();++i)
		  {
		    StateSet part(transitions[k].get_domain(j),
				  p.with_image_geq_var(*i, 80+32*j) & old_states);
		    
		    if(!(part.is_false())) partition.push_back(part);
		  }

		if(partition.size() > prev_partition.size())
		  break;
	      }

	    if(partition.size() > prev_partition.size())
	      break;
	  }

      } while(partition.size() != prev_partition.size());

    BinaryRelation renaming(Relation::enumeration(partition, Domain(128, Bdd::n_vars_needed(partition.size()))));

    return renaming;
  }
	

  Bnfta Bnfta::minimize() const
  {
    if (n_states == 0) return *this;
    
    Bnfta res = *this;
    
    BinaryRelation renaming = res.bisim();
	
    res.set_n_states(renaming.image().size());
    
    Domain new_domain = res.states().get_domain();
    
    res._accepting = StateSet(new_domain, res._accepting.compose(renaming));
 
    for(unsigned int k = 0; k <= res.max_arity(); ++k) 
      {  
	Domains new_transitions_domains = res.get_transitions_domains(k);

	for(unsigned int x = 0; x < k; ++x)
	  res._transitions[k] = res._transitions[k].compose(x, renaming);
 
	res._transitions[k] = Relation(new_transitions_domains, res._transitions[k].compose(k+1, renaming));
      }
    
    res.is_complete = is_complete;
    res.is_deterministic = is_deterministic;
    
    return res;
  }
  
}
