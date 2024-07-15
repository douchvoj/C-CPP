#include "misnfa_dfa.h"

// Function to check if a given NFA is already a DFA
bool isDFA(const MISNFA& nfa) {
    for (const auto& transition : nfa.m_Transitions) {
        if (transition.second.size() > 1) return false;
    }
    return true;
}

// Function to print the details of a DFA
void printDFA(const DFA& dfa) {
    cout << "States: ";
    for (const auto& state : dfa.m_States) cout << state << " ";
    cout << "\nAlphabet: ";
    for (const auto& symbol : dfa.m_Alphabet) cout << symbol << " ";
    cout << "\n------------\n";
    for (const auto& transition : dfa.m_Transitions) {
        cout << transition.first.first << " | " << transition.first.second << " | " << transition.second << "\n";
    }
    cout << "------------\nStarting point: " << dfa.m_InitialState << "\nTerminating points: ";
    for (const auto& state : dfa.m_FinalStates) cout << state << " ";
    cout << endl;
}

// Function to print the details of an NFA
void printNFA(const MISNFA& nfa) {
    cout << "States: ";
    for (const auto& state : nfa.m_States) cout << state << " ";
    cout << "\nAlphabet: ";
    for (const auto& symbol : nfa.m_Alphabet) cout << symbol << " ";
    cout << "\n------------\n";
    for (const auto& transition : nfa.m_Transitions) {
        cout << transition.first.first << " | " << transition.first.second << " | ";
        for (const auto& state : transition.second) cout << state << " | ";
        cout << "\n";
    }
    cout << "------------\nStarting point: " << *nfa.m_InitialStates.begin() << "\nTerminating points: ";
    for (const auto& state : nfa.m_FinalStates) cout << state << " ";
    cout << endl;
}

// Function to determine the initial states 
MISNFA getStartingPoint(const MISNFA& nfa, MISNFA& tmp) {
    // the NFA has only one initial state,
    if (nfa.m_InitialStates.size() == 1) {
        tmp.m_InitialStates.insert(*nfa.m_InitialStates.begin());
    } else {
        // More than one initial state, create a new initial state
        State new_start = nfa.m_States.size() + 1;
        tmp.m_InitialStates.insert(new_start);
        tmp.m_States.insert(new_start);

        // Check if any of the original initial states are also final states
        for (const auto& init_state : nfa.m_InitialStates) {
            if (nfa.m_FinalStates.find(init_state) != nfa.m_FinalStates.end()) {
                tmp.m_FinalStates.insert(new_start);
            }
        }

        // Create transitions from the new initial state to the original initial states
        for (const auto& transition : nfa.m_Transitions) {
            for (const auto& init_state : nfa.m_InitialStates) {
                if (init_state == transition.first.first) {
                    tmp.m_Transitions.insert({{new_start, transition.first.second}, transition.second});
                }
            }
        }

        // Add all transitions from the original initial states to the new start state transitions
        for (const auto& init_state : nfa.m_InitialStates) {
            for (const auto& transition : nfa.m_Transitions) {
                if (init_state == transition.first.first) {
                    for (auto& new_transition : tmp.m_Transitions) {
                        if (new_transition.first.second == transition.first.second) {
                            for (const auto& state : transition.second) {
                                new_transition.second.insert(state);
                            }
                        }
                    }
                }
            }
        }
    }

    // Add the original transitions 
    for (const auto& transition : nfa.m_Transitions) {
        tmp.m_Transitions.insert({{transition.first.first, transition.first.second}, transition.second});
    }

    return tmp;
}

// Function to remove transitions that do not lead to states in the NFA
MISNFA getRidOfTransitions(MISNFA& nfa) {
    map<pair<State, Symbol>, set<State>> transitions;
    for (const auto& transition : nfa.m_Transitions) {
        if (nfa.m_States.find(transition.first.first) != nfa.m_States.end() && nfa.m_States.find(*transition.second.begin()) != nfa.m_States.end()) {
            transitions[transition.first] = transition.second;
        }
    }
    nfa.m_Transitions = transitions;
    return nfa;
}

// Function to remove useless states 
MISNFA removeUseless(MISNFA& tmp) {
    tmp.m_States.clear();
    queue<State> stateQueue;
    for (const auto& finalState : tmp.m_FinalStates) {
        tmp.m_States.insert(finalState);
        stateQueue.push(finalState);
    }
    while (!stateQueue.empty()) {
        State currentState = stateQueue.front();
        stateQueue.pop();
        for (const auto& transition : tmp.m_Transitions) {
            const auto& transitionTargetStates = transition.second;
            const auto& transitionSourceState = transition.first.first;
            if (transitionTargetStates.count(currentState) > 0 && tmp.m_States.insert(transitionSourceState).second) {
                stateQueue.push(transitionSourceState);
            }
        }
    }
    tmp = getRidOfTransitions(tmp);
    return tmp;
}

