/*
 * nfta.cc:
 *    A default (partial) implementation for NFTA's.
 *
 * Authors:
 *    Julien d'Orso (julien.dorso@it.uu.se)
 *
 */

#include "nfta.h"
#include <assert.h>
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <sys/timeb.h>

namespace gautomata
{
  using namespace gbdd;

  // -- functions for internal use only ! --

  vector< vector<State> > Nfta::combine_states(vector<StateSet> V)
  {
    int size = V.size();
    vector< vector<State> > res;

    if(size > 0)
      {
	StateSet S = V[size-1];
	V.pop_back();
	vector< vector<State> > tmp = combine_states(V);

	for(StateSet::const_iterator s = S.begin(); s != S.end(); ++s)
	  {
	    if(tmp.size() > 0)
	      {
		for(vector< vector<State> >::const_iterator p = tmp.begin(); p != tmp.end(); ++p)
		  {
		    vector<State> x = (*p);
		    x.push_back(*s);
		    res.push_back(x);
		  }
	      }
	    else
	      {
		res.push_back(vector<State>(1, *s));
	      }
	  }
      }

    return res;
  }

  // -- internal use --
  // create a vector of length a with s everywhere...
  vector<StateSet> Nfta::combinations(unsigned int a, StateSet s)
  {
    return vector<StateSet> (a, s);
  }

  static bool with_probability(float p)
  {
    return rand() < RAND_MAX * p;
  }

  static SymbolSet edge_with_density(Space* space,
				     unsigned int n_vars,
				     float density)
  {
    Bdd p(space, true);

    for(int i = 0; i < n_vars; ++i)
      {
	if(with_probability(density))
	  {
	    if(with_probability(0.5))
	      {
		p &= Bdd::var_true(space, i);
	      }
	    else
	      {
		p &= Bdd::var_false(space, i);
	      }
	  }
      }

    return SymbolSet(Domain::infinite(), p);
  }


  // -- here we start really ! --



  hash_map<State, State> Nfta::copy_states(const Nfta& a,
					   StateSet accepting)
  {
    StateSet Q = a.states();
    StateSet F = accepting;
    hash_map<State, State> state_map;

    for(StateSet::const_iterator i = Q.begin(); i != Q.end(); ++i)
      {
	state_map[*i] = add_state(F.member(*i));
      }

    return state_map;
  }



  hash_map<State, State> Nfta::copy_states_and_transitions(const Nfta& a,
							   StateSet accepting)
  {
    hash_map<State, State> state_map = copy_states(a, accepting);
    StateSet Q = a.states();
    vector< vector<State> > Qcomb;
    unsigned int max_a = a.max_arity();


    for(unsigned int k = 0; k <= max_a; ++k)
      { // looping on the arity...
	if(k == 0)
	  { // special case: arity == 0 !
	    for(StateSet::const_iterator j = Q.begin(); j != Q.end(); ++j)
	      {
		add_edge(k,
			 vector<State>(),
			 edge_between(k, vector<State>(), *j),
			 state_map[*j]);
	      }
	  }
	else
	  { // arity > 0 !
	    Qcomb =    Nfta::combine_states( Nfta::combinations(k, Q) );

	    // loop over the different combinations stored in Qcomb...
	    for(vector< vector<State> >::const_iterator ivec = Qcomb.begin(); ivec != Qcomb.end(); ++ivec)
	      {
		vector<State> i = *ivec;
		vector<State> i_mapped(k);

		for(int x = 0; x < k; ++x)
		  i_mapped[x] = state_map[i[x]];

		for(StateSet::const_iterator j = Q.begin(); j != Q.end(); ++j)
		  {
		    add_edge(k,
			     i_mapped,
			     edge_between(k, i, *j),
			     state_map[*j]);
		  }

	      }
	  }
      } // looping on the arity...


    return state_map;
  }



  Space* Nfta::get_space() const
  {
    return states().get_space();
  }



  SymbolSet Nfta::edge_between(unsigned int arity, vector<State> q, State r) const
  {
    vector<StateSet> my_q(arity);

    for(int i = 0; i<arity; ++i)
      {
	my_q[i] = StateSet(states(), q[i]);
      }

    return edge_between(arity,
			my_q,
			StateSet(states(), r));
  }

  SymbolSet Nfta::edge_between(unsigned int arity, vector<StateSet> q, StateSet r) const
  {
    Relation tmp = transitions(arity);
    for(int i = 0; i<arity; ++i)
      tmp = tmp.restrict(i, q[i]);

    return tmp.restrict(arity+1, r).project_on(arity);
  }

