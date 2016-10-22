/*
 * bnfta-determinize.cc:
 *       Tree automata determinization algorithm.
 *
 * Authors:
 *       Julien d'Orso (julien.dorso@it.uu.se)
 */

#include "bnfta.h"


#define max(a, b) ((a > b) ? a : b)


namespace gautomata
{
  using namespace gbdd;


  // -- functions for internal use only ! --

  static  vector< vector<StateSet> >  combines( vector< vector<StateSet> > V)
  {
    int size = V.size();
    vector< vector<StateSet> > res;

    if(size > 0)
      {
	vector<StateSet> S = V[size-1];
	V.pop_back();
	vector< vector<StateSet> > tmp = combines(V);
	
	for(vector<StateSet>::const_iterator s = S.begin(); s != S.end(); ++s)
	  {
	    if(tmp.size() > 0)
	      {
		for(vector< vector<StateSet> >::const_iterator p = tmp.begin(); p != tmp.end(); ++p)
		  {
		    vector<StateSet> x = (*p);
		    x.push_back(*s);
		    res.push_back(x);
		  }
	      }
	    else
	      {
		res.push_back(vector<StateSet>(1, *s));
	      }
	  }
      }

    return res;
  }

   static vector< vector<StateSet> > combine_powerstates(unsigned int index,
							 unsigned int arity,
							 vector<StateSet> V)
   {
     vector< vector<StateSet> > res;
     vector<StateSet> V_truncated(V.begin(), V.begin()+index+1);
     int i, j;
     
     for(i = 0; i < arity; ++i)
       {
	 vector<vector<StateSet> > tmp;

	 for(j = 0; j < arity; ++j)
	   {
	     if(i==j)
	       tmp.push_back(vector<StateSet>(1, V[index]));
	     else
	       tmp.push_back(V_truncated);
	   }
	 
	 tmp = combines(tmp);

	 res.insert(res.end(),
		    tmp.begin(),
		    tmp.end());
       }

     return res;
   }





  static State add_powerstate(Bnfta& res,
			      Bnfta old,
			      hash_map<StateSet, State>& powerstate_to_state,
			      hash_map<State, StateSet>& state_to_powerstate,
			      StateSet new_state)
  {
    if(powerstate_to_state.find(new_state) == powerstate_to_state.end())
      {
	bool accepting = !((old.states_accepting() & new_state).is_false());

	powerstate_to_state[new_state] = res.add_state(accepting);
	state_to_powerstate[powerstate_to_state[new_state]] = new_state;
      }

    return powerstate_to_state[new_state];
  }


  static vector<SymbolSet> powerstate_successor_syms(Bnfta old,
						     unsigned int arity,
						     vector<StateSet> from)
  {
    Bdd::VarPool pool;

    Relation transitions = old.transitions(arity);

    Domains dom_src(arity);
    for(int i=0; i<arity; ++i)
      {
	dom_src[i] = transitions.get_domain(i);
      }

    Domain dom_dst = transitions.get_domain(arity+1);

    Domain dom_tot = dom_dst;
    for(int i=0; i<arity; ++i) dom_tot |= dom_src[i];

    pool.alloc(dom_tot); 

    // interleave two copies of the alphabet....
    Domain dom_a1 = Domain::infinite(128+32*arity, 2);
    Domain dom_a2 = Domain::infinite(129+32*arity, 2);

    Relation transitions_a1 = Relation((arity>0 ? 
					dom_src * dom_a1 * dom_dst :
					dom_a1 * dom_dst),
				       transitions);
    Relation transitions_a2 = Relation((arity>0 ? 
					dom_src * dom_a2 * dom_dst :
					dom_a2 * dom_dst),
				       transitions);

    for(int i = 0; i<arity; ++i)
      {
	transitions_a1 =  transitions_a1.restrict(i, from[i]).project(i);
	transitions_a2 =  transitions_a2.restrict(i, from[i]).project(i);
      }

    BinaryRelation exists_from_src_on_a1(dom_a1, dom_dst, transitions_a1.get_bdd());
    BinaryRelation exists_from_src_on_a2(dom_a2, dom_dst, transitions_a2.get_bdd());

    EquivalenceRelation a1_iff_a2(dom_a1, dom_a2, Bdd::bdd_product(exists_from_src_on_a1.get_bdd(),
							       exists_from_src_on_a2.get_bdd(),
							       Bdd::fn_iff).forall(dom_dst));

    vector<BddSet> bdd_sets = a1_iff_a2.quotient(old.alphabet());
    
    return vector<SymbolSet>(bdd_sets.begin(), bdd_sets.end());
  }




