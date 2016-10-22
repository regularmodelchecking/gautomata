#include "mnfa.h"
#include <assert.h>
#include <vector>
#include <set>
#include <numeric>
#include <gbdd/bdd.h>
#include <gbdd/domain.h>
#include <iostream>

#define max(a, b) ((a > b) ? a : b)

namespace gautomata
{
  using namespace gbdd;       
  
  MNfa::Factory::Factory(Space* space):
    _space(space)
  {}

struct vectorHash
{
  // hash vector with SymbolSets
  size_t operator()( vector<SymbolSet> v) const
  {
    vector<size_t> intVector;
    vector<size_t>::const_iterator j;
    vector< SymbolSet > :: const_iterator i;
    int tmpInt = v.size() + 1;
      hash<Set> H;
      size_t t;
    for (i=v.begin(); i!=v.end(); ++i){
      t = H(*i);
      intVector.push_back(t);

    }
    int res=0;
    for (j=intVector.begin(); j!= intVector.end(); j++){
      res= res + (*j) * tmpInt;   
      tmpInt--;
    }


return res;
  }
};

 
  
  MNfa* MNfa::Factory::ptr_empty() const
  {
    return new MNfa(_space, false);
  }
  
  MNfa* MNfa::Factory::ptr_universal() const
  {
    return new MNfa(_space, true);
  }
  
  MNfa* MNfa::Factory::ptr_epsilon() const
  {
    return new MNfa(MNfa::epsilon(_space));
  }
  

  MNfa* MNfa::Factory::ptr_symbol(SymbolSet sym) const
  {
    return new MNfa(MNfa::symbol(_space, sym));
  }

 
  MNfa::Factory* MNfa::ptr_factory() const
  {
	  return new Factory(_space);
  }
  
  MNfa* MNfa::ptr_clone() const
  {
    return new MNfa(*this);
  }

 

// Creates a new automaton
  /**
   * 
   *
   * @param space The BDD space which will be used for the representation
   */
  MNfa::MNfa(Space* space):
    _space(space),
    _starting(),
    _accepting(),
    _transition_matrix()
    
    
{
}


  MNfa::MNfa(const MNfa &mnfa):
    _space(mnfa._space),
    _starting(mnfa._starting),
    _accepting(mnfa._accepting),
    _transition_matrix(mnfa._transition_matrix)
    
  {
  }
  
  
  MNfa::MNfa(Space* space, bool v):
    _space(space),
    _starting(),
    _accepting(),
    _transition_matrix()
    
  {
    if (v)
      {
	State q = add_state(true, true);
                
	add_edge(q, alphabet(), q);
      }
  }
  
  

  Space* MNfa::get_space() const
  {
    return _space;
  }

  State MNfa::get_n_states()
  {

    State length = _transition_matrix.size();
    return length;
   
  }


  MNfa MNfa::epsilon(Space* space) {
    MNfa nfa(space);
  
  nfa.add_state(true, true);
  
  return nfa;
  }


  void MNfa::add_edge(State from, SymbolSet on, State to)
  {

    if (_transition_matrix.size() < from ){
      cout << "Cannot add edge between non existing states...\n";
    }
   
    _transition_matrix[from][to] |= Set(alphabet().get_domain(), on);
 
  }
  




  State MNfa:: add_state (bool accepting, bool starting)
  {

	  int size = (*this).get_n_states();

  
    SymbolSet empty_symbol_set(Domain::infinite(), Bdd(_space, false));
 
    vector<SymbolSet> x(size, empty_symbol_set);
    vector<vector <SymbolSet> >::const_iterator iter;
    _transition_matrix.push_back(x);
    int adding=0;
    
    for(iter= _transition_matrix.begin(); iter != _transition_matrix.end(); iter++){
      
        _transition_matrix[adding].push_back(empty_symbol_set);
	adding++;
	}
    
    State n_of_new_state = _transition_matrix.size();
 

    if (accepting == true)
      {
	
	_accepting.insert(n_of_new_state-1);
	
       }
    if (starting == true)
      {	
	_starting.insert(n_of_new_state-1);
	
      }
    return  n_of_new_state - 1;
  }
  

MNfa MNfa::symbol(Space* space, SymbolSet sym)
{
  MNfa nfa(space);
    
  State q = nfa.add_state(false, true);
  State r = nfa.add_state(true, false);
    
  nfa.add_edge(q, sym, r);
  
  return nfa;
}



SymbolSet MNfa::alphabet() const
  {
   

    return Set(Domain:: infinite(), Bdd(_space, true));
  }


TransitionMatrix MNfa::get_transition_matrix() const
{
  return _transition_matrix;
}




void MNfa::set_n_states(unsigned int new_n_states)
{
  unsigned int n_vars = Bdd::n_vars_needed(new_n_states);
  SymbolSet _states;
  _states = Set(Domain(0, n_vars), Bdd(_space, false));
  
  {
    unsigned int i = new_n_states;
    
    for (;i < (1 << n_vars);++i)
    
      {
	_states &= !(Set(_states, i));
      }
  }
 
}
	





StateSet MNfa::states() const
{

  StateSet states(_space); 
  int no_of_states = _transition_matrix.size();
  for(int i=0; i<no_of_states; i++ ){
    pair<Set::const_iterator, bool> tmp = states.insert(i);
    }
  


  return states;
}
  
StateSet MNfa::states_starting() const
{

  StateSet starting(_space);
  IntSet::const_iterator starting_iter;
  for(starting_iter=_starting.begin(); starting_iter!=_starting.end(); starting_iter++){
   pair<Set::const_iterator, bool> tmp = starting.insert(*starting_iter);
  }

  return starting;
}

  
IntSet MNfa:: starting_set_of_states() const
{
  return _starting;
}

  
IntSet MNfa:: accepting_set_of_states() const
{
  return _accepting;
}


StateSet MNfa::states_accepting() const
{
StateSet accepting(_space);
 IntSet::const_iterator acc_iter;
  for(acc_iter=_accepting.begin(); acc_iter!=_accepting.end(); acc_iter++){
   pair<Set::const_iterator, bool> tmp = accepting.insert(*acc_iter);
  }
 
    return accepting;  
}




  

StateSet MNfa::successors(StateSet q, SymbolSet on) const
{  
  StateSet successor_states(_space); 
  
  for (StateSet::const_iterator state_iterator = q.begin(); state_iterator!= q.end(); ++state_iterator ){
	  for(int i = 0; i <_transition_matrix.size(); i++){
		  if(!(_transition_matrix[*state_iterator][i] & on).is_empty()  ){
			  pair<Set::const_iterator, bool> tmp = successor_states.insert(i );
		  }
	  }	
  }
	  
  return successor_states;
  
}




SymbolSet MNfa::edge_between(State q, State r) const
{
  return _transition_matrix[q][r];
}





}













