#include <iostream>
#include <vector>
#include <set>
#include <array>
#include <algorithm>
#include <cassert>
using namespace std;

class CDummy
{
  public:
    CDummy(char c);
    bool operator == (const CDummy& other ) const = default;
    friend ostream& operator <<(ostream& os,const CDummy& x );
  private:
    char m_C;
};

template <typename T_>
class CSelfMatch
{
  public:
    CSelfMatch(initializer_list<T_>init);// constructor (initializer list)

    template<typename Container>// constructor (a container)
    CSelfMatch(const Container& container);

    template<typename Iterator>// constructor (2 iterators)
    CSelfMatch(Iterator begin, Iterator end);

    size_t sequenceLen(size_t n) const ;
    bool findVector(const vector<vector<T_>>& vec, const vector<T_>& target)const ;
    vector<vector<T_>> getSequences(size_t n)const ;
    set<size_t>getSetOfPositions(const vector<vector<T_>>& sequences,const vector<T_>& seq)const;

    template<size_t N_>
    void generateCombinations(vector<array<size_t, N_>>& final, const vector<size_t>& elements, array<size_t, N_>& combination, size_t index, size_t remaining) const;

    template<size_t N_>
    vector<array<size_t, N_>> getPermutations(const vector<set<size_t>>& positions) const;

    template<size_t N_> 
    vector<array<size_t, N_>> findSequences () const;
    void printIt()const;
  private:
    vector<T_> c_data;
};