  Relation Nfta::transitions(unsigned int arity) const
  {
    StateSet Q = states();
    unsigned int states_vars = Q.get_domain().size();
    // make sure we have enough room for states
    Domain dom_alphabet = alphabet().get_domain() + ((arity+1)*states_vars);

    Bdd::VarPool pool;
    pool.alloc(dom_alphabet);
    Domains doms_from(arity);
    // if arity>1, doms_from is a cross product
    for(int i=0; i<arity; ++i)
      doms_from[i] = pool.alloc(states_vars);
    Domain dom_to = pool.alloc(states_vars);

    // this domain should have an arity of (arity+2)...
    Domains tr_doms = doms_from * dom_alphabet * dom_to;

    Relation tr(tr_doms, Bdd(Q.get_space(), false));

    vector< vector<State> > Q_comb = Nfta::combine_states(vector<StateSet> (arity, Q));

    for(vector< vector<State> >::const_iterator i = Q_comb.begin(); i != Q_comb.end(); ++i)
      {
	for(StateSet::const_iterator j = Q.begin(); j != Q.end(); ++j)
	  {
	    if(arity>0)
	      {
		Bdd bdd_vi = Bdd::value(tr.get_space(), doms_from[0], (*i)[0]);
		for(int k=1; k<arity; ++k)
		  bdd_vi = bdd_vi & Bdd::value(tr.get_space(), doms_from[k], (*i)[k]);
		tr |= Relation(tr_doms,
			       (bdd_vi &
				Bdd::value(tr.get_space(), dom_to, *j) &
				SymbolSet(dom_alphabet, edge_between(arity, *i, *j))));
	      }
	    else
	      tr |= Relation(tr_doms,
			     (Bdd::value(tr.get_space(), dom_to, *j) &
			      SymbolSet(dom_alphabet, edge_between(arity, *i, *j))));
	  }
      }

    return tr;
  }
  

  // Analysis of the automaton....

 
  StateSet Nfta::successors(unsigned int arity, vector<StateSet> q, SymbolSet on) const 
  {
    Relation tmp =  transitions(arity);
    
    for(int i=0; i<arity; ++i)
      tmp = tmp.restrict(i, q[i]);
    
    return tmp.restrict(arity, on).project_on(arity+1);
  }

  StateSet Nfta::predecessors(unsigned int arity, StateSet q, SymbolSet on) const
  {
    Relation tmp_rel = transitions(arity).restrict(arity+1, q).restrict(arity, on);
    StateSet tmp(q.get_space());
    
    for(int i=0; i<arity; ++i)
      {
	tmp = tmp | tmp_rel.project_on(i);
      }
    
    return tmp;
  }

  StateSet Nfta::reachable_successors(StateSet q, SymbolSet on) const
  {
    StateSet q_prev;
    unsigned int arity;
    unsigned int ma = max_arity();
    
    do
      {
	q_prev = q;
	if(q.size() == 0)
	  {
	    q =  successors(0, vector<StateSet>(), on);
	  }
	else
	  {
	    for(arity=1; arity<=ma; ++arity)
	      q = q | successors(arity, Nfta::combinations(arity, q), on);
	  }
      } while(!(q_prev == q));
    
    return q;}
  
  StateSet Nfta::reachable_predecessors(StateSet q, SymbolSet on) const
  {
    StateSet q_prev;
    unsigned int arity;
    unsigned int ma = max_arity();
    
    do
      {
	q_prev = q;
	for(arity=1; arity<=ma; ++arity)
	  q = q | predecessors(arity, q, on);
      } while(!(q_prev == q));
    
    return q;
  }


  StateSet Nfta::states_reachable() const
  {
    return reachable_successors(StateSet(states().get_space()), alphabet());
  }
  
  StateSet Nfta::states_productive() const
  {
    return reachable_predecessors(states_accepting(), alphabet());
  }

  StateSet Nfta::states_live() const
  {
    return states_productive() & states_reachable();
  }




  // Here come the functions to add edges to a tree automaton...


  void Nfta::add_edge(unsigned int arity, vector<State> from, SymbolSet on, State to)
  {
    vector<StateSet>    vfrom(arity);
    
    for(int i=0; i<arity; ++i)
      vfrom[i] = StateSet(states(), from[i]);
      
    add_edge(arity,
	     vfrom,
	     on,
	     StateSet(states(), to));
  }

