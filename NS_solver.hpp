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
struct CycleArc {
    int edgeIndex;
    int direction; // +1 = aumenta flow, -1 = diminui flow
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
    /**
     * @brief Calcula o custo grande usado nas arestas artificiais.
     * @return O valor de penalidade utilizado na árvore inicial.
     */
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
    /**
     * @brief Inicializa um solver para uma rede com n nós.
     * @param n Número de nós da rede.
     */
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
        edges.push_back({u, v, cap, 0, cost, State::NB_INFERIOR});
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
     * @brief Encontra o menor ancestral comum de dois nós na árvore geradora.
     * @addtogroup NetworkSimplexSolver
     * @param u Índice do primeiro nó.
     * @param v Índice do segundo nó.
     * @return O índice do menor ancestral comum dos nós.
     */
    int findLCA(int u, int v) {
        while (depth[u] != depth[v]) {
            if (depth[u] > depth[v]) u = parent[u];
            else v = parent[v];
        }
        while (u != v) {
            u = parent[u];
            v = parent[v];
        }
        return u;
    }
    
    /**
     * @brief Constrói o ciclo formado pela aresta de entrada e pela árvore geradora.
     * @addtogroup NetworkSimplexSolver
     * @param enteringIdx Índice da aresta de entrada.
     * @param lca Índice do menor ancestral comum das extremidades da aresta.
     * @return Os arcos do ciclo e suas respectivas direções de variação de fluxo.
     */
    std::vector<CycleArc> buildCycle(int enteringIdx, int lca) {
        std::vector<CycleArc> cycle;
        const Edge& enter = edges[enteringIdx];

        // perna do "to": sobe até o LCA -> mesma direção do ciclo (natural)
        int v = enter.to;
        while (v != lca) {
            int eIdx = edgeToParent[v];
            int dir = (edges[eIdx].from == v) ? +1 : -1; // filho->pai = +1, pai->filho = -1
            cycle.push_back({eIdx, dir});
            v = parent[v];
        }

        // perna do "from": sobe até o LCA -> mas o ciclo percorre essa perna DESCENDO
        // (LCA -> from), então o sinal calculado pela regra acima é invertido
        int u = enter.from;
        std::vector<CycleArc> fromLeg;
        while (u != lca) {
            int eIdx = edgeToParent[u];
            int dir = (edges[eIdx].from == u) ? +1 : -1;
            fromLeg.push_back({eIdx, -dir}); // inverte
            u = parent[u];
        }
        // insere na ordem correta (LCA -> from), então reverte fromLeg antes de anexar
        cycle.insert(cycle.end(), fromLeg.rbegin(), fromLeg.rend());

        if (enter.actual_state == State::NB_SUPERIOR) {
            for (auto& arc : cycle)
                arc.direction = -arc.direction;
        }

        return cycle;
    }
    /**
     * @brief Calcula o fluxo residual de uma aresta em uma direção.
     * @addtogroup NetworkSimplexSolver
     * @param edgeIndex Índice da aresta.
     * @param direction Direção da variação do fluxo: +1 para aumentar e -1 para diminuir.
     * @return O fluxo residual disponível na direção informada.
     */
    long long getResidual(int edgeIndex, int direction) {
        const Edge& e = edges[edgeIndex];
        return (direction == +1) ? (e.capacity - e.flow) : e.flow;
    }

    /**
     * @brief Executa o teste da razão para determinar o passo do pivô.
     * @addtogroup NetworkSimplexSolver
     * @param enteringIdx Índice da aresta de entrada.
     * @param cycle Ciclo formado pela aresta de entrada e pela árvore geradora.
     * @return Um par contendo theta e o índice da aresta que sai, ou a própria aresta de entrada.
     */
    std::pair<long long, int> ratioTest(int enteringIdx, const std::vector<CycleArc>& cycle) {
        // direção da entrante: se está NB_INFERIOR, ela "entra" aumentando (+1);
        // se está NB_SUPERIOR, entra diminuindo (-1)
        int enterDir = (edges[enteringIdx].actual_state == State::NB_INFERIOR) ? +1 : -1;

        long long theta = getResidual(enteringIdx, enterDir);
        int leavingIdx = enteringIdx;

        for (auto& arc : cycle) {
            long long res = getResidual(arc.edgeIndex, arc.direction);
            if (res < theta) {
                theta = res;
                leavingIdx = arc.edgeIndex;
            }
        }
        return {theta, leavingIdx};
    }
    /**
     * @brief Aplica a variação de fluxo determinada pelo pivô ao ciclo.
     * @addtogroup NetworkSimplexSolver
     * @param enteringIdx Índice da aresta de entrada.
     * @param cycle Ciclo formado pela aresta de entrada e pela árvore geradora.
     * @param theta Tamanho do passo do pivô.
     * @return void
     */
    void applyPivot(int enteringIdx, const std::vector<CycleArc>& cycle, long long theta) {
        int enterDir = (edges[enteringIdx].actual_state == State::NB_INFERIOR) ? +1 : -1;
        edges[enteringIdx].flow += enterDir * theta;

        for (auto& arc : cycle)
            edges[arc.edgeIndex].flow += arc.direction * theta;
    }

