/*
 * bnfta.cc:
 *     An implementation of NFTA's using BDD's to encode the transition relation...
 *
 * Authors:
 *    Julien d'Orso (julien.dorso@it.uu.se)
 *
 */


#include "bnfta.h"


namespace gautomata
{
  using namespace gbdd;

  // -- internal use only --

  static bool fn_is_monotonic_in(bool (*fn)(bool v1, bool v2),
				 bool v1, bool v2)
  {
    return !fn(v1, v2) ||
      (fn(v1, true) &&
       fn(true, v2) &&
       fn(true, true));
  }

  // Check that this boolean function is well behaved.
  static bool fn_is_monotonic(bool (*fn)(bool v1, bool v2))
  {
    return 
      fn_is_monotonic_in(fn, false, false) && 
      fn_is_monotonic_in(fn, false, true) &&
      fn_is_monotonic_in(fn, true, false) && 
      fn_is_monotonic_in(fn, true, true);

  }


  static Domain first_vars(Domain vs)
  {
    unsigned int n_vars = vs.size() / 2;
    Domain res;

    Domain::const_iterator i;
    unsigned int index = 0;
    for(i = vs.begin(); n_vars > 0; ++i)
      {
	res |= Domain(*i);

	n_vars--;
      }

    return res;
  }


  static Domain last_vars(Domain vs)
  {
    unsigned int n_vars = vs.size() / 2;
    Domain res;

    Domain::const_iterator i;
    unsigned int index = 0;

    i = vs.begin();

    {
      int j;
      for(j = 0; j < n_vars; ++j)
	++i;
    }

    for(; n_vars > 0; ++i)
      {
	res |= Domain(*i);

	n_vars--;
      }

    return res;
  }


  // -- now we really start ! --


  Space* Bnfta::get_space() const
  {
    return _space;
  }


  unsigned int Bnfta::get_n_states() const
  {
    return n_states;
  }


  unsigned int Bnfta::max_arity() const
  {
    return _max_arity;
  }

  StateSet  Bnfta::states() const
  {
    return _states;
  }

  StateSet  Bnfta::states_accepting() const
  {
    return _accepting;
  }
    
  Bnfta::Factory::Factory(Space* space)
    : _space(space)
  {}

  Nfta* Bnfta::Factory::ptr_empty(unsigned int _max_arity) const
  {
    return new Bnfta(_space, _max_arity, false);
  }

  Nfta* Bnfta::Factory::ptr_universal(unsigned int _max_arity) const
  {
    return new Bnfta(_space, _max_arity, true);
  }

  Nfta* Bnfta::Factory::ptr_symbol(unsigned int max_arity, SymbolSet sym) const 
  {
    return new Bnfta(Bnfta::symbol(_space, max_arity, sym));
  } 
  
  Nfta*  Bnfta::Factory::ptr_symbol_with_children(unsigned int max_arity,
							   SymbolSet sym,
							   unsigned int arity,
							   vector<const Nfta*> children) const
  {
    return new Bnfta(Bnfta::symbol_with_children(_space, max_arity, sym, arity, children));
  } 

  Nfta* Bnfta::ptr_negate(const Nfta &a) const
  {
    Bnfta* res = new Bnfta(negate(Bnfta(a)));
    return res;
  }
  



  Nfta::Factory*  Bnfta::ptr_factory() const
  {
   return new Factory(_space);
  }



  Domain Bnfta::map_transitions_source(Domain vs, unsigned int a) const
  {
    return vs + (64+32*a);
  }

  Domain Bnfta::map_transitions_alphabet(Domain vs, unsigned int a) const
  {
    return vs + (128+32*a);
  }

  Domain Bnfta::map_transitions_dest(Domain vs, unsigned int a) const
  {
    return vs + (96+32*a);
  }


  Domains Bnfta::get_transitions_domains(unsigned int arity) const
  {
    Domain vars_states = Domain(0, Bdd::n_vars_needed(n_states));
    Domain vars_syms = Domain::infinite();

    if(arity>0)
      {
	Domains tmp(arity); 
	for (int i=0; i<arity; ++i)
	  tmp[i] = map_transitions_source(vars_states, i);
	    
	return
	  tmp *
	  map_transitions_alphabet(vars_syms, _max_arity) *
	  map_transitions_dest(vars_states, _max_arity);
      }
    else
      return 
	map_transitions_alphabet(vars_syms, _max_arity) *
	map_transitions_dest(vars_states, _max_arity);
  }