  void Nfta::add_edge(unsigned int arity, vector<StateSet> from, SymbolSet on, StateSet to)
  {
    vector<vector<State> > tmp = Nfta::combine_states(from);

    for(vector<vector<State> >::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
      for(StateSet::const_iterator j = to.begin(); j != to.end(); ++j)
	add_edge(arity, *i, on, *j);
  }

  // Method to add transitions to an automaton...

  void Nfta::add_transitions(unsigned int arity, Relation new_transitions)
  {
    // We begin with the states that appear on the left-hand
    // side of the new transitions, at the k'th rank:
    vector<StateSet> Q(arity);
    for(int k=0; k<arity; ++k)
      Q[k] = new_transitions.project_on(k);
    // List all combinations:
    vector< vector<State> > Q_comb = Nfta::combine_states(Q);
    
    for(vector< vector<State> >::const_iterator i = Q_comb.begin(); i != Q_comb.end(); ++i)
      {
	Relation Q_transitions = new_transitions;
	for(int k=0; k<arity; ++k)
	  Q_transitions = Q_transitions.restrict(k, StateSet(states(), (*i)[k]));
	StateSet R = Q_transitions.project_on(arity+1);
    
     	for(StateSet::const_iterator j = R.begin(); j != R.end(); ++j)
     	  {
     	    SymbolSet on = Q_transitions.restrict(arity+1, StateSet(states(), (*j))).project_on(arity);
    
     	    add_edge(arity, *i, on, *j);
	  }
      }
  }

  // Binary operations...
  Nfta* Nfta::ptr_product_and(const Nfta& a2) const
  {
    return ptr_product(a2, Bdd::fn_and);
  }

  Nfta* Nfta::ptr_product_or(const Nfta& a2) const
  {
    return ptr_product(a2, Bdd::fn_or);
  }

  Nfta* Nfta::ptr_product_minus(const Nfta& a2) const
  {
    return ptr_product(a2, Bdd::fn_minus);
  }


  // Unary operations....

  Nfta* Nfta::ptr_project(Domain vs) const
  {
    unsigned int max_a = max_arity();
    Nfta* res = auto_ptr<Factory>(ptr_factory())->ptr_empty(max_a);
    StateSet Q = states();
    vector< vector<State> > Qcomb;

    hash_map<State, State> state_map = res->copy_states(*this,
							states_accepting());

    for(unsigned int k = 0; k <= max_a; ++k)
      { // looping on the arity...
	if(k == 0)
	  { // special case: arity == 0 !

		for(StateSet::const_iterator j = Q.begin(); j != Q.end(); ++j)
		  {
		    SymbolSet edge = edge_between(k, vector<State>(), *j);

		    Bdd bdd_edge = SymbolSet(Domain::infinite(), edge);

		    SymbolSet projected = SymbolSet(Domain::infinite(),
						    bdd_edge.project(vs));

		    res->add_edge(k,
			     vector<State>(),
			     projected,
			     state_map[*j]);
		  }

	  }
	else
	  { // arity > 0 !
	    Qcomb =    Nfta::combine_states( Nfta::combinations(k, Q) );

	    // loop over the different combinations stored in Qcomb...
	    for(vector< vector<State> >::const_iterator ivec = Qcomb.begin(); ivec != Qcomb.end(); ++ivec)
	      {
		vector<State> i = *ivec;
		vector<State> i_mapped(k);

		for(int x = 0; x < k; ++x)
		  i_mapped[x] = state_map[i[x]];

		for(StateSet::const_iterator j = Q.begin(); j != Q.end(); ++j)
		  {
		    SymbolSet edge = edge_between(k, i, *j);

		    Bdd bdd_edge = SymbolSet(Domain::infinite(), edge);

		    SymbolSet projected = SymbolSet(Domain::infinite(),
						    bdd_edge.project(vs));

		    res->add_edge(k,
			     i_mapped,
			     projected,
			     state_map[*j]);
		  }

	      }
	  }
      } // looping on the arity...



    return res;
}

  Nfta::Var Nfta::highest_var() const
{
	assert(false);
}

Nfta::Var Nfta::lowest_var() const
{
	assert(false);
}

Nfta* Nfta::ptr_constrain_value(Var v, bool value) const
{
	unsigned int max_a = max_arity();
	Nfta* res = auto_ptr<Factory>(ptr_factory())->ptr_empty(max_a);
	StateSet Q = states();
	vector< vector<State> > Qcomb;
	hash_map<State,State> state_map = res->copy_states_and_transitions(*this,
									  states_accepting());
	for(unsigned int k = 0; k <= max_a; ++k)
	{
	 if(k == 0)
	 {
		for(StateSet::const_iterator j = Q.begin(); j != Q.end(); ++j)
		  {
		    SymbolSet edge = edge_between(k, vector<State>(), *j);

		    Bdd bdd_edge = SymbolSet(Domain::infinite(), edge);

		    SymbolSet constrained_edge(Domain::infinite(), dynamic_cast<Bdd&>(*auto_ptr<BddBased>(bdd_edge.ptr_constrain_value(v, value))));

		    res->add_edge(k, vector<State>(),constrained_edge, state_map[*j]);
		  }
	 }
	else
	 { // arity > 0 !
	    Qcomb =    Nfta::combine_states( Nfta::combinations(k, Q) );

	    // loop over the different combinations stored in Qcomb...
	    for(vector< vector<State> >::const_iterator ivec = Qcomb.begin(); ivec != Qcomb.end(); ++ivec)
	      {
		vector<State> i = *ivec;
		vector<State> i_mapped(k);

		for(int x = 0; x < k; ++x)
		  i_mapped[x] = state_map[i[x]];

		for(StateSet::const_iterator j = Q.begin(); j != Q.end(); ++j)
		  {
		    SymbolSet edge = edge_between(k, i, *j);

		    Bdd bdd_edge = SymbolSet(Domain::infinite(), edge);

		    SymbolSet constrained_edge(Domain::infinite(), dynamic_cast<Bdd&>(*auto_ptr<BddBased>(bdd_edge.ptr_constrain_value(v, value))));

		    res->add_edge(k, i_mapped,constrained_edge, state_map[*j]);
		  }

	      }
	  }
      } // looping on the arity...

      return res;
}


   BddBased* Nfta::ptr_negate() const
 {
   return ptr_negate(*this);
 }



  Nfta* Nfta::ptr_rename(VarMap map) const
  {
    unsigned int max_a = max_arity();
    Nfta* res = auto_ptr<Factory>(ptr_factory())->ptr_empty(max_a);
    StateSet Q = states();
    vector< vector<State> > Qcomb;

    hash_map<State, State> state_map = res->copy_states(*this,
							states_accepting());

    for(unsigned int k = 0; k <= max_a; ++k)
      { // looping on the arity...
	if(k == 0)
	  { // special case: arity == 0 !

		for(StateSet::const_iterator j = Q.begin(); j != Q.end(); ++j)
		  {
		    SymbolSet edge = edge_between(k, vector<State>(), *j);

		    Bdd bdd_edge = SymbolSet(Domain::infinite(), edge);

		    SymbolSet renamed = SymbolSet(Domain::infinite(), bdd_edge.rename(map));

		    res->add_edge(k,
			     vector<State>(),
			     renamed,
			     state_map[*j]);
		  }

	  }
	else
	  { // arity > 0 !
	    Qcomb =    Nfta::combine_states( Nfta::combinations(k, Q) );

	    // loop over the different combinations stored in Qcomb...
	    for(vector< vector<State> >::const_iterator ivec = Qcomb.begin(); ivec != Qcomb.end(); ++ivec)
	      {
		vector<State> i = *ivec;
		vector<State> i_mapped(k);

		for(int x = 0; x < k; ++x)
		  i_mapped[x] = state_map[i[x]];

		for(StateSet::const_iterator j = Q.begin(); j != Q.end(); ++j)
		  {
		    SymbolSet edge = edge_between(k, i, *j);

		    Bdd bdd_edge = SymbolSet(Domain::infinite(), edge);

		    SymbolSet renamed = SymbolSet(Domain::infinite(), bdd_edge.rename(map));

		    res->add_edge(k,
			     i_mapped,
			     renamed,
			     state_map[*j]);
		  }

	      }
	  }
      } // looping on the arity...



    return res;
  }
  
  Nfta* Nfta::ptr_rename(Domain vs1, Domain vs2) const
  {
    assert(vs1.size() == vs2.size());
    
    return ptr_rename(Domain::map_vars(vs1, vs2));
  }
  

  Nfta* Nfta::ptr_filter_states_live() const
  {
    return ptr_filter_states(states_live());
  }

  Nfta* Nfta::ptr_filter_states_reachable() const
  {
    return ptr_filter_states(states_reachable());
  }

  Nfta* Nfta::ptr_filter_states_productive() const
  {
    return ptr_filter_states(states_productive());
  }


  // Predicates....


  bool Nfta::is_true() const
  {
    auto_ptr<Nfta> ptr_det(ptr_complete()->ptr_determinize());
    StateSet accpt = ptr_det->states_accepting();
    StateSet rchbl = ptr_det->states_reachable();
    // check that rchbl is included in accpt.
    return (rchbl & (!accpt)).is_empty();
  }


  bool Nfta::is_false() const
  {
    return states_live().is_empty();
  }



  Nfta* Nfta::ptr_product(const gbdd::BddBased& a2, bool (*fn)(bool v1, bool v2)) const
 {
  return ptr_product(dynamic_cast<const Nfta&>(a2), fn);
 }

  bool Nfta::operator == (const Nfta& a2) const
 {
  return ptr_product(a2, Bdd::fn_iff)->is_true();
 }

  bool Nfta::operator == (const BddBased &a2) const 
  {
    return *this == dynamic_cast<const Nfta&>(a2);
  }

  bool Nfta::operator != (const Nfta& a2) const
  {
    return !(*this == a2);
  }

  bool Nfta::operator != (const BddBased &a2) const 
  {
    return *this != dynamic_cast<const Nfta&>(a2);
  }

  bool Nfta::operator <= (const Nfta& a2) const
  {
    return ptr_product(a2, Bdd::fn_implies)->is_true();
  }
  
 bool Nfta::operator <= (const BddBased &a2) const 
  {
    return *this <= dynamic_cast<const Nfta&>(a2);
  }
 
 bool Nfta::operator >= (const Nfta& a2) const
  {
    return a2 <= *this;
  }

 bool Nfta::operator >= (const BddBased &a2) const 
  {
    return *this >= dynamic_cast<const Nfta&>(a2);
  }

  bool Nfta::operator < (const Nfta& a2) const
  {
    return *this <= a2 && !(*this == a2);
  }

 bool Nfta::operator < (const BddBased &a2) const 
  {
    return *this < dynamic_cast<const Nfta&>(a2);
  }

  bool Nfta::operator > (const Nfta& a2) const
  {
    return a2 <= *this && !(*this == a2);
  }

 bool Nfta::operator > (const BddBased &a2) const 
  {
    return *this > dynamic_cast<const Nfta&>(a2);
  }
  // Generate random automaton...

  Nfta* Nfta::Factory::ptr_random(unsigned int max_arity,
					   unsigned int n_states,
					   unsigned int n_vars,
					   float edge_density,
					   float symbol_density) const
  {
    struct timeb tb;
    ftime(&tb);

    srand(time(NULL) + tb.millitm);

    Nfta* nfta = ptr_empty(max_arity);    
    Space* space = nfta->alphabet().get_space();

    for(int i = 0; i < n_states; ++i)
      {
	nfta->add_state(with_probability(0.5));
      }

    StateSet Q = nfta->states();
    
    for(unsigned int n=0; n <= max_arity; ++n)
      {
	
	if(n>0)
	  {
	    vector<StateSet> Qvec = Nfta::combinations(n, Q);
	    vector<vector<State> > Qvec_comb = Nfta::combine_states(Qvec);
	    
	    for(vector<vector<State> >::const_iterator i_q1 = Qvec_comb.begin(); i_q1 != Qvec_comb.end(); ++i_q1)
	      {
		vector<State> q1 = *i_q1;
		for(StateSet::const_iterator i_q2 = Q.begin(); i_q2 != Q.end(); ++i_q2)
		  {
		    State q2 = *i_q2;
		    
		    if(with_probability(edge_density))
		      {
			nfta->add_edge(n, q1, edge_with_density(space, n_vars, symbol_density), q2);
		      }
		  }
		}      
	    
	  }
	else
	  {
	    for(StateSet::const_iterator i_q2 = Q.begin(); i_q2 != Q.end(); ++i_q2)
	      {
		State q2 = *i_q2;
		
		if(with_probability(edge_density))
		  {
		    nfta->add_edge(n, vector<State>(), edge_with_density(space, n_vars, symbol_density), q2);
		  }
	      }
	  }
      

      }


  

    return nfta;
  }

  // help functions to print an automaton


  static set<string> generate_paths(gbdd::Bdd p, unsigned int i, string prefix)
  {
    if(p.bdd_is_leaf())
      {
	set<string> paths;

	if(p.bdd_leaf_value())
	  {
	    paths.insert(prefix);
	  }

	return paths;
      }
    else
      {
	unsigned int index = p.bdd_var();

	if(i < index)
	  {
	    return generate_paths(p, i+1, prefix + "X");
	  }
	else
	  {
	    assert(i == index);

	    set<string> paths;

	    set<string> paths1 = generate_paths(p.bdd_then(), i+1, prefix + "1");

	    set<string> paths0 = generate_paths(p.bdd_else(), i+1, prefix + "0");

	    paths = paths1;
	    paths.insert(paths0.begin(), paths0.end());

	    return paths;
	  }
      }
  }


  static void print_paths(ostream& out, set<string> paths)
  {
    if(paths.size()>0)
      {
	set<string>::const_iterator i = paths.begin();

	out << *i;
	++i;

	for(; i != paths.end(); ++i)
	  {
	    out << ", " << *i;
	  }
      }
  }


  // Printing an automaton to a stream:

  ostream& operator << (ostream& s, const Nfta& nfta)
  {
    unsigned int max_arity = nfta.max_arity();
    StateSet all_states = nfta.states();
    StateSet accepting = nfta.states_accepting();
    StateSet not_accepting = all_states - accepting;
    unsigned int nb_states = all_states.size();

    // print the arity and the number of states first:
    s << "Nfta {" << endl << "maximum arity=" << max_arity << ";" << endl;
    s << "nb of states=" << nb_states << ";" << endl;

    // print about the states, accepting and others:
    s << "accepting states:" << accepting << ";"  << endl;
    s << "non-accepting states:" << not_accepting << ";"  << endl;


    gbdd::Domain edge_print_domain = gbdd::Domain::infinite();


    // loop through all arities
    for(unsigned int a=0; a<=max_arity; ++a)
      {
	s << "arity " << a << ":" << endl;

	if(a>0)
	  { // arity > 0 -> normal case
	    vector< vector<State> > cmb = Nfta::combine_states(Nfta::combinations(a, all_states));
	    
	    vector< vector<State> >::const_iterator i = cmb.begin();

	    // cout << "cmb.size()=" << cmb.size() << endl;

	    // loop through all possible a-sized tuples of states
	    while(i != cmb.end())
	      {
		vector<State> vfrom = *i;
		vector<StateSet> from(a);
		for(int index=0; index<a; ++index) from[index] = Set(all_states , vfrom[index]);
		// successor states for the current tuple
		StateSet  to = nfta.successors(a, from,
					       Set::universal(nfta.alphabet()));

		// cout << "to.size()=" << to.size() << endl;

		StateSet::const_iterator j = to.begin();
		// loop through all successors for the current tuple
		while(j != to.end())
		  {
		    s << "<";

		    for(vector<State>::const_iterator t = vfrom.begin(); t != vfrom.end(); ++t)
		      { // output vfrom to s
			s << "(" << *t << ")";
		      }



		    s << "> -> " << *j << " on symbol(s) ";
		    SymbolSet edge = nfta.edge_between(a, vfrom, *j);
		    
		    print_paths(s, generate_paths(Set(edge_print_domain, edge).get_bdd(),
						  0,
						  " "));

		    s << ";" << endl;

		    ++j;
		  }

		++i;
	      }

	  }
	else
	  { // arity 0 -> special case
	    StateSet  to = nfta.successors(a, vector<StateSet>(),
					   Set::universal(nfta.alphabet()));

	    StateSet::const_iterator j = to.begin();
	    // loop through all successors for the current tuple
	    while(j != to.end())
	      {
		s << " -> " << *j << " on symbol(s) ";
		SymbolSet edge = nfta.edge_between(a, vector<State>(), *j);
		
		print_paths(s, generate_paths(Set(edge_print_domain, edge).get_bdd(),
					      0,
					      " "));
		
		s << ";" << endl;
		
		++j;
	      }

	  }
      }


    s << "}" << endl;

    return s;
  }

Nfta* Nfta::Factory::ptr_forall(const BoolConstraint& c) const
{
	const Bdd& p = dynamic_cast<const Bdd&>(c); // Only supporting this for now, use ptr_convert later

	assert(false); // Not implemented
}




} /* namespace treeAutomata  */
