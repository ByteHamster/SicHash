#ifndef TESTCOMPARISON_GAUSSIANELIMINATION_H
#define TESTCOMPARISON_GAUSSIANELIMINATION_H

#include <cstdarg>
#include <algorithm>
#include <cstring>
#include <vector>
#include <cassert>
#include <iostream>

#define VARIABLE_UNDEFINED INT32_MAX
#define CONTAINS(V, X) (std::find(V.begin(), V.end(), X) != V.end())
#define REMOVE(V, X) V.erase(std::remove(V.begin(), V.end(), X), V.end())
#define SWAP(V, X, Y) iter_swap(V.begin() + X, V.begin() + Y)

// Can solve equations using gaussian elimination.
// Assumes that the number of variables per equation is
// a lot smaller than the number of total variables.
class GaussianElimination {
    private:
        std::vector<std::vector<int>> equations; // Invariant: inner elements are sorted
        std::vector<char> results;
        char *variableAssignments;
        int NumVariables;
        int VariablesPerEquation;
    public:
        GaussianElimination(int NumVariables, int VariablesPerEquation);
        ~GaussianElimination();
        void addEquation(const int *variables, char result);
        bool solve();
        void printAssignment();
        char getAssignment(int variable);
        void print();
        void print(std::vector<std::vector<int>> equations, std::vector<char> results);
};

#endif //TESTCOMPARISON_GAUSSIANELIMINATION_H
