//
// Created by Doran on 4/15/2015.
//

#include "Function.h"

Function::Function(string signature) {
    this->signature = signature;
    children = {};
}

Expression *Function::evaluate() {
    string s = toString();
    s += "(";

    for (int i = 0; i < children.size(); ++i)
        if (i != children.size() - 1)
            s += children[i]->evaluate()->toString() + ", ";
        else
            s += children[i]->evaluate()->toString();

    s += ")";
    return new Expression(s);
}

string Function::toString() const {
    return signature;
}

Function *Function::setArguments(vector<TreeNode *> arguments) const {
    return new Function(this, arguments);
}

Function::Function(const Function *const other, vector<TreeNode *> arguments) {
    children = arguments;
    this->signature = other->toString();
}
