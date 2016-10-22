/*
 * nfa-deterministic.cc: 
 *
 * Copyright (C) 2002 Lisa Kaati (kaati@docs.uu.se)
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
 *    Lisa Kaati (kaati@docs.uu.se)
 */

#include "nfa.h"
#include <memory>


namespace gautomata
{

using namespace gbdd;

typedef std::set<int, less<int> > IntSet;

/* intHash: Used to hash integers  */
struct intHash
{ 
	size_t operator()(unsigned int s) const
		{
			hash<int> H;
			size_t t;
			t = H(int(s));
			return t;
		} 
};
  
/* setHash: Used to hash symbolSets */
struct setHash
{ 
	size_t operator()(Set s) const
		{
			hash<Set> H;
			size_t t= H(s);
			return t;
		} 
};
  

/* IntSetHash: Used to hash IntSets */
struct IntSetHash
{ 
	size_t operator()(IntSet s) const
		{
			hash< const char* > H;
			int count=0;
			char str[s.size()+1]; 
			for(IntSet::const_iterator i= s.begin(); i!= s.end(); i++){
				char n = char(*i);  
				str[count] =  n;
				count++;
			}  
			str[s.size()] = 0;
			size_t t= H(str);
			return t; 
		} 
};


/* Adds a powerstate to new automata,  
 *
 *
 * Returns: the newly created state
 */

static
State add_powerstate(Nfa& res, 
		     const Nfa& old, 
		     hash_map<IntSet, State, IntSetHash >& powerstate_to_state,
		     hash_map<State, IntSet, intHash >& state_to_powerstate,
		     IntSet new_state, 
		     bool starting)
{
	
	int accepting_states=0;
	bool accepting= false;
	
	if (powerstate_to_state.find(new_state) ==  powerstate_to_state.end())
	{
		
		StateSet acceptingStateSet = old.states_accepting();
		IntSet acc;
			for (StateSet::const_iterator i= acceptingStateSet.begin(); i!=acceptingStateSet.end(); ++i){
				acc.insert(*i);
			}
				//IntSet acc= old.accepting_set_of_states();
		
		for(IntSet::const_iterator i= new_state.begin(); i!=new_state.end(); i++)
		{
			if (acc.find(*i) != acc.end()){
				accepting_states++;
			}
		}
      
		
		if (accepting_states > 0 ){
			accepting= true;
		}
		
     
		powerstate_to_state[new_state] = res.add_state(accepting, starting);
		state_to_powerstate[powerstate_to_state[new_state]] = new_state;
			
		
	}
	
	State q1 = powerstate_to_state[new_state];
	
	return q1;
		
}



/* Creates all possible combinations on a bit vector 
 * 
 *  
 *@param a vector with bools
 *
 * Returns: A bit_vector with the next combination
 */
static bit_vector next_combination( bit_vector V ){
	int i = V.size()-1;
	int all_ones=0;
	while(i>= 0 && V[i] == true){
		V[i]=false;
		i--;
		all_ones++;
	}
	
	if(i >=0){
		V[i]= true;
	}
	
	return V;
	
	}



/* Locates the next set of powerstates and the symbols they can be reached on 
 *
 *
 * Returns: A hash_map with the reachable states and symbols 
 */

	
static hash_map <SymbolSet, IntSet, setHash> find_next_powerstates( IntSet iset, const Nfa& old) 
{
	
	/* find new powerstates reachable from the powerstate */
		
	IntSet::const_iterator i;
	StateSet Q = old.states();
	int no_of_states= Q.size();
	vector < Set > symbols_from_powerset(no_of_states, Set::empty(old.alphabet()));
	for (i = iset.begin(); i != iset.end(); i++){
		
		for (int k=0; k< no_of_states; k++){
			symbols_from_powerset[k] |= old.edge_between(*i, k); 
		}
		
	}

  
	
	/* Adds symbols from powerstate in a hash_map to successors states */
	
	int to_state=0;
	hash_map <SymbolSet, IntSet, setHash> states_for_symbols;
	for(vector< Set >::const_iterator j = symbols_from_powerset.begin(); j!= symbols_from_powerset.end(); j++){ 
		states_for_symbols[*j].insert(to_state);
		to_state++;
		
	}
	
	/* States_for_symbols contains symbols used for a transition to a new powerstate  (from the inital powerstate) 
	   edges contains all edges from the powerstate to a new powerstate */

	vector< SymbolSet > edges;  /* The edges from the powerstate */
	vector< IntSet > edge_to_state; /* powerstates corresponding to edges */
 
	for( hash_map <SymbolSet, IntSet, setHash>::const_iterator iter= states_for_symbols.begin(); iter != states_for_symbols.end(); iter++){
		
		if (!(iter->first.is_empty()))
		{
			edges.push_back((*iter).first);
			edge_to_state.push_back((*iter).second);
		}
		
	}
	
	
		// Create all combinations of symbolsets
 
	hash_map <SymbolSet, IntSet, setHash> differentsymbols_to_state;
	int state=0;
	Space* sp =old.get_space();
	bit_vector combinations(edges.size(), false);
	bit_vector zero(edges.size(), false);
	IntSet first_set;
	int c= 0;
 
		

 
	do { 
		
		SymbolSet intersection_set = old.alphabet();
		IntSet powerstate;
		int count=0;
		int count2=0;
		
		//Intersection of all symbolsets with true in the combinations vector
		
		for(bit_vector::const_iterator i = combinations.begin(); i!= combinations.end(); i++){
	
			if (*i == true){
				intersection_set= intersection_set & edges[count];
				IntSet tmp= edge_to_state[count];
				for ( IntSet::const_iterator s = tmp.begin(); s!=tmp.end(); s++)
					powerstate.insert(*s);
			}
			count++;
		}
		
		
		for(bit_vector::const_iterator i = combinations.begin(); i!= combinations.end(); i++){
			
			if (*i == false){
				intersection_set = intersection_set - edges[count2];
			}    
			count2++;
		}
		
		if (!(intersection_set.is_empty()))
			differentsymbols_to_state[intersection_set] = powerstate;
		
		combinations = next_combination(combinations);
	} while ( !((bit_vector)combinations == (bit_vector)zero) );




	
	return differentsymbols_to_state;
	
}



/* A new version of find_next_powerstates
 *
 * Locates the next set of powerstates and the symbols they can be reached on 
 * 
 *
 * Returns: A hash_map with the reachable states and symbols 
 */

	
static
hash_map <SymbolSet, IntSet, setHash> find_next_powerstates_new( IntSet iset, const Nfa& old) 
{
	
	/* find new powerstates reachable from the powerstate */
		
	IntSet::const_iterator i;
	StateSet Q = old.states();
	int no_of_states= Q.size();
	vector < Set > symbols_from_powerset(no_of_states, Set::empty(old.alphabet()));
	for (i = iset.begin(); i != iset.end(); i++){
		
		for (int k=0; k< no_of_states; k++){
			symbols_from_powerset[k] |= old.edge_between(*i, k); 
		}
		
	}

  
	
	/* Adds symbols from powerstate in a hash_map to successors states */
	
	int to_state=0;
	hash_map <SymbolSet, IntSet, setHash> states_for_symbols;
	for(vector< Set >::const_iterator j = symbols_from_powerset.begin(); j!= symbols_from_powerset.end(); j++){ 
		states_for_symbols[*j].insert(to_state);
		to_state++;
		
	}
	
	/* States_for_symbols contains symbols used for a transition to a new powerstate  (from the inital powerstate) 
	   edges contains all edges from the powerstate to a new powerstate */

	vector< SymbolSet > edges;  /* The edges from the powerstate */
	vector< IntSet > edge_to_state; /* powerstates corresponding to edges */
 
	for( hash_map <SymbolSet, IntSet, setHash>::const_iterator iter= states_for_symbols.begin(); iter != states_for_symbols.end(); iter++){
		

			edges.push_back((*iter).first);
			edge_to_state.push_back((*iter).second);
	
		
	}
	
/* Creates unique symbolsets that leads to different powerstates */
	



	hash_map <SymbolSet, IntSet, setHash> differentsymbols_to_state;
	Space* sp =old.get_space();
	vector<IntSet > S; // stores the different states
	vector<SymbolSet> P; // stores the different symbols
	SymbolSet s = old.alphabet(); 
/* Start by adding the entire alphabet that leads to a empty stateset */
	P.push_back(s); 
	IntSet empty_set;
	S.push_back(empty_set);



			
	for(int count = 0 ; count < edges.size();  count++ ){

		SymbolSet current = edges[count];
		IntSet current_state = edge_to_state[count];

		for(int t =0;  t < P.size() && !current.is_empty(); t++ ){
			SymbolSet common_symbols = current & P[t]; 
			IntSet newState = S[t];				
			current = current - common_symbols;			
			SymbolSet unique_symbols  = P[t] - common_symbols;
			IntSet powerstate = newState;				
			for ( IntSet::const_iterator s = current_state.begin(); s!=current_state.end(); s++){
				powerstate.insert(*s);
			}
				
				
				// Add the intersection to the vector
			if (!(common_symbols.is_empty())){		
				P[t]=common_symbols;
				S[t]= powerstate;
					
				if (!(unique_symbols.is_empty())){
					P.push_back(unique_symbols);
					S.push_back(newState);
				}
			}
		}
		assert(current.is_empty());
	} 
	for (int i = 0; i < P.size(); i++){
		differentsymbols_to_state[P[i]] = S[i];
	}
		
	return differentsymbols_to_state;
	
}
		


/*
 * Creates a deterministic automata 
 *
 * Returns: A deterministic verion of the automata 
 * 
 */

	
Nfa* Nfa::ptr_deterministic( ) const{
	const Nfa& old= *this;
	Nfa *res = auto_ptr<Nfa::Factory>(ptr_factory())->ptr_empty();
	
  
	hash_map<IntSet, State, IntSetHash> powerstate_to_state;
	hash_map<State, IntSet, intHash> state_to_powerstate;
	StateSet startingStateSet = old.states_starting();
	IntSet start_set;

	for (StateSet::const_iterator i= startingStateSet.begin(); i!=startingStateSet.end(); ++i){
		start_set.insert(*i);
	}

	
	//IntSet start_set =  old.starting_set_of_states();
	hash_map < Set, IntSet, setHash > tmp;
	hash_map < Set, IntSet, setHash >::const_iterator iter;
	
	// Add a inital state 
	State q1= add_powerstate(*res, old, powerstate_to_state, state_to_powerstate, start_set, true);


	/* start_set added in undiscovered_states vector, then new states and transitions are computed 
	   all new undiscovered set are added in the vector, function continues until no more states are found */
	
	vector <IntSet> undiscovered_states;
	undiscovered_states.push_back(start_set);
 
	while (undiscovered_states.empty() == false ){
		IntSet t = undiscovered_states.front();
		//If powestate not a new state in res, add new state
		if(powerstate_to_state.find(t) == powerstate_to_state.end() ){ 
			State q1 = add_powerstate(*res, old, powerstate_to_state, state_to_powerstate, t , false);
			
		}

		// create next set of powerstates and symbols
/*  find_next_powerstates_new is the new version , find_next_powerstates the old version */

		hash_map <SymbolSet, IntSet, setHash> new_powerstate_map =  find_next_powerstates_new(t, old);
   

		
		// State from where the powerstates and transitions are created, if there are any next states
			
		for ( hash_map <SymbolSet, IntSet, setHash>::const_iterator j= new_powerstate_map.begin(); j != new_powerstate_map.end(); j++ ){
			IntSet y= (*j).second;
     

				// if powerstate not added in res
				if( powerstate_to_state.find(y) == powerstate_to_state.end() ){ 
					undiscovered_states.push_back(y);
					State q2 = add_powerstate(*res, old, powerstate_to_state, state_to_powerstate, y , false);
				}
      

				// For all new powerstates, add a state and transition in res 
     
				res->add_edge(powerstate_to_state[t], (*j).first, powerstate_to_state[y] );
      
      
			}
    
    
			vector < IntSet >::iterator first = undiscovered_states.begin(); 
			undiscovered_states.erase(first);
    
		}


    
		return res; 
	}

	
}