  vector<StateSet> Bnfta::find_powerstates(const Bnfta& old,
					   vector<vector<vector<StateSet> > >& tuples,
					   vector<vector<Bdd> >& p2p_sym)
  {
    unsigned int max_a = old.max_arity();
    vector<StateSet> powerstates;
    hash_set<Bdd> found_powerstates;

    
    // The first step here is to rename the variables of the transition relation of
    // each arity, so that in the end the variables for q_dest are last.
    // This naturally involves a variable reordering...
    
    
    vector<Relation> transitions(max_a+1);
    for(unsigned int k = 0; k <= max_a; ++k)
      {
	Domains old_doms = old.transitions(k).get_domains();
	Domains dom_dest_last = old_doms;
	
	assert(k < dom_dest_last.size());

	// first make the alphabet domain finite...
	dom_dest_last[k] = Domain(128+32*max_a, 
				  max(128+32*max_a,
				      old.transitions(k).get_bdd().highest_var())
				  - (127+32*max_a)
				  );

	assert(k < transitions.size());
	transitions[k] = Relation(dom_dest_last,
				  old.transitions(k));


	assert((k+1) < dom_dest_last.size());
	// now make the destination state domain be the last variables...
	dom_dest_last[k+1] = Domain(256+32*max_a,
				    (old_doms[k+1]).size());
      
	assert(k < transitions.size());
	transitions[k] = Relation(dom_dest_last,
				  transitions[k]);
      }



    // generate the first powerstates from the transitions of arity 0
    { 
      Domains old_doms = old.transitions(0).get_domains();
      Domain  dom_pow  = Domain(256+32*max_a,
			      (old_doms[1]).size());
      
      vector<Bdd> pow2pow;
      Bdd sym_pow = transitions[0].get_bdd();

      pow2pow.push_back(sym_pow);

      hash_set<Bdd> successors = sym_pow.with_geq_var(256+32*max_a);
      hash_set<Bdd>::const_iterator i;
      for(i = successors.begin(); i != successors.end(); ++i)
	{
	  powerstates.push_back(StateSet(dom_pow, *i));

	  found_powerstates.insert(*i);
	}

      p2p_sym[0] = pow2pow;
    }



    // generate new powerstates...
    for(unsigned int index = 0; index < powerstates.size(); ++index)
      {

	for(unsigned int k = 1; k <= max_a; ++k)
	  {
	    Domains old_doms = old.transitions(k).get_domains();
	    Domain  dom_pow  =  Domain(256+32*max_a,
				       (old_doms[k+1]).size()); 
	   
	    vector<vector<StateSet> > comb_pow = combine_powerstates(index, k, powerstates);

	    vector<vector<StateSet> >::const_iterator tuple;

	    for(tuple = comb_pow.begin(); tuple != comb_pow.end(); ++tuple)
	      {
		vector<StateSet> actual_tuple = *tuple;

		Domains old_doms = old.transitions(k).get_domains();
		Domain  dom_pow  = Domain(256+32*max_a,
					  (old_doms[k+1]).size());
      
		Relation tmp =  transitions[k];
		for(unsigned int x=0; x<k; ++x) tmp = tmp.restrict(x, actual_tuple[x]).project(x); 
		Bdd sym_pow = tmp.get_bdd();
		

		// fix these two lines !
		tuples[k].push_back(actual_tuple);
		p2p_sym[k].push_back(sym_pow);
		
		hash_set<Bdd> successors = sym_pow.with_geq_var(256+32*max_a);
		hash_set<Bdd>::const_iterator i;
		for(i = successors.begin(); i != successors.end(); ++i)
		  {
		    if(found_powerstates.find(*i) == found_powerstates.end()) 
		      {
			powerstates.push_back(StateSet(dom_pow, *i));
			
			found_powerstates.insert(*i);
		      }
		  }
		// fix the next line ! is it necessary ? 
		// p2p_sym.push_back(pow2pow);
		// need a line for the tuples ?
	      }
	  }
      }

    return powerstates;
  }

