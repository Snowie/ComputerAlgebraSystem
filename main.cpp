#include <iostream>
#include <unordered_map>
#include "TreeNode.h"
#include "Operator.h"

using namespace std;

int main() {
    unordered_map<string, Operator> operators;

    operators.emplace("+", Operator(2, "+", false));
    operators.emplace("-", Operator(2, "-", false));
    operators.emplace("/", Operator(3, "/", false));
    operators.emplace("*", Operator(3, "*", false));
    operators.emplace("^", Operator(4, "^", true ));

    Operator * p = operators.find("+")->second.setOperands({"4", "5"});

    string res = p->evaluate()->toString();
    cout << res << endl;

    delete p;

    return 0;
}