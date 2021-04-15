#ifndef TESTCOMPARISON_GRAPHPEELER_H
#define TESTCOMPARISON_GRAPHPEELER_H

#include <vector>

class GraphPeeler {
    private:
        std::vector<std::vector<uint64_t>> edges;
        std::vector<uint64_t> edgeAssignments;
        std::vector<std::vector<uint64_t>> nodes;
        int numPeeled = 0;
    public:
        GraphPeeler(size_t maxNodes) {
            nodes.resize(maxNodes);
            edgeAssignments.resize(maxNodes);
        }

        void addEdge(std::vector<uint64_t> &edge) {
            size_t edgeId = edges.size();
            edges.push_back(edge);
            for (uint64_t node : edge) {
                nodes.at(node).push_back(edgeId);
            }
        }

        void solve() {
            std::vector<size_t> todo;
            for (size_t i = 0; i < nodes.size(); i++) {
                std::vector<uint64_t> &incoming = nodes.at(i);
                if (incoming.size() == 1) {
                    todo.push_back(i);
                }
                if (i % 10000000 == 0) {
                    std::cout<<"Preparing "<<i*100/nodes.size()<<"%"<<std::endl;
                }
            }
            while (!todo.empty()) {
                size_t location = todo.at(0);
                todo.erase(todo.begin());
                std::vector<uint64_t> &incoming = nodes.at(location);
                if (incoming.size() == 0) {
                    continue; // Was cleared in the meantime
                }
                assert(incoming.size() == 1);
                size_t edge = incoming.at(0);
                edgeAssignments.at(edge) = location;
                numPeeled++;
                //std::cout<<"Assigned "<<edge<<" to "<<location<<std::endl;
                if (numPeeled % 1000 == 0) {
                    std::cout<<"Peeling "<<numPeeled*100/edges.size()<<"%"<<std::endl;
                }
                for (uint64_t otherLocation : edges.at(edge)) {
                    nodes.at(otherLocation).erase(std::remove(nodes.at(otherLocation).begin(),
                           nodes.at(otherLocation).end(), edge), nodes.at(otherLocation).end());
                    if (nodes.at(otherLocation).size() == 1) {
                        todo.push_back(otherLocation);
                    }
                }
            }
            assert(numPeeled == edges.size() && "Not peelable");
        }

        int getAssignment(int edge) {
            return edgeAssignments.at(edge);
        }
};


#endif //TESTCOMPARISON_GRAPHPEELER_H
