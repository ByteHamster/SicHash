#include "GaussianElimination.h"

GaussianElimination::GaussianElimination(int NumVariables, int VariablesPerEquation)
        : NumVariables(NumVariables), VariablesPerEquation(VariablesPerEquation) {
    variableAssignments = new char[NumVariables];
    memset(variableAssignments, 0, NumVariables * sizeof(char));
}

GaussianElimination::~GaussianElimination() {
    delete [] variableAssignments;
}

void GaussianElimination::addEquation(const int *variables, char result) {
    std::vector<int> newRow;
    for (int i = 0; i < VariablesPerEquation; i++) {
        int newVariable = variables[i];
        if (!CONTAINS(newRow, newVariable)) {
            // Add variable only once
            newRow.push_back(newVariable);
        }
    }
    std::sort(newRow.begin(), newRow.end());
    equations.push_back(newRow);
    results.push_back(result);
}

bool GaussianElimination::solve() {
    assert(NumVariables < VARIABLE_UNDEFINED);

    for (int variable = 0; variable < NumVariables; variable++) {
        // Find equation that contains this variable in the first position
        int equationWithVariableFirst = 0;
        while (equationWithVariableFirst < equations.size()) {
            if (!equations.at(equationWithVariableFirst).empty()
                    && equations.at(equationWithVariableFirst).at(0) == variable) {
                break;
            }
            equationWithVariableFirst++;
        }
        if (equationWithVariableFirst >= equations.size()) {
            // Variable never appears in the equations. Add an arbitrary equation fixing that variable
            std::vector<int> newRow;
            newRow.push_back(variable);
            equations.push_back(newRow);
            results.push_back(0);
            equationWithVariableFirst = equations.size() - 1;
        }

        // Swap equation to the upper triangle position
        SWAP(equations, equationWithVariableFirst, variable);
        SWAP(results, equationWithVariableFirst, variable);

        // Eliminate variable from all rows below
        for (int otherRow = variable + 1; otherRow < equations.size(); otherRow++) {
            if (CONTAINS(equations.at(otherRow), variable)) {
                // Eliminate
                for (int x : equations.at(variable)) {
                    if (CONTAINS(equations.at(otherRow), x)) {
                        REMOVE(equations.at(otherRow), x);
                    } else {
                        equations.at(otherRow).push_back(x);
                    }
                }
                results.at(otherRow) = results.at(otherRow) ^ results.at(variable);
                std::sort(equations.at(otherRow).begin(), equations.at(otherRow).end());
            }
        }
    }

    // Back substitution
    for (int variable = NumVariables - 1; variable >= 0; variable--) {
        bool result = results.at(variable);

        // Assumes that other variables than the current one are already removed (triangle form)
        for (int otherVariable : equations.at(variable)) {
            if (otherVariable == variable) {
                continue;
            }
            result = result ^ variableAssignments[otherVariable];
        }
        variableAssignments[variable] = result;
    }

    // Check solvability in case of over-specification
    for (int i = NumVariables; i < equations.size(); i++) {
        if (results.at(i) != 0 || !equations.at(i).empty()) {
            return false;
        }
    }
    return true;
}

void GaussianElimination::printAssignment() {
    for (int variable = 0; variable < NumVariables; variable++) {
        std::cout<<"Assignment for variable "<<variable<<": "<<std::to_string(variableAssignments[variable])<<std::endl;
    }
}

char GaussianElimination::getAssignment(int variable) {
    return variableAssignments[variable];
}

void GaussianElimination::print() {
    print(equations, results);
}

void GaussianElimination::print(std::vector<std::vector<int>> equations, std::vector<char> results) {
    for (int i = 0; i < equations.size(); i++) {
        for (int j = 0; j < NumVariables; j++) {
            std::cout<<CONTAINS(equations.at(i), j);
        }
        std::cout<<" "<<std::to_string(results.at(i))<<" --- ";
        for (int variable : equations.at(i)) {
            std::cout<<variable<<" ";
        }
        std::cout<<std::endl;
    }
}
