#include <iostream>
#include <unordered_map>
#include <stack>
#include <regex>
#include "TreeNode.h"
#include "Operator.h"
#include "Function.h"

using namespace std;
TreeNode * derive(unordered_map<string, Operator>, unordered_map<string, Function>, string, TreeNode *);

stack<TreeNode *> addNode(stack<TreeNode *> operand_stack, Operator o) {
    if (o.toString() == "(" || o.toString() == ")")
        cout << "Problem" << endl;
    TreeNode *r = operand_stack.top();
    operand_stack.pop();
    TreeNode *l = operand_stack.top();
    operand_stack.pop();
    operand_stack.push(o.setOperands({l, r}));
    return operand_stack;
}

//TODO: Fix this!
//      Does C++ regex support lookaround?
vector<string> tokenize(string toTokenize) {
    //std::regex r4("[0-9]", std::regex_constants::basic);
    /*(?<=op)|(?=op)
    [-+*^/()]*/
    regex pattern("(?=[-+*^/()])");
    sregex_token_iterator first{toTokenize.begin(), toTokenize.end(), pattern}, last;
    return {first, last};
}

TreeNode *ShuntingYard(unordered_map<string, Operator> operators, string toParse) {
    stack<Operator> operator_stack;
    stack<TreeNode *> operand_stack;

    for (char c: toParse) {
        string s = "";
        s += c;

        if (s == " ")
            continue;

        if (s == "(") {
            operator_stack.push(Operator(9, "(", false));
            continue;
        }

        if (s == ")") {
            while (!operator_stack.empty()) {
                Operator popped = operator_stack.top();
                operator_stack.pop();
                if (popped.toString() == "(") {
                    break;
                }
                else {
                    operand_stack = addNode(operand_stack, popped);
                }
            }
            continue;
        }
        if (operators.find(s) != operators.end()) {
            Operator *o1 = &operators.find(s)->second;
            Operator *o2 = nullptr;
            while (!operator_stack.empty() && (o2 = &operators.find(operator_stack.top().toString())->second)) {
                if (operators.find(operator_stack.top().toString()) == operators.end())
                    break;

                if ((!o1->isRightAssociative() && 0 == o1->comparePrecedence(o2)) || o1->comparePrecedence(o2) < 0) {
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
    while (!operator_stack.empty()) {
        operand_stack = addNode(operand_stack, operator_stack.top());
        operator_stack.pop();
    }

    if (!operand_stack.empty())
        return operand_stack.top();
    else
        return new Expression("0");
}

TreeNode * deriveFunction(unordered_map<string, Operator> operators, unordered_map<string, Function> functions, string wrt, TreeNode * root) {

    //D/dx(sin(x)) = cos(x) * x'
    if(root->toString() == "sin") {
        return operators.find("*")->second.setOperands(
                {functions.find("cos")->second.setArguments(root->children), derive(operators, functions, wrt, root->children[0])});
    }
    //D/dx(log(x)) = 1/x * x'
    if(root->toString() == "log") {
        Operator * divisionOperator = operators.find("/")->second.setOperands({new Expression("1"), root->children[0]});
        Operator * multiplyOperator = operators.find("*")->second.setOperands({divisionOperator, derive(operators, functions, wrt, root->children[0])});
        return multiplyOperator;
    }
    //D/dx(cos(x)) = -sin(x) * x'
    if(root->toString() == "cos") {
        return operators.find("*")->second.setOperands(
                {functions.find("sin")->second.setArguments(root->children), derive(operators, functions, wrt, root->children[0]), new Expression("-1")});
    }
    //D/dx(tan(x)) = sec(x)^2 * x'
    if(root->toString() == "tan") {
        Operator * powerOperator = operators.find("^")->second.setOperands({functions.find("sec")->second.setArguments({root->children[0]}), new Expression("2")});
        Operator * multiplyOperator = operators.find("*")->second.setOperands({powerOperator, derive(operators, functions, wrt, root->children[0])});

        return multiplyOperator;
    }

    Function failSafe("D");
    return failSafe.setArguments({root});
}

//Returns a TreeNode that represents the derivative of the supplied function wrt 'wrt'
TreeNode *derive(unordered_map<string, Operator> operators, unordered_map<string, Function> functions, string wrt, TreeNode *root) {
    Function *functionRoot = dynamic_cast<Function *>(root);
    if(functionRoot != nullptr)
        return deriveFunction(operators, functions, wrt, functionRoot);

    //It's an operator
    if (!root->children.empty()) {
        Operator *opRoot = dynamic_cast<Operator *>(root);
        switch (opRoot->toString()[0]) {
            case '+': {
                //D/dx(a + b) = a' + b'
                vector<TreeNode *> newChildren;

                for (auto c: opRoot->children)
                    newChildren.push_back(derive(operators, functions, wrt, c));

                return operators.find("+")->second.setOperands(newChildren);
            }
            case '-': {
                //D/dx(a - b) = a' - b'
                vector<TreeNode *> newChildren;

                for (auto c: opRoot->children)
                    newChildren.push_back(derive(operators, functions, wrt, c));

                return operators.find("-")->second.setOperands(newChildren);
            }
            case '*': {
                //D/dx(ab) = a'b + ab'
                vector<TreeNode *> toPassAdd;

                for (int j = 0; j < opRoot->children.size(); ++j) {
                    //Safely get a copy of this tree
                    vector<TreeNode *> copyOfChildren = ShuntingYard(operators,
                                                                     opRoot->evaluate()->toString())->children;
                    vector<TreeNode *> toPassMultiply;

                    //Go through variable by variable in each of the terms and select which one to derive
                    for (int k = 0; k < opRoot->children.size(); ++k) {
                        if (k == j)
                            toPassMultiply.push_back(derive(operators, functions, wrt, copyOfChildren[k]));
                        else
                            toPassMultiply.push_back(copyOfChildren[k]);
                    }
                    toPassAdd.push_back(operators.find("*")->second.setOperands(toPassMultiply));
                }
                Operator *addOp = operators.find("+")->second.setOperands(toPassAdd);
                return addOp;
            }
            case '/': {
                //D/dx(f(x)/g(x)) = (f'(x)g(x) - f(x)g'(x))/(g(x))^2
                Operator *multiplyOp1 = operators.find("*")->second.setOperands(
                        {derive(operators, functions, wrt, opRoot->children[0]), opRoot->children[1]});

                Operator *multiplyOp2 = operators.find("*")->second.setOperands(
                        {opRoot->children[0], derive(operators, functions, wrt, opRoot->children[1])});

                Operator *subOp = operators.find("-")->second.setOperands({multiplyOp1, multiplyOp2});
                Operator *powOp = operators.find("^")->second.setOperands({opRoot->children[1], new Expression("2")});
                Operator *divOp = operators.find("/")->second.setOperands({subOp, powOp});
                return divOp;
            }
            case '^': {
                //D/dx(b^c) = c * b^(c - 1) * D/dx(b)
                TreeNode *base = opRoot->children[0];
                TreeNode *derivedBase = derive(operators, functions, wrt, base);

                TreeNode *oldExponent = opRoot->children[1];
                Operator *newExponent = operators.find("-")->second.setOperands(
                        {ShuntingYard(operators, oldExponent->evaluate()->toString()), new Expression("1")});

                TreeNode *newCoefficient = ShuntingYard(operators, oldExponent->evaluate()->toString());
                Operator *newPowerOp = operators.find("^")->second.setOperands(
                        {ShuntingYard(operators, base->evaluate()->toString()), newExponent});

                Operator *multiplyOp = operators.find("*")->second.setOperands({newCoefficient, newPowerOp, derivedBase});
                return multiplyOp;
            }
        }
    }
        //It's an expression
    else {
        Expression *exRoot = dynamic_cast<Expression *>(root);
        if (isdigit(exRoot->toString()[0]))
            return new Expression("0");

        if (exRoot->toString() == wrt)
            return new Expression("1");

        return new Expression("0");
    }
    //Fail safely
    return new Expression("0");
}

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

    cout << "Testing expressions..." << endl;
    TreeNode *rootOfEquation = ShuntingYard(operators, "1/x^2");
    cout << "f(x) = " << rootOfEquation->evaluate()->toString() << endl;
    TreeNode *dRootOfEquation = derive(operators, functions, "x", rootOfEquation);
    cout << "f'(x) = " << ((dRootOfEquation == nullptr) ? "nullptr" : dRootOfEquation->evaluate()->toString()) << endl;

    cout << "\n\n\nTesting trig functions..." << endl;

    TreeNode * g = functions.find("tan")->second.setArguments({ShuntingYard(operators, "x^2")});
    cout << "g(x) = " << g->evaluate()->toString() << endl;
    TreeNode * gPrime = derive(operators, functions, "x", g);
    cout << "g'(x) = " << ((gPrime == nullptr) ? "nullptr" : gPrime->evaluate()->toString()) << endl;

    cout << "\n\n\nTesting composite functions..." << endl;
    TreeNode * composite = functions.find("log")->second.setArguments({functions.find("sin")->second.setArguments({new Expression("x")})});
    cout << "c(x) = " << composite->evaluate()->toString() << endl;
    TreeNode * compositePrime = derive(operators, functions, "x", composite);
    cout << "c'(x) = " << ((compositePrime == nullptr) ? "nullptr" : compositePrime->evaluate()->toString()) << endl;


    return 0;
}