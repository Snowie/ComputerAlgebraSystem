#include <iostream>
#include "cas.h"

/**
 * @file main.cpp
 * The starting point of execution for CAS
 */
using namespace std;
using namespace cas;

int main() {
    unordered_map<string, Operator> operators;
    unordered_map<string, Function> functions;

    operators.emplace("+", Operator(2, "+", false));
    operators.emplace("-", Operator(2, "-", false));
    operators.emplace("/", Operator(3, "/", false));
    operators.emplace("*", Operator(3, "*", false));
    operators.emplace("^", Operator(4, "^", true));

    functions.emplace("sin", Function("sin"));
    functions.emplace("cos", Function("cos"));
    functions.emplace("tan", Function("tan"));
    functions.emplace("sec", Function("sec"));
    functions.emplace("log", Function("log"));
    functions.emplace("harmonic", Function("harmonic"));

    while (true) {
        cout << "\n\n\nEnter the function you would like to parse, or type quit to quit: ";
        string expression = "0 * 1 * 0";
        getline(cin, expression);
        if (expression == "quit") {
            cout << "Caught exit command, exiting..." << endl;
            break;
        }
        TreeNode *f = ShuntingYard(operators, functions, expression);
        f = simplify(operators, functions, f);

        TreeNode *fPrime = derive(operators, functions, "x", f);
        fPrime = simplify(operators, functions, fPrime);

        cout << "f(x) = " << f->evaluate()->toString() << endl;
        cout << "f'(x) = " << ((fPrime == nullptr) ? "nullptr" : fPrime->evaluate()->toString()) << endl;

        delete f;
        delete fPrime;
    }

    return 0;
}