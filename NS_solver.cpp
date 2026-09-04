#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
/**
 * @brief Enumeração que representa o estado de uma aresta na rede.
 */
enum class State{
    BASIC, ///< A aresta está na árvore geradora.
    NB_SUPERIOR, ///< A aresta não está na árvore geradora e está no limite superior(fluxo = capacidade).
    NB_INFERIOR ///< A aresta não está na árvore geradora e está no limite inferior (fluxo=0).
};

/**
 * @brief Estrutura que representa um nó na rede.
 */
struct Node{
    long long potential;
    long long demand;
};
/**
 * @brief Estrutura que representa uma aresta na rede.
 */
struct Edge{
    int from;
    int to;
    long long capacity;
    long long flow;
    long long cost;
    State actual_state;
};

class NetworkSimplexSolver {
private:
    int numNodes;
    int numEdges;

    std::vector<Node> nodes;
    std::vector<Edge> edges;
    std::vector<int> parent;///< Índice do nó pai na árvore geradora
    std::vector<int> depth;
    std::vector<int> thread;///< Ordem de travessia (pre-order) para iterar subárvores
    std::vector<int> edgeToParent;///< Índice da aresta que conecta este nó ao seu pai
    long long computeBigM() {
        long long M = 1;
        for (auto& e : edges) M += std::llabs(e.cost);
        M *= (numNodes + 1);
        return M;
    }
    /**
     * @brief Constrói a árvore inicial para o algoritmo de Network Simplex.
     * @addtogroup NetworkSimplexSolver
     */
    void buildInitialTree() {
        long long M = computeBigM();

        nodes[0].demand = 0;
        nodes[0].potential = 0;
        parent[0] = -1;
        depth[0] = 0;

        for (int i = 1; i <= numNodes; ++i) {
            parent[i] = 0;
            depth[i] = 1;
            thread[i - 1] = i;

            long long nodeDemand = nodes[i].demand;

            if (nodeDemand > 0) {
                // Nó de oferta: aresta i -> raiz
                addEdge(i, 0, std::numeric_limits<long long>::max(), M);
                int arcIndex = (int)edges.size() - 1;
                edges[arcIndex].flow = nodeDemand;
                edges[arcIndex].actual_state = State::BASIC;
                edgeToParent[i] = arcIndex;
                nodes[i].potential = M;
            } else {
                // Nó de demanda/transbordo: aresta raiz -> i
                addEdge(0, i, std::numeric_limits<long long>::max(), M);
                int arcIndex = (int)edges.size() - 1;
                edges[arcIndex].flow = -nodeDemand;
                edges[arcIndex].actual_state = State::BASIC;
                edgeToParent[i] = arcIndex;
                nodes[i].potential = -M;
            }
        }

        thread[numNodes] = 0;
    }
    /**
     * @brief Calcula os potenciais dos nós na árvore geradora.
     * @addtogroup NetworkSimplexSolver
     */
    void computePotentials() {
        nodes[0].potential = 0;
        int v = thread[0];
        while (v != 0) {
            int e = edgeToParent[v];
            const Edge& edge = edges[e];
            if (edge.from == parent[v])
                nodes[v].potential = nodes[parent[v]].potential - edge.cost;
            else
                nodes[v].potential = nodes[parent[v]].potential + edge.cost;
            v = thread[v];
        }
    }
    /**
     * @brief Encontra a aresta de entrada com custo reduzido negativo.
     * @addtogroup NetworkSimplexSolver
     * @return O índice da aresta de entrada, ou -1 se não houver aresta de entrada.
     */
    int findEnteringEdge() {
        for (int i = 0; i < (int)edges.size(); ++i) {
            Edge& e = edges[i];
            if (e.actual_state == State::BASIC) continue;

            long long reducedCost = e.cost - nodes[e.from].potential + nodes[e.to].potential;

            if (e.actual_state == State::NB_INFERIOR && reducedCost < 0)
                return i;
            if (e.actual_state == State::NB_SUPERIOR && reducedCost > 0)
                return i;
        }
        return -1;
    }
public:
    NetworkSimplexSolver(int n) : numNodes(n), numEdges(0) {
        //os vetores serão usados a partir do índice 1. os indices vão de 1 até n
        nodes.resize(n + 1, {0, 0});
        parent.resize(n + 1, 0);
        depth.resize(n + 1, 0);
        thread.resize(n + 1, 0);
        edgeToParent.resize(n + 1, -1);
    }
    /**
     * @brief Adiciona uma aresta ao grafo.
     * @addtogroup NetworkSimplexSolver
     * @param u O índice do nó de origem (1 a n).
     * @param v O índice do nó de destino (1 a n).
     * @param cap A capacidade da aresta.
     * @param cost O custo da aresta.
     * @return void
     */
    void addEdge(int u, int v, long long cap, long long cost) {
        edges.push_back({u, v, cap, cost, 0, State::NB_INFERIOR});
        numEdges++;
    }
    /**
     * @brief Define a demanda de um nó específico.
     * @addtogroup NetworkSimplexSolver
     * @param u O índice do nó (1 a n).
     * @param demand A demanda a ser definida para o nó.
     * @return void
     * **/
    void setDemand(int u, long long demand) {
        nodes[u].demand = demand;
    }
    /**
     * @brief Executa o algoritmo de Network Simplex para resolver o problema de fluxo de custo mínimo.
     * @addtogroup NetworkSimplexSolver
     * @return void
     * **/
     void solve() {
        buildInitialTree();
        computePotentials();
        int entering = findEnteringEdge();
        // próximo passo: LCA + ciclo + teste da razão + pivotamento
    }
};

