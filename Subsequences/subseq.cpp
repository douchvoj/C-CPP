#include "subseq.h"
#include <unordered_map>
CDummy::CDummy(char c)
    : m_C ( c )
{}
ostream& operator <<(ostream& os,const CDummy& x ){
    return os << x . m_C;
}

template <typename T_>
CSelfMatch<T_>::CSelfMatch(initializer_list<T_>init)// constructor (initializer list)
    :c_data(init)
{}

template <typename T_>
template<typename Container> // constructor (a container)
CSelfMatch<T_>::CSelfMatch(const Container& container) 
    : c_data(container.begin(), container.end()) 
{}

template <typename T_>
template<typename Iterator> // constructor (2 iterators)
CSelfMatch<T_>::CSelfMatch(Iterator begin, Iterator end) 
    : c_data(begin, end) 
{}

template <typename T_>
size_t CSelfMatch<T_>::sequenceLen(size_t n) const {
    if (n == 0) throw invalid_argument("Zero");
    size_t maxLen = 0;
    for (size_t len = 1; len <= c_data.size(); ++len) {
        for (size_t start = 0; start <= c_data.size() - len; ++start) {
            vector<T_> subsequence(c_data.begin() + start, c_data.begin() + start + len);
            size_t count = 0;
            for (size_t i = 0; i <= c_data.size() - len; ++i) {
                if (equal(subsequence.begin(), subsequence.end(), c_data.begin() + i)) {
                    ++count;
                    if (count >= n) {
                        maxLen = max(maxLen, len);
                        break;
                    }
                }
            }
        }
    }
    return maxLen;
}
template <typename T_>
bool CSelfMatch<T_>::findVector(const vector<vector<T_>>& vec, const vector<T_>& target) const {
    return find(vec.begin(), vec.end(), target) != vec.end();
}

template <typename T_>
vector<vector<T_>> CSelfMatch<T_>::getSequences(size_t n) const {
    size_t maxLen = sequenceLen(n);
    vector<vector<T_>> tmp;
    for (size_t len = 1; len <= c_data.size(); ++len) {
        for (size_t start = 0; start <= c_data.size() - len; ++start) {
            vector<T_> subsequence(c_data.begin() + start, c_data.begin() + start + len);
            size_t count = 0;
            for (size_t i = 0; i <= c_data.size() - len; ++i) {
                if (equal(subsequence.begin(), subsequence.end(), c_data.begin() + i)) {
                    ++count;
                    if (count >= n && len == maxLen) {
                        if (!findVector(tmp, subsequence)) tmp.push_back(subsequence);
                        break;
                    }
                }
            }
        }
    }
    return tmp;
}

template <typename T_>
set<size_t> CSelfMatch<T_>::getSetOfPositions(const vector<vector<T_>>& sequences, const vector<T_>& seq) const {
    set<size_t> final;
    for (size_t i = 0; i <= c_data.size() - seq.size(); ++i) {
        if (equal(seq.begin(), seq.end(), c_data.begin() + i)) final.insert(i);
    }
    return final;
}

template <typename T_>
template<size_t N_>
void CSelfMatch<T_>::generateCombinations(vector<array<size_t, N_>>& final, const vector<size_t>& elements, array<size_t, N_>& combination, size_t index, size_t remaining) const {
    if (remaining == 0) {
        final.push_back(combination);
        return;
    }
    for (size_t i = index; i <= elements.size() - remaining; ++i) {
        combination[N_ - remaining] = elements[i];
        generateCombinations(final, elements, combination, i + 1, remaining - 1);
    }
}

template <typename T_>
template<size_t N_>
vector<array<size_t, N_>> CSelfMatch<T_>::getPermutations(const vector<set<size_t>>& positions) const {
    vector<array<size_t, N_>> final;
    for (const auto& s : positions) {
        vector<size_t> elements(s.begin(), s.end());
        if (elements.size() < N_) continue;
        array<size_t, N_> combination;
        generateCombinations(final, elements, combination, 0, N_);
    }
    return final;
}

template <typename T_>
template<size_t N_>
vector<array<size_t, N_>> CSelfMatch<T_>::findSequences() const {
    vector<vector<T_>> sequences = getSequences(N_);
    vector<set<size_t>> posBySeq;
    for (const auto& i : sequences) posBySeq.push_back(getSetOfPositions(sequences, i));
    vector<array<size_t, N_>> positions = getPermutations<N_>(posBySeq);
    return positions;
}

template <typename T_>
void CSelfMatch<T_>::printIt() const { 
    for (const auto& i : c_data) cout << i; 
    cout << endl; 
}