#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

enum class State{
    BASIC,
    NB_SUPERIOR,
    NB_INFERIOR
};

struct Node{
    long long potential;
    long long demand;
};
struct Edge{
    int from;
    int to;
    long long capacity;
    long long flow;
    long long cost;
    State actual_state;
};

class NetworkSimplexSolver {

};

