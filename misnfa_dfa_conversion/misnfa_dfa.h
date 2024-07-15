#ifndef MISNFA_DFA_H
#define MISNFA_DFA_H
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>
#include <cassert>
using namespace std;

using State = unsigned int;
using Symbol = char;

struct MISNFA {
    set<State> m_States;
    set<Symbol> m_Alphabet;
    map<pair<State, Symbol>, set<State>> m_Transitions;
    set<State> m_InitialStates;
    set<State> m_FinalStates;
};

struct DFA {
    set<State> m_States;
    set<Symbol> m_Alphabet;
    map<pair<State, Symbol>, State> m_Transitions;
    State m_InitialState;
    set<State> m_FinalStates;

    bool operator==(const DFA& dfa)
    {
        return tie(m_States, m_Alphabet, m_Transitions, m_InitialState, m_FinalStates) == tie(dfa.m_States, dfa.m_Alphabet, dfa.m_Transitions, dfa.m_InitialState, dfa.m_FinalStates);
    }
};
#endif