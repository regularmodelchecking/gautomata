/*
 * test-bnfta.cc:
 *      A test suite for the bnfta implementation.
 *
 * Authors:
 *      Julien d'Orso (julien.dorso@it.uu.se)
 */

#include <gautomata/gautomata.h>

#include <iostream>

using namespace std;

using namespace gautomata;
using gbdd::Bdd;
using gbdd::Space;
using gbdd::Domain;

static Space* space = Space::create_default();

static bool test1()
{
  Set alphabet = Set(Domain(0, 2), Bdd(space, true));

  SymbolSet s_0 = Set(alphabet, 0);
  SymbolSet s_1 = Set(alphabet, 1);
  SymbolSet s_2 = Set(alphabet, 2);
  SymbolSet s_3 = Set(alphabet, 3);
  
  Bnfta nfta0(space, 2);
  {
    State q0 = nfta0.add_state(false);
    State q1 = nfta0.add_state(true);
    vector<State> v1, v2, v3;

    nfta0.add_edge(0, vector<State>(), s_0, q0);
    v1.push_back(q0); v1.push_back(q0);
    nfta0.add_edge(2, v1, s_2, q1);
    v2.push_back(q1); v2.push_back(q0);
    nfta0.add_edge(2, v2, s_2, q1);
    v3.push_back(q0); v3.push_back(q1);
    nfta0.add_edge(2, v3, s_2, q1);
  }


    Bnfta nfta1(space, 2);
    {
    State q0 = nfta1.add_state(false);
    State q1 = nfta1.add_state(true);
    vector<State> v1, v2, v3;

    nfta1.add_edge(0, vector<State>(), s_0 | s_1, q0);
    v1.push_back(q0); v1.push_back(q0);
    nfta1.add_edge(2, v1, s_2, q1);
    v2.push_back(q1); v2.push_back(q0);
    nfta1.add_edge(2, v2, s_2, q1);
    v3.push_back(q0); v3.push_back(q1);
    nfta1.add_edge(2, v3, s_2, q1);
    }


    Bnfta intersect_0_1 = (nfta0 & nfta1);

    //  cout << "nfta0=" << nfta0 << endl;
  // cout << "intersect_0_1=" << intersect_0_1 << endl;

    //   Bnfta det0 = nfta0.determinize();

  // cout << "det0.states(): " << Bdd(det0.states()) << endl;
  //  cout << "det0.states_acceptintg(): " <<  Bdd(det0.states_accepting()) << endl;

    //      cout << "det0="  << det0 << endl;

    return (intersect_0_1 == nfta0);
    // return true;
}


static bool test2()
{
        Set alphabet = Set(Domain(0, 2), Bdd(space, true));

	SymbolSet s_0 = Set(alphabet, 0);
	SymbolSet s_1 = Set(alphabet, 1);
	SymbolSet s_2 = Set(alphabet, 2);
	SymbolSet s_3 = Set(alphabet, 3);

	Bnfta nfta0(space, 2);
	{
	  State q0 = nfta0.add_state(false);
	  State q1 = nfta0.add_state(true);
	  vector<State> v1, v2, v3;

	  nfta0.add_edge(0, vector<State>(), s_0 | s_1, q0);
	  v1.push_back(q0); v1.push_back(q0);
	  nfta0.add_edge(2, v1, s_2, q1);
	  v2.push_back(q1); v2.push_back(q0);
	  nfta0.add_edge(2, v2, s_2, q1);
	  v3.push_back(q0); v3.push_back(q1);
	  nfta0.add_edge(2, v3, s_2, q1);
	}


	Bnfta nfta1(space, 2);
	{
	  State q0 = nfta1.add_state(false);
	  State q1 = nfta1.add_state(true);
	  vector<State> v1, v2, v3;

	  nfta1.add_edge(0, vector<State>(), s_0, q0);
	  v1.push_back(q0); v1.push_back(q0);
	  nfta1.add_edge(2, v1, s_2, q1);
	  v2.push_back(q1); v2.push_back(q0);
	  nfta1.add_edge(2, v2, s_2, q1);
	  v3.push_back(q0); v3.push_back(q1);
	  nfta1.add_edge(2, v3, s_2, q1);
	}

	Bnfta nfta2(space, 2);
	{
	  State q0 = nfta2.add_state(false);
	  State q1 = nfta2.add_state(true);
	  vector<State> v1, v2, v3;

	  nfta2.add_edge(0, vector<State>(), s_1, q0);
	  v1.push_back(q0); v1.push_back(q0);
	  nfta2.add_edge(2, v1, s_2, q1);
	  v2.push_back(q1); v2.push_back(q0);
	  nfta2.add_edge(2, v2, s_2, q1);
	  v3.push_back(q0); v3.push_back(q1);
	  nfta2.add_edge(2, v3, s_2, q1);
	}


	Bnfta difference = (nfta0 - nfta1);

	//	 cout << "nfta2=" << nfta2 << endl;
	// cout << "difference=" << difference << endl;


        return (difference > nfta2);
}