  static Bdd redirect_powerstates(Bdd::Var v, Bdd p,
				  hash_map<Bdd, Bdd> p_to_s)
  {
    if(!p.bdd_is_leaf() && p.bdd_var() < v)
      {
	return Bdd::var_then_else(p.get_space(), p.bdd_var(),
				  redirect_powerstates(v, p.bdd_then(), p_to_s),
				  redirect_powerstates(v, p.bdd_else(), p_to_s));
      }
    else
      {
	return p_to_s[p];
      }
  }




  vector<Relation> Bnfta::powerstates_transitions(const Bnfta& old,
						  const Bnfta& res_auto,
						  const vector<StateSet>& powerstates,
						  const vector<vector<vector<StateSet> > >& tuples,
						  const vector<vector<Bdd> >& p2p_sym)
  {
    vector<Relation> res;
    unsigned int max_a = old.max_arity();
    
    hash_map<Bdd, int> powerstate_to_state;
    hash_map<Bdd, Bdd> p_to_s;
    Domain dom_pow = Domain(256+32*max_a,
			    (res_auto.get_transitions_domains(0)[1]).size());
    // map powerstates to states in the result automaton
    for(int i = 0; i < powerstates.size(); ++i)
      {
	powerstate_to_state[powerstates[i].get_bdd()] = i;
	p_to_s[powerstates[i].get_bdd()] = (Bdd::value(res_auto.get_space(), dom_pow, i));
      }
      
	
    for(unsigned int k = 0; k <= max_a; ++k)
      {
	Domains domains =  res_auto.get_transitions_domains(k);
	domains[k] = Domain(128+32*max_a, 
			    max(128+32*max_a,
				old.transitions(k).get_bdd().highest_var())
			    - (127+32*max_a)
			    );
	domains[k+1] = dom_pow;

	Bdd trans(res_auto.get_space(), false);

	// construct the transition relation
	if(k == 0)
	  {
	    trans =  p2p_sym[k][0];
	  }
	else 
	  {
	    for(int t = 0; t < tuples[k].size(); ++t)
	      {
		vector<StateSet> tuple = tuples[k][t];
		
		Bdd temp(res_auto.get_space(), true);
		for(int j = 0; j < k; ++j)
		  temp &= (Bdd::value(res_auto.get_space(), domains[j], powerstate_to_state[tuple[j].get_bdd()]));
		
		trans |= (temp & p2p_sym[k][t]);
	      }
	  }
	
	// if the original automaton was complete, we don't get the empty powerstate...
	if(p_to_s.find(Bdd(res_auto.get_space(), false)) == p_to_s.end())
	  {
	    p_to_s[Bdd(res_auto.get_space(), false)] = Bdd(res_auto.get_space(), false);
	  }
	
	Relation tmp = Relation(domains, redirect_powerstates(256+32*max_a, trans, p_to_s));
	for(int j = 0; j < k; ++j)
	  tmp =  tmp.restrict(j, res_auto.states());
	
	res.push_back(tmp);
      }


    return res;
  }



#undef  DETERMINIZE_OLD_VERSION
#define DETERMINIZE_NEW_VERSION


