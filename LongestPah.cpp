#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

enum Point : size_t {};

struct Path {
  Point from, to;
  unsigned length;

  Path(size_t f, size_t t, unsigned l) : from{f}, to{t}, length{l} {}

  friend bool operator == (const Path& a, const Path& b) {
    return tie(a.from, a.to, a.length) == tie(b.from, b.to, b.length);
  }
  
  friend bool operator != (const Path& a, const Path& b) { return !(a == b); }
};
vector<size_t> topologicalSort(vector<vector<size_t>> &adj, vector<size_t> &indegree) {
    size_t n = adj.size();
    queue <size_t> que;
    vector<size_t> result;
    for (size_t i = 0; i < n; i++) if (indegree[i] == 0) que.push(i);
    while (!que.empty()) {
        size_t u = que.front();
        result.push_back(u);
        que.pop();
        for (size_t v : adj[u]) if (--indegree[v] == 0) que.push(v);
    }
    return result;
}
vector<vector<pair<size_t,unsigned>>>get_outcoming(const size_t points, const vector<Path> & all_paths){
    vector<vector<pair<size_t, unsigned>>> outcoming(points,vector<pair<size_t, unsigned>>());
    for (auto &i : all_paths) {
        outcoming[i.from].push_back(make_pair(i.to, i.length));
    }
    return outcoming;
}   
vector<vector<pair<size_t,unsigned>>>get_incoming(const size_t points, const vector<Path> & all_paths){
    vector<vector<pair<size_t, unsigned>>>incoming(points,vector<pair<size_t, unsigned>>());
    for (auto &i : all_paths) {
        incoming[i.to].push_back(make_pair(i.from, i.length));
    }
    return incoming;
}  
vector<size_t>get_lengths(const vector<size_t>sorted,const vector<vector<pair<size_t,unsigned>>>incoming,vector<vector<pair<size_t,unsigned>>>outcoming){
    vector<size_t>lengths(sorted.size(),0);
    for(auto & i: sorted) {
        for(auto & x : outcoming[i]){
            if(x.second+lengths[i]>lengths[i]) {
                lengths[x.first]=lengths[i]+x.second;
            }
        }
    }
    return lengths;

}
vector<pair<Path,unsigned>>get_ways(const vector<size_t>&sorted,vector<vector<pair<size_t,unsigned>>>&outcoming){
    vector<pair<Path,unsigned>>result(sorted.size()+1,make_pair(Path(0,0,0),0));
    unsigned max=0;
    for(auto & i: sorted) {
        for(auto & x : outcoming[i]){
            if(result[i].second +x.second> result[x.first].second) {
                result[x.first].second=result[i].second+x.second;
                result[x.first].first=Path(i,x.first,x.second);
                
                if(result[x.first].second>max)max=result[x.first].second;

            }
        }
    }
    result[result.size()-1].second=max;
    return result;

}
vector<Path>get_final(const vector<pair<Path,unsigned>>& result,const vector<size_t> &sorted,const vector<vector<pair<size_t,unsigned>>>& incoming,vector<vector<pair<size_t,unsigned>>>& outcoming){
    vector<Path>final;
    unsigned max=result.back().second;
    auto it = find_if(result.begin(), result.end(), [max](const pair<Path,unsigned>& element) { return element.second == max; });
    size_t i=it->first.from;
    Path x(it->first.from,it->first.to,it->first.length);
    final.insert(final.begin(),x);

    while(true){
        Path x(result[i].first.from,result[i].first.to,result[i].first.length);
        if(x.from==0 && x.to==0 && x.length==0)break;
        final.insert(final.begin(),x);
        i=result[i].first.from;
    }
    
    return final;

}
vector<Path> longest_track(size_t points, const vector<Path>& all_paths){
    vector<vector<size_t>> adj(points);
    vector<size_t> indegree(points, 0);
    for(auto & i:all_paths){
        adj[i.from].push_back(i.to);
        indegree[i.to]++;
    }
    vector<size_t>sorted=topologicalSort(adj, indegree);
    vector<vector<pair<size_t,unsigned>>>outcoming=get_outcoming(points,all_paths);
    vector<vector<pair<size_t,unsigned>>>incoming=get_incoming(points,all_paths);
    vector<pair<Path,unsigned>>result=get_ways(sorted,outcoming);
    vector<Path>x=get_final(result,sorted,incoming,outcoming);

    return x;
}


#ifndef __PROGTEST__


struct Test {
  unsigned longest_track;
  size_t points;
  vector<Path> all_paths;
};

inline const Test TESTS[] = {
    {13, 5, { {3,2,10}, {3,0,9}, {0,2,3}, {2,4,1} } },
    {11, 5, { {3,2,10}, {3,1,4}, {1,2,3}, {2,4,1} } },
    {16, 8, { {3,2,10}, {3,1,1}, {1,2,3}, {1,4,15} } },

};

#define CHECK(cond, ...) do { \
    if (cond) break; \
    printf("Fail: " __VA_ARGS__); \
    printf("\n"); \
    return false; \
  } while (0)

bool run_test(const Test& t) {
  auto sol = longest_track(t.points, t.all_paths);

  unsigned length = 0;
  for (auto [ _, __, l ] : sol) length += l;

  CHECK(t.longest_track == length,
    "Wrong length: got %u but expected %u", length, t.longest_track);

  for (size_t i = 0; i < sol.size(); i++) {
    CHECK(count(t.all_paths.begin(), t.all_paths.end(), sol[i]),
      "Solution contains non-existent path: %zu -> %zu (%u)",
      sol[i].from, sol[i].to, sol[i].length);

    if (i > 0) CHECK(sol[i].from == sol[i-1].to,
      "Paths are not consecutive: %zu != %zu", sol[i-1].to, sol[i].from);
  }

  return true;
}
#undef CHECK

int main() {
  int ok = 0, fail = 0;

  for (auto&& t : TESTS) (run_test(t) ? ok : fail)++;
  
  if (!fail) printf("Passed all %i tests!\n", ok);
  else printf("Failed %u of %u tests.\n", fail, fail + ok);
}

#endif