static bool test3()
{
  Set alphabet = Set(Domain(0, 1), Bdd(space, true));

  SymbolSet a = Set(alphabet, 0);
  SymbolSet b = Set(alphabet, 1);
  
  
  Bnfta nfta0(space, 2);
  {
    State q0 = nfta0.add_state(false);
    State q1 = nfta0.add_state(false);
    State q2 = nfta0.add_state(true);
    vector<State> v1, v2, v3;

    nfta0.add_edge(0, vector<State>(), a, q0);
    nfta0.add_edge(0, vector<State>(), b, q1);
    v1.push_back(q0); v1.push_back(q0);
    nfta0.add_edge(2, v1, b, q1);
    v2.push_back(q1); v2.push_back(q0);
    nfta0.add_edge(2, v2, a, q2);
    v3.push_back(q0); v3.push_back(q2);
    nfta0.add_edge(2, v3, a, q2);
  }


    Bnfta nfta1(space, 2);
    {
      State p0 = nfta1.add_state(false);
      State p1 = nfta1.add_state(false);
      State p2 = nfta1.add_state(true);
      State p3 = nfta1.add_state(true);
      State p4 = nfta1.add_state(true);
      vector<State> w1, w2, w3, w4, w5, w6;

      nfta1.add_edge(0, vector<State>(), a, p0); // -a->q0
      nfta1.add_edge(0, vector<State>(), b, p1);// -b->q1
      w1.push_back(p0); w1.push_back(p0);
      nfta1.add_edge(2, w1, b, p1);             // q0,q0-b->q1

      w2.push_back(p1); w2.push_back(p0);
      nfta1.add_edge(2, w2, a, p2);      //q1,q0-a->q2

      w3.push_back(p0); w3.push_back(p2);
      nfta1.add_edge(2, w3, a, p2);      //q0,q2-a->q2
      w4.push_back(p0); w4.push_back(p2);
      nfta1.add_edge(2, w4, a, p3);     //q0.q2-a->q3
      

      w5.push_back(p0); w5.push_back(p3);
      nfta1.add_edge(2, w5, a, p4);      //q0,q3-a->q4
      
      w6.push_back(p0); w6.push_back(p4);  //q0,q4-a->q3
      nfta1.add_edge(2, w6, a, p3);
    
    }
    //  cout << "nfta0=" << nfta0 << endl;
    //cout << "nfta1=" << nfta1 << endl;

    Bnfta intersect_0_1 =  nfta1.minimize();

    cout << "intersect_0_1=" << intersect_0_1 << endl;

    //   Bnfta det0 = nfta0.determinize();

  // cout << "det0.states(): " << Bdd(det0.states()) << endl;
  //  cout << "det0.states_acceptintg(): " <<  Bdd(det0.states_accepting()) << endl;

    //      cout << "det0="  << det0 << endl;

    return (intersect_0_1 == nfta0);
    // return true;
}

static bool test4()
{
  Set alphabet = Set(Domain(0, 1), Bdd(space, true));

  SymbolSet a = Set(alphabet, 0);
  SymbolSet b = Set(alphabet, 1);
  
  
  Bnfta nfta0(space, 2);
  {
    State q0 = nfta0.add_state(false);
    State q1 = nfta0.add_state(false);
    State q2 = nfta0.add_state(true);
    vector<State> v1, v2, v3;

    nfta0.add_edge(0, vector<State>(), a, q0);
    nfta0.add_edge(0, vector<State>(), b, q1);
    v1.push_back(q0); v1.push_back(q0);
    nfta0.add_edge(2, v1, b, q1);
    v2.push_back(q1); v2.push_back(q0);
    nfta0.add_edge(2, v2, a, q2);
    v3.push_back(q0); v3.push_back(q2);
    nfta0.add_edge(2, v3, a, q2);
  }


Bnfta nfta1(space, 2);
  {
    State q0 = nfta1.add_state(false);
    State q1 = nfta1.add_state(false);
    State q2 = nfta1.add_state(true);
    vector<State> v1, v2, v3;

    nfta1.add_edge(0, vector<State>(), a, q0);
    nfta1.add_edge(0, vector<State>(), b, q1);
    v1.push_back(q0); v1.push_back(q0);
    nfta1.add_edge(2, v1, b, q1);
    v2.push_back(q0); v2.push_back(q1);
    nfta1.add_edge(2, v2, a, q2);
    v3.push_back(q0); v3.push_back(q2);
    nfta1.add_edge(2, v3, a, q2);
  }


    
    //  cout << "nfta0=" << nfta0 << endl;
    //cout << "nfta1=" << nfta1 << endl;
    return !((nfta0>=nfta1)||(nfta0 <= nfta1));
    // return true;
}

int main(int argc, char** argv)
{
  struct {
    const char* name;
    bool (*test_f)(void);
  } tests[] = {
    {"Operators & and ==", test1},
    {"Operators - and >", test2},
    {"minimize         ", test3},
    {"determinize      ", test4}
  };

  int i;

  for(i=0; i < sizeof(tests)/ sizeof(tests[0]); ++i)
  {
    cout << tests[i].name << "...";
    cout.flush();

    cout << ((tests[i].test_f()) ? "Ok" : "Fail");

    cout << endl;
  }

 
  return 0;
}
