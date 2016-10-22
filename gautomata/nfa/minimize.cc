/*
 * nfa-minimize.cc: 
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
namespace gautomata
{

using namespace gbdd;


typedef std::set<int, less<int> > IntSet;

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

 


Nfa* Nfa::ptr_minimize( ) const{
	
	hash_map<vector<SymbolSet>, IntSet, vectorHash> states_for_vector;
	hash_map<vector<SymbolSet>, IntSet, vectorHash> states_and_symbols;
	const Nfa& old= *this;
	Nfa* res = ptr_factory()->ptr_empty();
	
 
	vector < IntSet > P;
	vector < IntSet > P_old;
	vector< vector <SymbolSet> > transitions_to_partitions_new;
	vector< vector <SymbolSet> > transitions_to_partitions;
	// P = {Q \ F, F} 
	IntSet QF, F;
	int n = (old.states()).size();
	// create Q and F
	//Separate accepting states from states 

	StateSet acceptingStateSet = old.states_accepting();
	IntSet _accepting;
	for (StateSet::const_iterator i= acceptingStateSet.begin(); i!=acceptingStateSet.end(); ++i){
		_accepting.insert(*i);
	}


	StateSet startingStateSet = old.states_starting();
	IntSet _starting;
	for (StateSet::const_iterator i= startingStateSet.begin(); i!=startingStateSet.end(); ++i){
		_starting.insert(*i);
	}

	
	for (unsigned int j= 0;  j < n; j++) {
		
		
		if ( _accepting.find(j) != _accepting.end() ){
 
			F.insert(j);
			
		} else {

			QF.insert(j);
		}
	}

   
 
	P.push_back(QF);
	P.push_back(F);
 
	while (P_old != P)     
		
	{
		vector < IntSet > P_new;
	
      
		for (vector<IntSet>::const_iterator i_part = P.begin(); i_part != P.end(); ++i_part){
			IntSet Q_p1 = *i_part;
			
			for (IntSet::const_iterator i_q = Q_p1.begin(); i_q != Q_p1.end(); ++i_q){
				int q1 = *i_q;
				vector<SymbolSet> v; 

				for (vector<IntSet>::const_iterator j_part = P.begin(); j_part != P.end(); ++j_part){
					IntSet Q_p2 = *j_part;
					
					//create empty symbolset
					SymbolSet sigma(SymbolSet::empty(old.alphabet()));
					
					for (IntSet::const_iterator j_q = Q_p2.begin();j_q != Q_p2.end();++j_q){
						
						int q2 = *j_q;
						// sigma= sigma U _transition_matrix
						
						sigma |= old.edge_between(q1, q2);
		     
					}

					v.push_back(sigma);
					
					
				}
				
				states_for_vector[v].insert(q1);
	      
				
			}
			
			hash_map<vector<SymbolSet>, IntSet, vectorHash>:: const_iterator map_iter;
			for (map_iter=states_for_vector.begin(); map_iter!= states_for_vector.end(); map_iter++ ){
				
				P_new.push_back((*map_iter).second);
				
				transitions_to_partitions_new.push_back((*map_iter).first);
			} 
	 
			states_and_symbols = states_for_vector;  
			states_for_vector.clear();
	  
		}
		

		P_old = P;
		P = P_new;
		transitions_to_partitions = transitions_to_partitions_new;
		transitions_to_partitions_new.clear();
	}// while (P_old != P) 
  
    
	hash_map<vector<SymbolSet>, IntSet, vectorHash> :: const_iterator hash_map_iter;
	vector<SymbolSet> symbol_set_vector;
	vector < vector<SymbolSet> >:: const_iterator sym_iter; 
	vector<SymbolSet>:: const_iterator symbol_set_iter;
	bool is_accepting;
	bool is_starting;
	IntSet::iterator find_state;
	vector<IntSet> :: const_iterator partition_iter;
	int no_of_states=0;
	int col=0;
	IntSet new_accepting;
	IntSet new_starting;
	
	for (partition_iter = P.begin(); partition_iter != P.end(); partition_iter++ ){
		
		col++;
		IntSet states=  *partition_iter;
		is_accepting=false; 
		is_starting=false;
		IntSet:: const_iterator state_set_iter;
	
		for (state_set_iter =states.begin(); state_set_iter!= states.end(); state_set_iter++){
			

			if( _starting.find(*state_set_iter) != _starting.end()){
	    
				is_starting=true;
			} 
			if(_accepting.find(*state_set_iter) != _accepting.end() ){
				
				is_accepting=true;
			}
			
		}

		

		State q2=  res->add_state(is_accepting, is_starting);
		no_of_states++;
		
	}

	
	hash_map<vector<SymbolSet>, IntSet, vectorHash> :: const_iterator hash_map_iterator;
	int state_from=0;
	
	for( sym_iter= transitions_to_partitions.begin();  sym_iter!= transitions_to_partitions.end(); sym_iter++ ){
		int state_to= 0;  
		symbol_set_vector = *sym_iter;
		
  
		for(symbol_set_iter = symbol_set_vector.begin(); symbol_set_iter!= symbol_set_vector.end(); symbol_set_iter++){
			
			res-> add_edge( state_from,  *symbol_set_iter , state_to);
     			state_to++;
		}
		
		

		state_from++;
    
	}
 
  
	
  
	return res;
}
}