  Bnfta::Bnfta(Space* space, unsigned int max_arity)
    : _space(space),
      n_states(0),
      _max_arity(max_arity),
      _accepting(Domain(), Bdd(space, false)),
      _states(Domain(), Bdd(space, false)),
      _transitions(max_arity+1),
      is_complete(false),
      is_deterministic(true)
  {
    for(int i = 0; i<=_max_arity ; ++i)
      {
	_transitions[i] = Relation(get_transitions_domains(i), Bdd(space, false));
      }
  }

  Bnfta::Bnfta(Space* space, unsigned int max_arity, bool v)
    : _space(space),
      n_states(0),
      _max_arity(max_arity),
      _accepting(Domain(), Bdd(space, false)),
      _states(Domain(), Bdd(space, false)),
      _transitions(max_arity+1),
      is_complete(true),
      is_deterministic(true)
  {
    for(int i = 0; i<= _max_arity; ++i)
      {
	_transitions[i] = Relation(get_transitions_domains(i), Bdd(space, false));
      }
    
    if(v)
      {
	State q = add_state(true);
	for(int i = 0; i <= _max_arity ; ++i)
	  {
	    vector<State> vq(i, q);
	    add_edge(i, vq, alphabet(), q);
	  }
      }
  }


  Bnfta::Bnfta(const Bnfta& bnfta)
    : _space(bnfta._space),
      n_states(bnfta.n_states),
      _max_arity(bnfta._max_arity),
      _accepting(bnfta._accepting),
      _states(bnfta._states),
      _transitions(bnfta._transitions),
      is_complete(bnfta.is_complete),
      is_deterministic(bnfta.is_deterministic)
  {}

  

  Bnfta::Bnfta(const Nfta& nfta)
    : _max_arity(nfta.max_arity()),
      _accepting(nfta.states_accepting()),
      _states(nfta.states()),
      _transitions(nfta.max_arity()+1),
      is_complete(false),
      is_deterministic(false)
  {
    _space = _accepting.get_space();
    n_states = _states.size();

    for(unsigned int i = 0; i <= _max_arity; ++i)
      _transitions[i] = nfta.transitions(i);
  }



  Bnfta Bnfta::symbol(Space* space, unsigned int max_arity, SymbolSet sym)
  {
    Bnfta res(space, max_arity);
    
    State q = res.add_state(true);
   
    res.add_edge(0, vector<State>(), sym, q);

    return res;
  }

  
  Bnfta  Bnfta::symbol_with_children(Space* space, unsigned int max_arity,
				     SymbolSet sym,
				     unsigned int arity,
				     vector<const Nfta*> children)
  {
    vector<Bnfta*> a_vec(arity);
    for(unsigned int k = 0; k < arity; ++k)
      { // get a copy of the children automata
	a_vec[k] = new Bnfta(*(children[k]));
      }

    unsigned int max_n_vars = 0; 
    for(unsigned int k = 0; k < arity; ++k)
      { // number of vars needed to represent the states of
	// any of the children automata
	max_n_vars = max(max_n_vars,
			 Bdd::n_vars_needed(a_vec[k]->n_states));
      }

    // the extension needed to make sure states of 
    // the children will be distinct.
    unsigned int ext_n_vars = Bdd::n_vars_needed(arity+1);

    for(unsigned int k = 0; k < arity; ++k)
      { // all children automata should have the same 
	// state-space
	a_vec[k]->increase_to_n_states(1 << max_n_vars, false);
      }


    Bnfta res(space, max_arity);
    res.set_n_states(1 << (max_arity + ext_n_vars));

   
    for(unsigned int x = 0; x <= max_arity; ++x)
      {
	Bdd trans(space, false);

	for(unsigned int y = 0; y < arity; ++y)
	  {
	    Domain dom(max_n_vars, ext_n_vars); 
	    Bdd b = Bdd(space, true);

	    for(unsigned int z = 0; z < x; ++z)
	      {
		Domain dom_translated = res.map_transitions_source(dom, z);
		
		b &= Bdd::value(space, dom_translated, y);
	      }

	    b &= Bdd::value(space, res.map_transitions_dest(dom, max_arity), y);


	    Bdd a_trans = (a_vec[y]->transitions(x)).get_bdd() & b;
	  
	    trans |= a_trans;
	  }

	res._transitions[x] = Relation(res.get_transitions_domains(x), trans);
      }

    vector<StateSet> svec(arity);


    
    for(unsigned int y = 0; y < arity; ++y)
      {
	Domain dom(max_n_vars, ext_n_vars);
	
	Bdd baccpt = (a_vec[y]->_accepting).get_bdd() & Bdd::value(space, dom, y);
	
	svec[y] = StateSet(res.states().get_domain(), baccpt);
      }

    State q = (1 << (max_n_vars+ext_n_vars))-1;
    
    res.add_edge(arity, svec, sym, StateSet(res._states, q));
    
    res._accepting = StateSet(res._states, q);

    return res.filter_states_live();
    //    return res;
  }



