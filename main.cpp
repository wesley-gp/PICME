#include "NS_solver.hpp"
#include <cassert>


int main() {
    // Duas rotas entre a oferta 1 e a demanda 4. A rota 1->2->4 e barata,
    // mas sua capacidade limita o fluxo; o restante usa 1->3->4.
    NetworkSimplexSolver solver(4);
    solver.addEdge(1, 2, 3, 1);
    solver.addEdge(2, 4, 3, 1);
    solver.addEdge(1, 3, 7, 4);
    solver.addEdge(3, 4, 7, 4);
    solver.addEdge(2, 3, 3, 0);
    solver.setDemand(1, 7);
    solver.setDemand(4, -7);

    long long cost = solver.solve();

    std::cout << "Custo total: " << cost << " (esperado: 38)\n";
    std::cout << "Fluxo 1->2: " << solver.getFlow(0) << " (esperado: 3)\n";
    std::cout << "Fluxo 2->4: " << solver.getFlow(1) << " (esperado: 3)\n";
    std::cout << "Fluxo 1->3: " << solver.getFlow(2) << " (esperado: 4)\n";
    std::cout << "Fluxo 3->4: " << solver.getFlow(3) << " (esperado: 4)\n";
    std::cout << "Fluxo 2->3: " << solver.getFlow(4) << " (esperado: 0)\n";

    assert(cost == 38);
    assert(solver.getFlow(0) == 3);
    assert(solver.getFlow(1) == 3);
    assert(solver.getFlow(2) == 4);
    assert(solver.getFlow(3) == 4);
    assert(solver.getFlow(4) == 0);
    return 0;
}