    /**
     * @brief Verifica se um nó pertence à subárvore de outro nó.
     * @param x Nó que será consultado.
     * @param vNode Possível ancestral de x.
     * @return true se x for descendente de vNode; caso contrário, false.
     */
    bool isDescendant(int x, int vNode) {
        while (depth[x] > depth[vNode]) x = parent[x];
        return x == vNode;
    }
    /**
     * @brief Executa um pivô do algoritmo de Network Simplex.
     * @addtogroup NetworkSimplexSolver
     * @param enteringIdx Índice da aresta de entrada.
     * @return void
     */
    void pivot(int enteringIdx) {
        int lca = findLCA(edges[enteringIdx].from, edges[enteringIdx].to);
        auto cycle = buildCycle(enteringIdx, lca);
        auto [theta, leavingIdx] = ratioTest(enteringIdx, cycle);

        applyPivot(enteringIdx, cycle, theta);

        if (leavingIdx == enteringIdx) {
            edges[enteringIdx].actual_state =
                (edges[enteringIdx].actual_state == State::NB_INFERIOR) ? State::NB_SUPERIOR : State::NB_INFERIOR;
            return;
        }

        edges[enteringIdx].actual_state = State::BASIC;
        edges[leavingIdx].actual_state =
            (edges[leavingIdx].flow == 0) ? State::NB_INFERIOR : State::NB_SUPERIOR;

        // v = endpoint mais profundo da aresta que saiu -> raiz da subárvore que se desconecta
        int a = edges[leavingIdx].from, b = edges[leavingIdx].to;
        int v = (edgeToParent[a] == leavingIdx) ? a : b;

        // qual endpoint da entrante está dentro da subárvore(v)?
        int childSide, attachSide;
        if (isDescendant(edges[enteringIdx].from, v)) {
            childSide = edges[enteringIdx].from;
            attachSide = edges[enteringIdx].to;
        } else {
            childSide = edges[enteringIdx].to;
            attachSide = edges[enteringIdx].from;
        }

        // captura a cadeia childSide -> ... -> v ANTES de mexer em parent/edgeToParent
        std::vector<int> path;
        std::vector<int> origEdge;
        for (int x = childSide; x != v; x = parent[x]) {
            path.push_back(x);
            origEdge.push_back(edgeToParent[x]);
        }
        path.push_back(v);

        // inverte a cadeia de parentesco
        for (int i = 0; i + 1 < (int)path.size(); ++i) {
            parent[path[i + 1]] = path[i];
            edgeToParent[path[i + 1]] = origEdge[i];
        }
        parent[childSide] = attachSide;
        edgeToParent[childSide] = enteringIdx;

        rebuildDepthAndThread();
    }
    /**
     * @brief Reconstrói profundidades e ordem de travessia da árvore.
     */
    void rebuildDepthAndThread() {
        std::vector<std::vector<int>> children(numNodes + 1);
        for (int x = 0; x <= numNodes; ++x)
            if (parent[x] != -1) children[parent[x]].push_back(x);

        depth[0] = 0;
        std::vector<int> preorder;
        std::vector<int> stack = {0};
        while (!stack.empty()) {
            int u = stack.back(); stack.pop_back();
            preorder.push_back(u);
            for (int c : children[u]) {
                depth[c] = depth[u] + 1;
                stack.push_back(c);
            }
        }

        for (size_t i = 0; i + 1 < preorder.size(); ++i)
            thread[preorder[i]] = preorder[i + 1];
        thread[preorder.back()] = 0;
    }
    /**
     * @brief Executa o algoritmo de Network Simplex para resolver o problema de fluxo de custo mínimo.
     * @addtogroup NetworkSimplexSolver
     * @return custo total minimo
     * **/
    long long solve() {
        buildInitialTree();
        computePotentials();

        int entering = findEnteringEdge();
        while (entering != -1) {
            pivot(entering);
            computePotentials(); 
            entering = findEnteringEdge();
        }

        for (auto& e : edges) {
            if ((e.from == 0 || e.to == 0) && e.flow > 0)
                return -1; 
        }

        long long totalCost = 0;
        for (auto& e : edges) {
            if (e.from != 0 && e.to != 0)
                totalCost += e.flow * e.cost;
        }
        return totalCost;
    }
    /**
     * @brief Retorna o fluxo atual de uma aresta.
     * @param edgeIndex Índice da aresta consultada.
     * @return Fluxo atual da aresta.
     */
    long long getFlow(int edgeIndex) const {
        return edges[edgeIndex].flow;
    }
};

