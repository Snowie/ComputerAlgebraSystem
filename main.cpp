#include <iostream>
#include <unordered_map>
#include <stack>
#include <regex>
#include "TreeNode.h"
#include "Operator.h"
#include "Function.h"

using namespace std;

TreeNode *derive(unordered_map<string, Operator>, unordered_map<string, Function>, string, TreeNode *);

/**
 * @function
 * A function to create nodes for operators and their operands
 * @param operand_stack a stack of TreeNode pointers.
 * @param o An operator to apply.
 * @return A copy of the stack with the new operator on it
 */
stack<TreeNode *> addNode(stack<TreeNode *> operand_stack, Operator *o) {
    TreeNode *r = operand_stack.top();
    operand_stack.pop();
    TreeNode *l = operand_stack.top();
    operand_stack.pop();
    operand_stack.push(o->setOperands({l, r}));
    return operand_stack;
}

vector<string> tokenize(string toTokenize) {
    regex pattern("([-+*^/()]|[^-+*^/()\\s]+)");
    sregex_token_iterator first{toTokenize.begin(), toTokenize.end(), pattern}, last;
    return {first, last};
}

TreeNode *ShuntingYard(unordered_map<string, Operator> operators, unordered_map<string, Function> functions,
                       string toParse) {
    stack<TreeNode *> operator_stack;
    stack<TreeNode *> operand_stack;
    vector<string> tokens = tokenize(toParse);

    for (int i = 0; i < tokens.size(); ++i) {
        string s = tokens[i];
        if (s == " ")
            continue;

        if (s == "(") {
            operator_stack.push(new Operator(9, "(", false));
            continue;
        }

        if (s == ")") {
            while (!operator_stack.empty()) {
                TreeNode *popped = operator_stack.top();
                operator_stack.pop();
                if (popped->toString() == "(") {
                    break;
                }
                else {
                    if (Operator *o = dynamic_cast<Operator *>(popped)) {
                        //cout << "Awesome O" << endl;
                        operand_stack = addNode(operand_stack, o);
                    }
                }
            }
            continue;
        }

        //Witchcraft!
        if (functions.find(s) != functions.end()) {
            int rightParenRequiredCount = 0;
            string functionSubstring = "";
            int tokenSkip = 0;
            for (int j = i + 1; j < tokens.size(); ++j) {
                tokenSkip++;
                if (tokens[j] == "(")
                    rightParenRequiredCount++;
                if (tokens[j] == ")")
                    rightParenRequiredCount--;
                functionSubstring += tokens[j];

                if (rightParenRequiredCount == 0)
                    break;
            }
            Function *f = functions.find(s)->second.setArguments(
                    {ShuntingYard(operators, functions, functionSubstring)});

            //Place the function on the stack like any other literal/expression
            operand_stack.push(f);

            //Don't reparse the skipped expression
            i += tokenSkip;
            continue;
        }

        if (operators.find(s) != operators.end()) {
            Operator *o1 = &(operators.find(s)->second);
            Operator *o2 = nullptr;
            while (!operator_stack.empty() && (o2 = &(operators.find(operator_stack.top()->toString())->second))) {
                if (operators.find(operator_stack.top()->toString()) == operators.end())
                    break;

                if ((!o1->isRightAssociative() && 0 == o1->comparePrecedence(o2)) || o1->comparePrecedence(o2) < 0) {
                    operator_stack.pop();
                    operand_stack = addNode(operand_stack, o2);
                } else {
                    break;
                }
            }
            operator_stack.push(&(operators.find(s)->second));
        }
        else {
            operand_stack.emplace(new Expression(s));
        }
    }
    while (!operator_stack.empty()) {
        Operator *o = dynamic_cast<Operator *>(operator_stack.top());
        operand_stack = addNode(operand_stack, o);
        operator_stack.pop();
    }

    if (!operand_stack.empty())
        return operand_stack.top();
    else
        return new Expression("0");
}

TreeNode *deriveFunction(unordered_map<string, Operator> operators, unordered_map<string, Function> functions,
                         string wrt, TreeNode *root) {

    //D/dx(sin(x)) = cos(x) * x'
    if (root->toString() == "sin") {
        return operators.find("*")->second.setOperands(
                {functions.find("cos")->second.setArguments(root->children),
                 derive(operators, functions, wrt, root->children[0])});
    }
    //D/dx(cos(x)) = -sin(x) * x'
    if (root->toString() == "cos") {
        return operators.find("*")->second.setOperands(
                {functions.find("sin")->second.setArguments(root->children),
                 derive(operators, functions, wrt, root->children[0]), new Expression("-1")});
    }
    //D/dx(tan(x)) = sec(x)^2 * x'
    if (root->toString() == "tan") {
        Operator *powerOperator = operators.find("^")->second.setOperands(
                {functions.find("sec")->second.setArguments({root->children[0]}), new Expression("2")});
        Operator *multiplyOperator = operators.find("*")->second.setOperands(
                {powerOperator, derive(operators, functions, wrt, root->children[0])});

        return multiplyOperator;
    }
    //D/dx(log(x)) = 1/x * x'
    if (root->toString() == "log") {
        Operator *divisionOperator = operators.find("/")->second.setOperands({new Expression("1"), root->children[0]});
        Operator *multiplyOperator = operators.find("*")->second.setOperands(
                {divisionOperator, derive(operators, functions, wrt, root->children[0])});
        return multiplyOperator;
    }
    //D/dx(sec(x)) = sec(x) * tan(x) * x'
    if (root->toString() == "sec") {
        return operators.find("*")->second.setOperands(
                {functions.find("sec")->second.setArguments(root->children),
                 functions.find("tan")->second.setArguments(root->children),
                 derive(operators, functions, wrt, root->children[0])});
    }

    Function failSafe("D/d" + wrt);
    return failSafe.setArguments({root});
}

//Returns a TreeNode that represents the derivative of the supplied function wrt 'wrt'
TreeNode *derive(unordered_map<string, Operator> operators, unordered_map<string, Function> functions, string wrt,
                 TreeNode *root) {
    Function *functionRoot = dynamic_cast<Function *>(root);
    if (functionRoot != nullptr)
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
                    vector<TreeNode *> copyOfChildren = ShuntingYard(operators, functions,
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
                        {ShuntingYard(operators, functions, oldExponent->evaluate()->toString()), new Expression("1")});

                TreeNode *newCoefficient = ShuntingYard(operators, functions, oldExponent->evaluate()->toString());
                Operator *newPowerOp = operators.find("^")->second.setOperands(
                        {ShuntingYard(operators, functions, base->evaluate()->toString()), newExponent});

                Operator *multiplyOp = operators.find("*")->second.setOperands(
                        {newCoefficient, newPowerOp, derivedBase});
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
    functions.emplace("harmonic", Function("harmonic"));

    while (true) {
        cout << "\n\n\nEnter the function you would like to parse, or type quit to quit: ";
        string expression;
        //cin >> expression;
        getline(cin, expression);
        if (expression == "quit") {
            cout << "Caught exit command, exiting..." << endl;
            break;
        }
        TreeNode *f = ShuntingYard(operators, functions, expression);
        TreeNode *fPrime = derive(operators, functions, "x", f);
        cout << "f(x) = " << f->evaluate()->toString() << endl;
        cout << "f'(x) = " << ((fPrime == nullptr) ? "nullptr" : fPrime->evaluate()->toString()) << endl;

    }


    return 0;
}