  Nfta* Bnfta::ptr_clone() const
  {
    return new Bnfta(*this);
  }
  

  SymbolSet Bnfta::alphabet() const
  {
    return Set(Domain::infinite(), Bdd(_space, true));
  }

  Relation Bnfta::transitions(unsigned int arity) const
  {
    return _transitions[arity];
  }

  // Updating n_states ....

  void Bnfta::set_n_states(unsigned int new_n_states)
  {
    unsigned int n_vars = Bdd::n_vars_needed(new_n_states);

    _states = Set(Domain(0, n_vars), Bdd(_space, true));

    {
      unsigned int  i = new_n_states;
      
      for(; i < (1 << n_vars); ++i)
	{
	  _states &= !(Set(_states, i));
	}
    }

    n_states = new_n_states;

    is_deterministic = false;
    is_complete = false;
  }


  void Bnfta::increase_to_n_states(unsigned int new_n_states,
				   bool accepting)
  {
    unsigned int old_n_states = n_states;
    unsigned int old_n_vars = Bdd::n_vars_needed(old_n_states);
    unsigned int new_n_vars = Bdd::n_vars_needed(new_n_states);

    n_states = new_n_states;

    if(new_n_states > old_n_states)
      {
	Domain new_domain = Domain(0, new_n_vars);

	_accepting = _accepting.extend_domain(new_domain);
	_states = _states.extend_domain(new_domain);

	for(unsigned int i=0; i <= _max_arity; ++i)
	  {
	    Domains new_transitions_domains = get_transitions_domains(i);

	    for(unsigned int j=0; j<i; ++j)
	      _transitions[i] = _transitions[i].extend_domain(j, new_transitions_domains[j]);

	    _transitions[i] = _transitions[i].extend_domain(i+1, new_transitions_domains[i+1]);
	  }
      }

    Set new_states(_states, old_n_states, new_n_states - 1);

    _states |= new_states;
    if(accepting)
      _accepting |= new_states;

    is_deterministic = false;
    is_complete = false;
  }

  
  // Construction methods
  
  State  Bnfta::add_state(bool accepting)
  {
    unsigned int old_n_vars = Bdd::n_vars_needed(n_states);
    unsigned int new_n_vars = Bdd::n_vars_needed(n_states+1);
    
    State new_state = n_states;
    n_states++;
    
    if(new_n_vars > old_n_vars)
      {
	Domain new_domain = Domain(0, new_n_vars);    
	
	_accepting = _accepting.extend_domain(new_domain);
	_states = _states.extend_domain(new_domain);
	
	
	for(int i = 0; i <= _max_arity; ++i)
	  {
	    Domains new_transitions_domains = get_transitions_domains(i);

	    // cout << "arity " << i << endl;
	    
	    for(int j = 0; j<i; ++j)
	      {
		_transitions[i] = _transitions[i].extend_domain(j, new_transitions_domains[j]);
		// cout << "domains " << new_transitions_domains[j] << endl;
	      }
	    
	    _transitions[i] = _transitions[i].extend_domain(i+1, new_transitions_domains[i+1]);
	  }
      }

    //    cout.flush();
    
    if(accepting)
      _accepting |= Set(_accepting, new_state);
    
    _states |= Set(_states, new_state);
    
    is_complete = false;
    is_deterministic = false;
    
    return new_state;
  }

