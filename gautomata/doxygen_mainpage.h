/*
 * doxygen_mainpage.h: 
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
 *    Marcus Nilsson (marcusn@docs.uu.se)
 */

#ifndef DOXYGEN_MAINPAGE_H
#define DOXYGEN_MAINPAGE_H

/** \mainpage GAUTOMATA - A package for automata with BDD based alphabets

\section introduction Introduction

gautomata is a package for finite-state automata with alphabets represented
as BDDs. It is based on the gbdd package.

The general interface is exported by the gautomata::Nfa, which
contains the interface gautomata::Nfa::Factory for creating automata
for a given implementation and a number of methods on automata such as
concatenation, determinization, minimization and so on.

Every implementation inherits from gautomata::Nfa and needs to define:

- A factory used to create automata with this implementation
- Methods returning alphabet, sets of states, set of starting states and
  set of accepting states.
- Method adding state
- Implement one of:
  - Method to return set of symbols between two sets of states
  - Method to return set of symbols between two states
  - Method to return the transition relation
- Implement on of:
  - Method to add edge between sets of states
  - Method to add edge between two states
  - Method to add transitions given a relation
- Currently, minimization and determinization also have to be defined

All other operations have default implementations inherited from gautomata::Nfa. The
case where there is a choice of defining one of several methods, there are default
implementation of all of them, but they are defined in terms of each other.

Factories are useful for implementing general procedures that are
independent of Nfa implementation. Examples include procedures for
testing and measurements.

Current implementations are:

- gautomata::BNfa, entire transition relation implemented as BDD
- gautomata::RefNfa, wrapper around gautomata::Nfa pointer

gautomata::RefNfa is just a wrapper around a pointer to a gautomata::Nfa and is a technique
to get value semantics instead of pointer semantics for methods that return automata.

*/


#endif /* DOXYGEN_MAINPAGE_H */
