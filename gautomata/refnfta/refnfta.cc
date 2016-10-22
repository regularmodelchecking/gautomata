/*
 * refnfta.cc: 
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
 *    Marcus Nilsson (marcus@docs.uu.se)
 *    Julien d'Orso  (juldor@it.uu.se)
 */

#include "refnfta.h"

namespace gautomata
{

   RefNfta::Factory::Factory(Nfta::Factory* factory):
    ptr_factory(factory)
  {}

  RefNfta::Factory::~Factory()
  {}

  const Nfta& RefNfta::follow_if_refnfta(const Nfta& a)
  {
    try
      {
	return *(dynamic_cast<const RefNfta&>(a)).ptr_nfta;
      }
    catch (...)
      {
	return a;
      }
  }

  Nfta* RefNfta::Factory::ptr_empty(unsigned int max_arity) const
  {
    return ptr_factory->ptr_empty(max_arity);
  }

  Nfta* RefNfta::Factory::ptr_universal(unsigned int max_arity) const
  {
    return ptr_factory->ptr_universal(max_arity);
  }
	
  Nfta* RefNfta::Factory::ptr_symbol(unsigned int max_arity, SymbolSet sym) const
  {
    return ptr_factory->ptr_symbol(max_arity, sym);
  }

  Nfta*  RefNfta::Factory::ptr_symbol_with_children(unsigned int max_arity,
					  SymbolSet sym,
					  unsigned int arity,
					  vector<const Nfta*> children) const
  {
    return ptr_factory->ptr_symbol_with_children(max_arity, sym,
						 arity, children);
  }  


  RefNfta::RefNfta(Nfta* nfta):
    ptr_nfta(nfta)
  {}

  RefNfta::RefNfta(const RefNfta& ref_nfta):
    ptr_nfta(ref_nfta.ptr_clone())
  {}

  RefNfta& RefNfta::operator= (const RefNfta& a)
  {
    Nfta* ptr_a = a.ptr_clone();

    ptr_nfta = ptr_a;

    return *this;
  }
  

  RefNfta::~RefNfta()
  {}

  Nfta* RefNfta::ptr_clone() const
  {
    return ptr_nfta->ptr_clone();
  }

  Nfta::Factory*  RefNfta::ptr_factory() const
  {
    return ptr_nfta->ptr_factory();
  }


  unsigned int RefNfta::max_arity() const
  {
    return ptr_nfta->max_arity();
  }


  SymbolSet RefNfta::alphabet() const
  {
    return ptr_nfta->alphabet();
  }

  StateSet RefNfta::states() const
  {
    return ptr_nfta->states();
  }


  Relation RefNfta::transitions(unsigned int arity) const
  {
    return ptr_nfta->transitions(arity);
  }

  StateSet RefNfta::states_accepting() const
  {
    return ptr_nfta->states_accepting();
  }

  Nfta* RefNfta::ptr_product(const Nfta& a2,
				      bool (*fn)(bool v1, bool v2)) const
  {
    return ptr_nfta->ptr_product(follow_if_refnfta(a2), fn);
  }

  Nfta* RefNfta::ptr_concatenation(const Nfta& a2, SymbolSet sym) const
  {
    return ptr_nfta->ptr_concatenation(follow_if_refnfta(a2), sym);
  }

  // Unary operations

  Nfta*  RefNfta::ptr_with_accepting(StateSet accepting) const
  {
    return ptr_nfta->ptr_with_accepting(accepting);
  }