  void  Bnfta::add_edge(unsigned int arity, vector<State> from, SymbolSet on, State to)
  {
    Nfta::add_edge(arity, from, on, to);
  }

  void  Bnfta::add_edge(unsigned int arity, vector<StateSet> from, SymbolSet on, StateSet to)
  {
    vector<BddBasedSet> tuple;
    
    for(int i=0; i<arity; ++i)
      tuple.push_back(from[i]);

    tuple.push_back(on);
    tuple.push_back(to);
    
    _transitions[arity] |= BddBasedRelation::cross_product(_transitions[arity].get_domains(), tuple);
    
    is_complete = false;
    is_deterministic = false;
  }

  void  Bnfta::add_transitions(unsigned int arity, Relation new_transitions)
  {
    _transitions[arity] |= new_transitions;
    
    is_complete = false;
    is_deterministic = false;
  }
  

  // general method to remove un-necessary states

  Bnfta Bnfta::filter_states(StateSet s) const
  {
    if(s.size() == 0)
      return Bnfta(_space, _max_arity);

    Bnfta res = *this;

    res._accepting &= s;

    Relation compressmap = s.compress();

    res.set_n_states(s.size());

    Domain new_domain = res.states().get_domain();
    res._accepting = StateSet(new_domain, res._accepting.compose(compressmap));

    for(int k=0; k <= _max_arity; ++k)
      {
	Domains new_transitions_domains = res.get_transitions_domains(k);

	for(int i=0; i<k; ++i)
	  res._transitions[k] = res._transitions[k].compose(i, compressmap);
	res._transitions[k] =  Relation(new_transitions_domains, res._transitions[k].compose(k+1, compressmap));
      }

    res.is_complete = false;

    return res;
  }

  Bnfta Bnfta::filter_states_reachable() const
  {
    return filter_states(states_reachable());
  }

  Bnfta Bnfta::filter_states_productive() const
  {
    return filter_states(states_productive());
  }

  Bnfta Bnfta::filter_states_live() const
  {
    return filter_states(states_live());
  }


  Bnfta Bnfta::project(Domain vs) const
  {
    Bnfta res(*this);
    
    for(unsigned int k = 0; k <= _max_arity; ++k)
      {
	res._transitions[k] = 
	  Relation(res._transitions[k].get_domains(),
		   res._transitions[k].get_bdd().project(map_transitions_alphabet(vs, _max_arity)));
      }

    res.is_complete = false;
    res.is_deterministic = false;

    return res;
  }
  
  Bnfta Bnfta::rename(VarMap map) const
  {
    Bnfta res(*this);

    for(unsigned int k = 0; k <= _max_arity; ++k)
      {	
	VarMap new_map;

	VarMap::const_iterator i;
	for(i = map.begin(); i != map.end(); ++i)
	  {
	    Domain source = map_transitions_alphabet(Domain(i->first, 1),
						     _max_arity);
	    Domain dest   = map_transitions_alphabet(Domain(i->second, 1),
						     _max_arity);

	    //    cout << "new_map: " << *(source.begin())  << " -> "   << *(dest.begin()) << endl;

	    new_map[*(source.begin())] = *(dest.begin());
	  }
	
	// cout << "res.transitions[k]:" << (Bdd)(res._transitions[k]) << endl;

	res._transitions[k] =
	  Relation(res._transitions[k].get_domains(),
		   res._transitions[k].get_bdd().rename(new_map));

      }

    return res;    
  }

  Bnfta Bnfta::rename(Domain vs1, Domain vs2) const
  {
    assert(vs1.size() == vs2.size());
    
    return rename(Domain::map_vars(vs1, vs2));
  }



