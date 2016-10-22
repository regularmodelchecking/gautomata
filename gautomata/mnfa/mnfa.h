#ifndef GAUTOMATA_MNFA_H
#define GAUTOMATA_MNFA_H
#include <set>
#include <gautomata/nfa/nfa.h>
#include <gbdd/gbdd.h>
namespace gautomata
{
	using gbdd::Space;
	using gbdd::Domain;

	typedef gbdd::Space::VarMap VarMap;
	typedef std::vector<vector<SymbolSet> > TransitionMatrix;
	typedef std::set<int, less<int> > IntSet;
	class MNfa : public Nfa
	{
		Space *_space;
		TransitionMatrix _transition_matrix;
		StateSet _states;
		IntSet _starting;
		IntSet _accepting;



		bool is_a_member(unsigned int state);
		void print();
		IntSet starting_set_of_states() const;
		State get_n_states();
		void set_n_states(unsigned int new_n_states);


	public:
		class Factory : public Nfa:: Factory
		  {
		    Space* _space;
		  public:
			Factory(Space* space);
			~Factory() {}
			
			MNfa* ptr_empty() const;
			MNfa* ptr_universal() const;
			MNfa* ptr_epsilon() const;
			MNfa* ptr_symbol(SymbolSet sym) const;
		  };

		Factory* ptr_factory() const;

		MNfa(Space* space);
		MNfa(Space* space, bool v);
		MNfa(const MNfa& mnfa);
		MNfa(const Nfa& nfa);

		~MNfa() {}
		
		MNfa* ptr_clone() const;

		SymbolSet alphabet() const;
		StateSet states() const;
		StateSet states_starting() const;
		StateSet states_accepting() const;
		Space* get_space() const;

		MNfa deterministic() const;
		MNfa minimize() const;
		TransitionMatrix get_transition_matrix() const;
		IntSet accepting_set_of_states() const;

		// Explicit Construction
		StateSet successors(StateSet q, SymbolSet on) const;
		SymbolSet edge_between(State q, State r) const;
		State add_state(bool accepting, bool starting= false);
		void add_edge(State from, SymbolSet on, State to);

		static MNfa epsilon(Space* space);
		static MNfa symbol(Space* space, SymbolSet sym);
	};
}		


#endif /* GAUTOMATA_MNFA_H */