  /*
   * determinize():
   *      Returns an equivalent automaton that is deterministic.
   */
  Bnfta  Bnfta::determinize() const
  {
    if(is_deterministic) 
      return *this;

#ifdef DETERMINIZE_NEW_VERSION
    unsigned int max_a = max_arity();
    vector<vector<vector<StateSet> > > tuples(max_a+1);
    vector<vector<Bdd> > p2p_sym(max_a+1);

    vector<StateSet> powerstates = find_powerstates(*this, tuples, p2p_sym);

    Bnfta res(_space, max_a);
    res.set_n_states(powerstates.size());

    vector<Relation> tr =  powerstates_transitions(*this, res, powerstates, tuples, p2p_sym);
    assert(tr.size() > max_a);
    for(int i = 0; i <= max_a; ++i)
      res._transitions[i] = Relation(res.get_transitions_domains(i), tr[i]);
    
    Relation powerstate_enum = Relation::enumeration(powerstates, res.states().get_domain());

    res._accepting = powerstate_enum.restrict(0, states_accepting()).project_on(1);

    res.is_deterministic = true;
    res.is_complete = true;

    return res;

#endif /* DETERMINIZE_NEW_VERSION */

#ifdef DETERMINIZE_OLD_VERSION


    Bnfta old = *this;

    Bnfta res(_space, old.max_arity());

    hash_map<StateSet, State> powerstate_to_state;
    hash_map<State, StateSet> state_to_powerstate;


    //    cout << "In determinize(), old.n_states="<< old.n_states << "." << endl;
    //    cout << "res.max_arity()=" << res.max_arity() << "." << endl;

    // start by the rules of arity 0...
    {
      vector<SymbolSet> syms0 = powerstate_successor_syms(old, 0, vector<StateSet>());
      
      //     cout << "syms0 has length " << syms0.size() << "." << endl;

      vector<SymbolSet>::const_iterator i = syms0.begin();
      while(i != syms0.end())
	{
	  SymbolSet on = *i;
	  StateSet power_to = old.successors(0, vector<StateSet>(), on);

	  State to = add_powerstate(res, old, powerstate_to_state,
				    state_to_powerstate, power_to);
	  res.add_edge(0, vector<State>(), on, to);
	  
	  ++i;
	  //	  cout << "Added a powerstate/arity 0" << endl;
	}
    }


    //    cout << "We're now ready to deal with other arities." << endl;
    //    cout << "res.n_states=" << res.n_states << "." << endl;

    unsigned int n_previous = 0;
    // have we reached a fixed point yet ?
    while(n_previous < res.n_states)
      { 
	//	cout << "res.n_states=" << res.n_states << "." << endl;
	//	cout << "res.states().size()=" << res.states().size() << "." << endl;
	
	n_previous = res.n_states;
	// go through all arities !
	for(unsigned int a=1; a<=res.max_arity(); ++a) 
	  {
	    // take all tuples (q1, ..., qa) 
	    vector<vector<State> > cmb = Nfta::combine_states(Nfta::combinations(a, res.states()));
	    
	    //    cout << "a=" << a << ", cmb.size()=" << cmb.size() << "." << endl;

	    for(vector<vector<State> >::const_iterator from = cmb.begin(); from != cmb.end(); ++from)
	      {
		vector<State> vfrom = *from;
		vector<StateSet> power_from(a);
		for(int i=0; i<a; ++i) power_from[i] = state_to_powerstate[vfrom[i]];
	      
		vector<SymbolSet> successor_syms = powerstate_successor_syms(old,
									     a,
									     power_from);

		//	cout << "successor_syms.size()=" << successor_syms.size() << "." << endl;

		vector<SymbolSet>::const_iterator j = successor_syms.begin();
		while(j != successor_syms.end())
		  {
		    SymbolSet on = *j;
		    
		    StateSet power_to = old.successors(a, power_from, on);
		    
		    State to = add_powerstate(res, old,
					      powerstate_to_state,
					      state_to_powerstate,
					      power_to);
		    res.add_edge(a, vfrom, on, to);
		  
		    ++j;

		    //	    cout << "Added a powerstate/arity " << a << "." << endl;
		  }
	      }
	  } // for(a...)
      } // while(n_previous...)
    
    
    //    cout << "done determinize(), res.n_states="<< res.n_states << "." << endl;

    res.is_complete = true;
    res.is_deterministic = true;

    return res;

#endif /* DETERMINIZE_OLD_VERSION */

  }


} /* namespace treeAutomata   */