  Bnfta   Bnfta::concatenation(const Bnfta& a1, const Bnfta& a2, SymbolSet sym)
  {
    unsigned int n_vars_a1 = Bdd::n_vars_needed(a1.n_states);
    unsigned int n_vars_a2 = Bdd::n_vars_needed(a2.n_states);
    unsigned int max_n_vars = max(n_vars_a1, n_vars_a2);
    unsigned int max_a = a1._max_arity;
    
    Bnfta res(a1._space, max_a);
    res.set_n_states(1 << (max_n_vars + 1));
    
    Domain new_states_dom = res._states.get_domain();
    
    
    StateSet a1_accpt = a1.states_accepting().extend_domain(new_states_dom, false);
    StateSet a2_accpt = a2.states_accepting().extend_domain(new_states_dom, true);
    
    StateSet state_dup = StateSet(a1._states.get_domain(), a1.transitions(0).restrict(0, sym).project(0).get_bdd()).extend_domain(new_states_dom, false);
    
    for(unsigned int x = 0; x <= max_a; ++x) 
      {
	Domains new_trans_doms = res.get_transitions_domains(x);
	
	Relation a1_trans = a1.transitions(x);
	Relation a2_trans = a2.transitions(x);
	
	for(unsigned int z = 0; z < x; ++z)
	  {
	    a1_trans = a1_trans.extend_domain(z, new_trans_doms[z], false);
	    a2_trans = a2_trans.extend_domain(z, new_trans_doms[z], true);
	  }
	a1_trans = a1_trans.extend_domain(x+1, new_trans_doms[x+1], false);
	a2_trans = a2_trans.extend_domain(x+1, new_trans_doms[x+1], true);

	Relation t_dup = a2_trans.restrict(x+1, a2_accpt).project(x+1).restrict(x+1, state_dup);

	res._transitions[x] = a1_trans | a2_trans | t_dup;
      }

    res._accepting = a1_accpt;
    
    return res.filter_states_live();
    // return res;
  }
  


  // Unary operations:



  Bnfta Bnfta::with_accepting(StateSet accepting) const
  {
    Bnfta res = *this;

    res._accepting = StateSet(res._accepting.get_domain(), accepting);

    res.is_complete = false;
    res.is_deterministic = false;

    return res;
  }


  Bnfta  Bnfta::kleene(SymbolSet sym) const
  {
    Bnfta res = *this;
    
    StateSet state_dup = StateSet(_states.get_domain(),
				  StateSet(_transitions[0].get_domain(1),
					   _transitions[0].restrict(0, sym).project(0).get_bdd())		  
				  );
				  
    StateSet target = _accepting;

    for(unsigned int k = 0; k <= _max_arity; ++k)
      {
	Relation t = _transitions[k];
	t = t.restrict(k+1, target).project(k+1);
	t = t.restrict(k+1, state_dup);

	res._transitions[k] |= t;
      }

    res._accepting |= state_dup;

    res.is_complete = false;
    res.is_deterministic = false;

    return res;
  }