// Function to remove transitions that do not lead to states in the given set of states
MISNFA removeTransitions(MISNFA& nfa, const set<State>& states) {
    map<pair<State, Symbol>, set<State>> transitions;
    for (const auto& transition : nfa.m_Transitions) {
        if (states.find(transition.first.first) != states.end()) {
            transitions[transition.first] = transition.second;
        }
    }
    nfa.m_Transitions = transitions;
    return nfa;
}

// Function to remove unreachable states from the NFA
MISNFA removeUnreachableStates(MISNFA& nfa) {
    queue<State> queue;
    set<State> reachableStates;
    State startState = *nfa.m_InitialStates.begin();
    reachableStates.insert(startState);
    queue.push(startState);

    while (!queue.empty()) {
        State state = queue.front();
        queue.pop();
        for (const auto& transition : nfa.m_Transitions) {
            if (state == transition.first.first) {
                for (const auto& nextState : transition.second) {
                    if (reachableStates.insert(nextState).second) {
                        queue.push(nextState);
                    }
                }
            }
        }
    }

    nfa.m_States = reachableStates;
    return removeTransitions(nfa, reachableStates);
}

// Function to get transitions for a given set of states and a symbol
set<State> getTransitions(const MISNFA& nfa, Symbol symbol, const set<State>& currentStates) {
    set<State> result;
    for (const auto& transition : nfa.m_Transitions) {
        if (currentStates.find(transition.first.first) != currentStates.end() && symbol == transition.first.second) {
            result.insert(transition.second.begin(), transition.second.end());
        }
    }
    return result;
}


MISNFA determinizeNFA(const MISNFA& nfa, MISNFA& tmp) {
    // If tmp is already a DFA, return it
    if (isDFA(tmp)) return tmp;

    // Initialize data structures
    map<pair<State, Symbol>, set<State>> transitions;
    queue<set<State>> stateQueue;
    map<set<State>, State> stateMapping;
    int stateIndex = 0;
    bool initialIsFinal = false;

    // Check if the initial state is also a final state
    for (const auto& finalState : tmp.m_FinalStates) {
        if (finalState == *tmp.m_InitialStates.begin()) {
            initialIsFinal = true;
            break;
        }
    }

    // Start the process with the initial states
    stateQueue.push(tmp.m_InitialStates);
    stateMapping[tmp.m_InitialStates] = stateIndex++;
    
    // Process the queue until it's empty
    while (!stateQueue.empty()) {
        auto currentStateSet = stateQueue.front();
        stateQueue.pop();

        // Iterate over the alphabet to determine transitions
        for (const auto& symbol : tmp.m_Alphabet) {
            set<State> nextStates = getTransitions(tmp, symbol, currentStateSet);
            if (stateMapping.find(nextStates) == stateMapping.end()) {
                stateQueue.push(nextStates);
                stateMapping[nextStates] = stateIndex++;
            }
            transitions[{stateMapping[currentStateSet], symbol}].insert(stateMapping[nextStates]);
        }
    }

    // Update the states, transitions, and final states in tmp
    tmp.m_States.clear();
    for (const auto& statePair : stateMapping) {
        tmp.m_States.insert(statePair.second);
    }
    
    tmp.m_Transitions.clear();
    tmp.m_Transitions = transitions;

    tmp.m_FinalStates.clear();
    for (const auto& finalState : nfa.m_FinalStates) {
        for (const auto& statePair : stateMapping) {
            if (statePair.first.find(finalState) != statePair.first.end()) {
                tmp.m_FinalStates.insert(statePair.second);
            }
        }
    }

    // If the initial state is also a final state, include it in the final states
    if (initialIsFinal) {
        tmp.m_FinalStates.insert(0);
    }

    // Set the initial state
    tmp.m_InitialStates.clear();
    tmp.m_InitialStates.insert(0);

    return tmp;
}

DFA convertToDFA(MISNFA& nfa) {
    DFA dfa;
    dfa.m_Alphabet = nfa.m_Alphabet;
    dfa.m_InitialState = *nfa.m_InitialStates.begin();

    if (nfa.m_FinalStates.empty()) {
        dfa.m_States.insert(0);
        return dfa;
    }

    dfa.m_FinalStates = nfa.m_FinalStates;
    dfa.m_States = nfa.m_States;

    for (const auto& transition : nfa.m_Transitions) {
        dfa.m_Transitions[{transition.first.first, transition.first.second}] = *transition.second.begin();
    }

    return dfa;
}
DFA determinize(const MISNFA& nfa){
    MISNFA tmp;
    tmp.m_Alphabet=nfa.m_Alphabet;
    tmp.m_States=nfa.m_States;
    tmp.m_FinalStates=nfa.m_FinalStates;
    tmp=getStartingPoint(nfa,tmp);
    tmp=removeUnreachableStates(tmp);
    tmp=determinizeNFA(nfa,tmp);
    tmp=removeUseless(tmp);
    DFA result=convertToDFA(tmp);
    return result;
}