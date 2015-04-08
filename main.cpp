#include <iostream>
#include <unordered_map>
#include <stack>

#include "TreeNode.h"
#include "Operator.h"

using namespace std;

stack<TreeNode *> addNode(stack<TreeNode *> operand_stack, Operator o) {
    //cout << "In add node" << endl;
    if(o.toString() == "(" || o.toString() == ")")
        cout << "Problem" << endl;
    TreeNode * r = operand_stack.top();
    operand_stack.pop();
    TreeNode * l = operand_stack.top();
    operand_stack.pop();
    operand_stack.push(o.setOperands({l, r}));
    return operand_stack;
}

TreeNode *ShuntingYard(unordered_map<string, Operator> operators, string toParse) {
    stack<Operator> operator_stack;
    stack<TreeNode *> operand_stack;

    for (char c: toParse) {
        string s = "";
        s += c;

        if (s == " ")
            continue;

        if(s == "(") {
            operator_stack.push(Operator(9, "(", false));
            continue;
        }

        if(s == ")"){
            while(!operator_stack.empty()) {
                Operator popped = operator_stack.top();
                operator_stack.pop();
                if(popped.toString() == "(") {
                    break;
                }
                else {
                    operand_stack = addNode(operand_stack, popped);
                }
            }
            continue;
        }
        if (operators.find(s) != operators.end()) {
            Operator * o1 = &operators.find(s)->second;
            Operator * o2 = nullptr;
            while (!operator_stack.empty() && (o2 = &operators.find(operator_stack.top().toString())->second)){
                if(operators.find(operator_stack.top().toString()) == operators.end())
                    break;

                if((!o1->isRightAssociative() && 0 == o1->comparePrecedence(o2)) || o1->comparePrecedence(o2) < 0) {
                    operator_stack.pop();
                    operand_stack = addNode(operand_stack, *o2);
                } else {
                    break;
                }
            }
            operator_stack.push(operators.find(s)->second);
        }
        else {
            operand_stack.emplace(new Expression(s));
        }
    }
    while(!operator_stack.empty()) {
        //cout << "Working op: " << operator_stack.top().toString() << endl;
        operand_stack = addNode(operand_stack, operator_stack.top());
        operator_stack.pop();
    }

    if(!operand_stack.empty())
        return operand_stack.top();
    else
        return new Expression("0");
}

TreeNode * derive(unordered_map<string, Operator> operators, string wrt, TreeNode * root) {
    //It's an operator or function
    if(!root->children.empty()) {
        Operator * opRoot = dynamic_cast<Operator*>(root);
        switch(opRoot->toString()[0]){
            case '+':
            {
                //D/dx(a + b) = a' + b'
                vector<TreeNode*> newChildren;

                for(auto c: opRoot->children)
                    newChildren.push_back(derive(operators, wrt, c));

                return operators.find("+")->second.setOperands(newChildren);
            }
            case '-':
            {
                //D/dx(a - b) = a' - b'
                vector<TreeNode*> newChildren;

                for(auto c: opRoot->children)
                    newChildren.push_back(derive(operators, wrt, c));

                return operators.find("-")->second.setOperands(newChildren);
            }
            case '*':
            {
                //D/dx(ab) = a'b + ab'
                vector<TreeNode*> toPassAdd;

                for(int j = 0; j < opRoot->children.size(); ++j){
                    //Safely get a copy of this tree
                    vector<TreeNode *> copyOfChildren = ShuntingYard(operators, opRoot->evaluate()->toString())->children;// = opRoot->children;
                    vector<TreeNode *> toPassMult;

                    //Go through variable by variable in each of the terms and select which one to derive
                    for(int k = 0; k < opRoot->children.size(); ++k){
                        if(k == j)
                            toPassMult.push_back(derive(operators, wrt, copyOfChildren[k]));
                        else
                            toPassMult.push_back(copyOfChildren[k]);
                    }
                    toPassAdd.push_back(operators.find("*")->second.setOperands(toPassMult));
                }
                Operator * addOp = operators.find("+")->second.setOperands(toPassAdd);
                return addOp;
            }
            case '/':
            {
                //D/dx(f(x)/g(x)) = (f'(x)g(x) - f(x)g'(x))/(g(x))^2
                Operator * multOp1 = operators.find("*")->second.setOperands( {derive(operators, wrt, opRoot->children[0]), opRoot->children[1]} );
                Operator * multOp2 = operators.find("*")->second.setOperands( {opRoot->children[0], derive(operators, wrt, opRoot->children[1])} );
                Operator * subOp = operators.find("-")->second.setOperands( {multOp1, multOp2} );
                Operator * powOp = operators.find("^")->second.setOperands( {opRoot->children[1], new Expression("2")} );
                Operator * divOp = operators.find("/")->second.setOperands( {subOp, powOp} );
                return divOp;
            }
            case '^':
            {
                //D/dx(b^c) = c * b^(c - 1) * D/dx(b)
                TreeNode * base = opRoot->children[0];
                TreeNode * derivedBase = derive(operators, wrt, base);

                TreeNode * oldExponent = opRoot->children[1];
                Operator * newExponent = operators.find("-")->second.setOperands( {ShuntingYard(operators, oldExponent->evaluate()->toString()), new Expression("1")} );

                TreeNode * newCoefficient = ShuntingYard(operators, oldExponent->evaluate()->toString());
                Operator * newPowerOp = operators.find("^")->second.setOperands( {ShuntingYard(operators, base->evaluate()->toString()), newExponent} );

                Operator * multOp = operators.find("*")->second.setOperands( {newCoefficient, newPowerOp, derivedBase} );
                return multOp;
            }
        }
    }
    //It's an expression
    else {
        Expression * exRoot = dynamic_cast<Expression*>(root);
        if(isdigit(exRoot->toString()[0]))
            return new Expression("0");

        if(exRoot->toString() == wrt)
            return new Expression("1");

        return new Expression("0");
    }
    //Fail safely
    return new Expression("0");
}

int main() {
    unordered_map<string, Operator> operators;

    operators.emplace("+", Operator(2, "+", false));
    operators.emplace("-", Operator(2, "-", false));
    operators.emplace("/", Operator(3, "/", false));
    operators.emplace("*", Operator(3, "*", false));
    operators.emplace("^", Operator(4, "^", true));

    TreeNode * rootOfEquation = ShuntingYard(operators, "2/x^3 + 1/x^9 + (2 * (x^2))^(4)");

    cout << "f(x) = " << rootOfEquation->evaluate()->toString() << endl;

    TreeNode * dRootOfEquation = derive(operators, "x", rootOfEquation);
    cout << "f'(x) = " << ((dRootOfEquation == nullptr) ? "nullptr" : dRootOfEquation->evaluate()->toString()) << endl;

    return 0;
}