  Bnfta Bnfta::remove_leaves(SymbolSet leaves) const 
  {
    Bnfta res = *this;

    Bdd t = _transitions[0].get_bdd() - _transitions[0].restrict(0, leaves).get_bdd();

    res._transitions[0] = Relation(res.get_transitions_domains(0), t);

    res.is_complete = false;

    return res.filter_states_live();
}

 
  /*
   * complete():
   *     Returns an equivalent automaton that is complete.
   */ 
  Bnfta  Bnfta::complete() const
  { 
    if(is_complete)
      return *this;
    
    Bnfta res = *this;

    // Add a dummy non-accepting state,
    // and add transitions to this state on any symbol from all other states.

    State q = res.add_state(false);
    
    for(int i = 0; i <= _max_arity ; ++i)
      {
	vector<StateSet> vq(i, res.states());
	res.add_edge(i, vq, res.alphabet(), Set(res.states(), q));
      }

    res.is_complete = true;
    res.is_deterministic = false;

    return res;
  }
    

  
  // compute the product automaton, assuming that a1 and a2 have the right
  // properties to make the result correct.
   Bnfta Bnfta::actual_product(Bnfta a1, Bnfta a2, bool (*fn)(bool v1, bool v2))
  {
    unsigned int a1_n_vars = Bdd::n_vars_needed(a1.n_states);
    unsigned int a2_n_vars = Bdd::n_vars_needed(a2.n_states);

    unsigned int max_n_vars = max(a1_n_vars, a2_n_vars);

    unsigned int even_n_states = 1 << max_n_vars;

    // extend both automata to contain even_n_states

    a1.increase_to_n_states(even_n_states, false);
    a2.increase_to_n_states(even_n_states, false);


    // we are now going to interleave the variables for the states of 
    // a1 and those for the states of a2, so that the result will
    // be the cross product as we want...

    unsigned int arity_prod = max(a1._max_arity, a2._max_arity);
    Bnfta product_a(a1._space, arity_prod);

    product_a.set_n_states(1 << (max_n_vars*2));

    Domain new_state_domain = Domain(0, 2*max_n_vars);
    Domain old_state_domain = Domain(0, max_n_vars);

    product_a._accepting = 
      Set(new_state_domain,
	  Bdd::bdd_product(
		   a1._accepting.get_bdd(),
		   a2._accepting.get_bdd().rename(old_state_domain, old_state_domain + max_n_vars),
		   fn));

    for(int i=0; i <= arity_prod; ++i)
      {
	Domains new_transitions_domains = product_a.get_transitions_domains(i);

	if(i<=a1._max_arity && i<=a2._max_arity)
	  {
	    Domains new_domains_a1 = new_transitions_domains;
	    for(int j=0; j<i; ++j)
	      new_domains_a1[j] = first_vars(new_domains_a1[j]);
	    new_domains_a1[i+1] = first_vars(new_domains_a1[i+1]);
	    
	    Domains new_domains_a2 = new_transitions_domains;
	    for(int j=0; j<i; ++j)
	      new_domains_a2[j] = last_vars(new_domains_a2[j]);
	    new_domains_a2[i+1] = last_vars(new_domains_a2[i+1]);
	    
	    Bdd transitions_a1 = Relation(new_domains_a1, a1._transitions[i]).get_bdd();
	    Bdd transitions_a2 = Relation(new_domains_a2, a2._transitions[i]).get_bdd();
	    
	    Bdd new_transitions = transitions_a1 & transitions_a2;
	    
	    product_a._transitions[i] = Relation(new_transitions_domains,
						 new_transitions);
	  }
	else
	  product_a._transitions[i] = Relation(new_transitions_domains,
					       Bdd(product_a._space, false));
      }

    product_a.is_deterministic = (a1.is_deterministic && a2.is_deterministic);
    product_a.is_complete =(a1.is_complete && a2.is_complete);

    return product_a.filter_states_live();
  } 
  
  Bnfta Bnfta::negate(const Bnfta& a)
  {
    Bnfta res = Bnfta(a);
    res= res.complete();
    res=res.determinize();
    StateSet new_acc= res.states() - res.states_accepting();
    res = res.with_accepting(new_acc);
    return res;
  }

  // Boolean operations:

  // Compute the product automaton. Depending on fn, we do
  // some modifications on a1 and a2 first. Then call actual_product()
  Bnfta  Bnfta::product(const Bnfta& a1, const Bnfta& a2, bool (*fn)(bool v1, bool v2))
  {
    Bnfta a1_prod = a1.complete();
    Bnfta a2_prod = a2.complete();   

    // If "fn" is nice, then it's enough to make a1 and a2 complete.
    // Otherwise, we need to make a1 and a2 deterministic as well.
    if(!fn_is_monotonic(fn))
      {
	a1_prod = a1_prod.determinize();
	a2_prod = a2_prod.determinize(); 
      } 

	return actual_product(a1_prod, a2_prod, fn);
  }
  
  Bnfta operator & (const Bnfta& a1, const Bnfta& a2)
  {
    return Bnfta::product(a1, a2, Bdd::fn_and);
  }
 
  Bnfta operator | (const Bnfta& a1, const Bnfta& a2)
  {
    return Bnfta::product(a1, a2, Bdd::fn_or);
  }
 
