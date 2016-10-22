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
using gbdd::Set;
using gbdd::Relation;

static Space* space = Space::create_default();

static bool test1()
{
  Domain dom(0, 4);
  Domain dom1(0,2);
  Domain dom2(2,2);

  Bdd d1_s0 = Bdd::value(space, dom1, 0);
  Bdd d1_s1 = Bdd::value(space, dom1, 1);
  Bdd d1_s2 = Bdd::value(space, dom1, 2);
  Bdd d1_s3 = Bdd::value(space, dom1, 3);

  Bdd d2_s0 = Bdd::value(space, dom2, 0);
  Bdd d2_s1 = Bdd::value(space, dom2, 1);
  Bdd d2_s2 = Bdd::value(space, dom2, 2);
  Bdd d2_s3 = Bdd::value(space, dom2, 3);
  
  Bnfta nfta01(space, 2);
  {
    State q0 = nfta01.add_state(false);
    State q1 = nfta01.add_state(false);
    State q2 = nfta01.add_state(true);
    vector<State> v1, v2, v3;

    nfta01.add_edge(0, vector<State>(), Set(dom, d1_s0 & d2_s1), q0);

    nfta01.add_edge(0, vector<State>(), Set(dom, d1_s0 & d2_s3), q1);

    v1.push_back(q0); v1.push_back(q0);
    nfta01.add_edge(2, v1, Set(dom, d1_s1 & d2_s2), q1);

    v2.push_back(q1); v2.push_back(q0);
    nfta01.add_edge(2, v2, Set(dom, d1_s0 & d2_s0), q2);

    v3.push_back(q2); v3.push_back(q0);
    nfta01.add_edge(2, v3, Set(dom, d1_s0 & d2_s2), q2);

    //    cout<< nfta01 << endl; 
  }

  Bnfta nfta12(space, 2);
  {
    State q0 = nfta12.add_state(false);
    State q1 = nfta12.add_state(false);
    State q2 = nfta12.add_state(true);
    vector<State> v1, v2, v3;

    nfta12.add_edge(0, vector<State>(), Set(dom, d1_s1 & d2_s3), q0);

    nfta12.add_edge(0, vector<State>(), Set(dom, d1_s3 & d2_s0), q1);

    v1.push_back(q0); v1.push_back(q0);
    nfta12.add_edge(2, v1, Set(dom, d1_s2 & d2_s0), q1);

    v2.push_back(q1); v2.push_back(q0);
    nfta12.add_edge(2, v2, Set(dom, d1_s0 & d2_s1), q2);

    v3.push_back(q2); v3.push_back(q0);
    nfta12.add_edge(2, v3, Set(dom, d1_s2 & d2_s3), q2);

    // cout<< nfta12 << endl; 
   }

   Bnfta nfta02(space, 2);
  {
    State q0 = nfta02.add_state(false);
    State q1 = nfta02.add_state(false);
    State q2 = nfta02.add_state(true);
    vector<State> v1, v2, v3;

    nfta02.add_edge(0, vector<State>(), Set(dom, d1_s0 & d2_s3), q0);

    nfta02.add_edge(0, vector<State>(), Set(dom, d1_s0 & d2_s0), q1);

    v1.push_back(q0); v1.push_back(q0);
    nfta02.add_edge(2, v1, Set(dom, d1_s1 & d2_s0), q1);

    v2.push_back(q1); v2.push_back(q0);
    nfta02.add_edge(2, v2, Set(dom, d1_s0 & d2_s1), q2);

    v3.push_back(q2); v3.push_back(q0);
    nfta02.add_edge(2, v3, Set(dom, d1_s0 & d2_s3), q2);

    //    cout<< nfta02 << endl; 
 }

  Domains domains = dom1 * dom2;

  TreeRegularRelation rel01(domains, nfta01);
  TreeRegularRelation rel12(domains, nfta12);

  //  cout << rel01 << endl;

  return nfta02 == Bnfta (rel01.compose(1, rel12).get_automaton());
 
}

static bool test2()
{
  Domain dom(0, 4);
  Set sv0 = Set(dom, Bdd::var_true(space, 0));
  Set sv1 = Set(dom, Bdd::var_true(space, 1));
  Set sv2 = Set(dom, Bdd::var_true(space, 2));
  Set sv3 = Set(dom, Bdd::var_true(space, 3));
  
Bnfta nfta01(space, 2);
  {
    State q0 = nfta01.add_state(false);
    State q1 = nfta01.add_state(false);
    State q2 = nfta01.add_state(true);
    vector<State> v1, v2, v3;

    nfta01.add_edge(0, vector<State>(), sv1, q0);

    nfta01.add_edge(0, vector<State>(), sv1, q1);

    v1.push_back(q0); v1.push_back(q0);
    nfta01.add_edge(2, v1,sv0, q1);

    v2.push_back(q1); v2.push_back(q0);
    nfta01.add_edge(2, v2, sv1 , q2);

    v3.push_back(q2); v3.push_back(q0);
    nfta01.add_edge(2, v3,sv0 , q2);

    //    cout<< nfta01 << endl; 
  }

 Bnfta nfta23(space, 2);
  {
    State q0 = nfta23.add_state(false);
    State q1 = nfta23.add_state(false);
    State q2 = nfta23.add_state(true);
    vector<State> v1, v2, v3;

    nfta23.add_edge(0, vector<State>(), sv3, q0);

    nfta23.add_edge(0, vector<State>(), sv3, q1);

    v1.push_back(q0); v1.push_back(q0);
    nfta23.add_edge(2, v1, sv2, q1);

    v2.push_back(q1); v2.push_back(q0);
    nfta23.add_edge(2, v2,sv3 , q2);

    v3.push_back(q2); v3.push_back(q0);
    nfta23.add_edge(2, v3, sv2, q2);

    //    cout<< nfta01 << endl; 
  }



	Domains domains0 = Domain(0, 1) * Domain(1, 1);
	Domains domains2 = Domain(2, 1) * Domain(3, 1);

	TreeRegularRelation rel0(domains0, nfta01);
	TreeRegularRelation rel2(domains2, nfta23);

	return
		(nfta01 != nfta23) &&
		(rel0 == rel2) &&
		TreeRegularRelation(domains2, rel0).get_automaton() == static_cast<const Nfta&>(nfta23);

 
}


int main(int argc, char** argv)
{
  struct {
    const char* name;
    bool (*test_f)(void);
  } tests[] = {
    {"Composition ", test1},
    {"Renaming    ", test2},
    //{"minimize         ", test3},
    //{"determinize      ", test4}
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
