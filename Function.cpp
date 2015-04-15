//
// Created by Doran on 4/15/2015.
//

#include "Function.h"

Function::Function(string signature) {
    this->signature = signature;
}

Expression *Function::evaluate() {
    string s;
    for (int i = 0; i < children.size(); ++i) {
        if (i != children.size() - 1) {
            ;
        }
        else {

        }
    }
    return nullptr;
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