  Bnfta operator - (const Bnfta& a1, const Bnfta& a2)
  {
    return Bnfta::product(a1, a2, Bdd::fn_minus);
  }


  // predicates

  bool Bnfta::is_true() const
  {
    Bnfta det = complete().determinize();
    StateSet accpt = det.states_accepting();
    StateSet rchbl = det.states_reachable();

    //    cout << "inside is_true(), det=" << det << endl;
    // cout << "is_true(), accepting=" << accpt << endl;
    // cout << "is_true(), reachable=" << rchbl << endl;

    return (rchbl & (!accpt)).is_empty();//i.e. rchbl <= accpt...  old test was: accpt == rchbl;
  }

  bool Bnfta::is_false() const
  {
    return states_live().is_empty();
  }

  bool operator == (const Bnfta& a1, const Bnfta& a2)
  {
    Bnfta tmp = Bnfta::product(a1, a2, Bdd::fn_iff);

    //    cout << "inside ==(), tmp=" << tmp << endl;

    return tmp.is_true();
  }


  bool operator != (const Bnfta& a1, const Bnfta& a2)
  {
    return !(a1 == a2);
  }


  bool operator <= (const Bnfta& a1, const Bnfta& a2)
  {
    return Bnfta::product(a1, a2, Bdd::fn_implies).is_true();
  }



  bool operator >= (const Bnfta& a1, const Bnfta& a2)
  {
    return (a2 <= a1);
  }


  bool operator > (const Bnfta& a1, const Bnfta& a2)
  {
    return (a2 <= a1) && (a1 != a2);
  }



  bool operator < (const Bnfta& a1, const Bnfta& a2)
  {
    return (a1 <= a2) && (a1 != a2);
  }



  // misc

  Nfta* Bnfta::ptr_product(const Nfta &a2, bool (*fn)(bool v1, bool v2)) const
  {
    return new Bnfta(product(*this, Bnfta(a2), fn));
  }
  

  Bnfta* Bnfta::ptr_product(const Bnfta& a2, bool (*fn)(bool v1, bool v2)) const
  {
    return new Bnfta(product(*this, Bnfta(a2), fn));
  }

  Nfta*  Bnfta::ptr_concatenation(const Nfta& a2, SymbolSet sym) const
  {
    return new Bnfta(concatenation(*this, Bnfta(a2), sym));
  }



    // unary operations

  Nfta* Bnfta::ptr_with_accepting(StateSet accepting) const
  {
    return new Bnfta(with_accepting(accepting));
  }


    
  Nfta*  Bnfta::ptr_complete() const
  {
    return new Bnfta(complete());
  }

  Nfta*  Bnfta::ptr_determinize() const
  {
    return new Bnfta(determinize());
  }

  Nfta*  Bnfta::ptr_minimize() const
  {
    return new Bnfta(minimize());
  }

  Nfta*  Bnfta::ptr_project(Domain vs) const
  {
    return new Bnfta(project(vs));
  }

  Nfta*  Bnfta::ptr_rename(VarMap map) const
  {
    return new Bnfta(rename(map));
  }

  Nfta* Bnfta::ptr_rename(Domain vs1, Domain vs2) const
  {
    return new Bnfta(rename(vs1, vs2));
  }


  Nfta* Bnfta::ptr_kleene(SymbolSet sym) const
  {
    return new Bnfta(kleene(sym));
  }


  Nfta* Bnfta::ptr_remove_leaves(SymbolSet leaves) const
  {
    return new Bnfta(remove_leaves(leaves));
  }




  Nfta*  Bnfta::ptr_filter_states(StateSet s) const
  {
    return  new Bnfta(filter_states(s));
  }
    
  Nfta*  Bnfta::ptr_filter_states_live() const
  {
    return  new Bnfta(filter_states_live());
  }

  Nfta*  Bnfta::ptr_filter_states_reachable() const
  {
    return  new Bnfta(filter_states_reachable());
}
  
  Nfta*  Bnfta::ptr_filter_states_productive() const
  {
    return  new Bnfta(filter_states_productive());
}

    

} /* namespace treeAutomata  */