  Nfta* RefNfta::ptr_complete() const
  {
    return ptr_nfta->ptr_complete();
  }

Nfta* RefNfta::ptr_determinize() const
{
  return ptr_nfta->ptr_determinize();
}

Nfta* RefNfta::ptr_minimize() const
{
  return ptr_nfta->ptr_minimize();
}

Nfta* RefNfta::ptr_project(Domain vs) const
{
  return ptr_nfta->ptr_project(vs);
}

Nfta* RefNfta::ptr_rename(VarMap map) const
{
  return ptr_nfta->ptr_rename(map);
}

Nfta* RefNfta::ptr_rename(Domain vs1, Domain vs2) const
{
  return ptr_nfta->ptr_rename(vs1,vs2);
}

Nfta* RefNfta::ptr_kleene(SymbolSet sym) const
{
  return ptr_nfta->ptr_kleene(sym);
}

Nfta* RefNfta::ptr_remove_leaves(SymbolSet leaves) const
{
  return ptr_nfta->ptr_remove_leaves(leaves);
}

//  Nfta* RefNfta::ptr_reverse() const
//  {
//  	return ptr_nfta->ptr_reverse();
//  }
  Nfta* RefNfta::ptr_negate(const Nfta &a) const
  {
    return ptr_nfta->ptr_negate(a);
  }


Nfta* RefNfta::ptr_filter_states(StateSet s) const
{
	return ptr_nfta->ptr_filter_states(s);
}

Nfta* RefNfta::ptr_filter_states_live() const
{
	return ptr_nfta->ptr_filter_states_live();
}

Nfta* RefNfta::ptr_filter_states_reachable() const
{
	return ptr_nfta->ptr_filter_states_reachable();
}

Nfta* RefNfta::ptr_filter_states_productive() const
{
	return ptr_nfta->ptr_filter_states_productive();
}

// Explicit Construction

State RefNfta::add_state(bool accepting)
{
  return ptr_nfta->add_state(accepting);
}

void RefNfta::add_edge(unsigned int arity, vector<StateSet> from, SymbolSet on, StateSet to)
{
	return ptr_nfta->add_edge(arity, from, on, to);
}

void RefNfta::add_edge(unsigned int arity, vector<State> from, SymbolSet on, State to)
{
	return ptr_nfta->add_edge(arity, from, on, to);
}

void RefNfta::add_transitions(unsigned int arity, Relation new_transitions)
{
	return ptr_nfta->add_transitions(arity, new_transitions);
}

bool RefNfta::is_true() const
{
	return ptr_nfta->is_true();
}

bool RefNfta::is_false() const
{
	return ptr_nfta->is_false();
}

// Value semantics versions of operations

RefNfta operator&(const RefNfta &a1, const RefNfta &a2)
{
	return RefNfta(a1.ptr_nfta->ptr_product_and(*a2.ptr_nfta));
}

RefNfta operator|(const RefNfta &a1, const RefNfta &a2)
{
	return RefNfta(a1.ptr_nfta->ptr_product_or(*a2.ptr_nfta));
}

RefNfta operator-(const RefNfta &a1, const RefNfta &a2)
{
	return RefNfta(a1.ptr_nfta->ptr_product_minus(*a2.ptr_nfta));
}

//  RefNfta operator*(const RefNfta& a1, const RefNfta& a2)
//  {
//  	return RefNfta(a1.ptr_nfta->ptr_concatenation(*a2.ptr_nfta));
//  }

RefNfta RefNfta::determinize() const
{
	return RefNfta(ptr_determinize());
}

RefNfta RefNfta::minimize() const
{
	return RefNfta(ptr_minimize());
}

RefNfta RefNfta::project(Domain vs) const
{
	return RefNfta(ptr_project(vs));
}

RefNfta RefNfta::rename(VarMap map) const
{
	return RefNfta(ptr_rename(map));
}

RefNfta RefNfta::rename(Domain vs1, Domain vs2) const
{
	return RefNfta(ptr_rename(vs1, vs2));
}


RefNfta RefNfta::kleene(SymbolSet sym) const
{
	return RefNfta(ptr_kleene(sym));
}


RefNfta RefNfta::remove_leaves(SymbolSet leaves) const
{
	return RefNfta(ptr_remove_leaves(leaves));
}

//  RefNfta RefNfta::reverse() const
//  {
//  	return RefNfta(ptr_reverse());
//  }

//  RefNfta RefNfta::negate() const
//  {
    /*    RefNfta res = RefNfta(a);
    res = res.complete();
    res=res.determinize();
    StateSet new_acc = res.states() - res.states_accepting();
    res =res.with_accepting(new_acc);
    return res;*/

//    return RefNfta(ptr_negate());
//  }


RefNfta RefNfta::with_accepting(StateSet accepting) const
{
	return RefNfta(ptr_with_accepting(accepting));
}



RefNfta RefNfta::filter_states(StateSet s) const
{
	return RefNfta(ptr_filter_states(s));
}

RefNfta RefNfta::filter_states_live() const
{
	return RefNfta(ptr_filter_states_live());
}

RefNfta RefNfta::filter_states_reachable() const
{
	return RefNfta(ptr_filter_states_reachable());
}

RefNfta RefNfta::filter_states_productive() const
{
	return RefNfta(ptr_filter_states_productive());
}


}
