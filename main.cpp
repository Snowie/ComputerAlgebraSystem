#include <iostream>
#include <unordered_map>
#include <stack>
#include <regex>
#include "TreeNode.h"
#include "Operator.h"
#include "Function.h"
/**
 * @file main.cpp
 * The starting point of execution for CAS
 */
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

/**
 *  @function
 *  A function to tokenize and split a string on operators
 *  @param toTokenize The string to be tokenized
 *  @return A vector of string tokens
 */
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
        //Spaces don't do anything!
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
                if (popped->toString() == "(")
                    break;
                else if (Operator *o = dynamic_cast<Operator *>(popped))
                    operand_stack = addNode(operand_stack, o);
            }
            continue;
        }

        //Start a new recursive call to ShuntingYard and parse the arguments of the function, add the new function
        //  to the operand stack upon completion
        if (functions.find(s) != functions.end()) {
            //Our stop condition
            int rightParenRequiredCount = 0;

            string functionSubstring = "";

            //How far forward should we go once done?
            int tokenSkip = 0;

            //Iterate through until all left parentheses have been matched
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

            //The arguments of the function are the return value of shunting yard
            Function *f = functions.find(s)->second.setArguments(
                    {ShuntingYard(operators, functions, functionSubstring)});

            //Place the function on the stack like any other literal/expression
            operand_stack.push(f);

            //Don't reparse the skipped expression
            i += tokenSkip;
            continue;
        }

        //Is the token an operator?
        if (operators.find(s) != operators.end()) {
            Operator *o1 = &(operators.find(s)->second);
            Operator *o2 = nullptr;

            //While there are operators...
            while (!operator_stack.empty()) {
                o2 = &(operators.find(operator_stack.top()->toString())->second);

                //This is no longer an operator!
                if (operators.find(operator_stack.top()->toString()) == operators.end())
                    break;

                //If o1 is right associative and is of the same or higher precedence than o1...
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

/**
 * @function
 * A helper function that extends derive for the purposes of functions
 * @param operators An unordered map of operators
 * @param functions An unordered map of functions
 * @param wrt A string containing the variable for which we should derive with respect to
 * @param root A pointer to the root of the function
 * @return A TreeNode pointer representing the derivative of the requested function
 */
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

/**
 * @function
 * A function for computing the derivative of an expression
 * @param operators An unordered map of operators
 * @param functions An unordered map of functions
 * @param wrt A string containing the variable for which we should derive with respect to
 * @param root A pointer to the root of the equation
 * @return A TreeNode pointer that represents the derivative of the supplied expression wrt 'wrt'
 */
TreeNode *derive(unordered_map<string, Operator> operators, unordered_map<string, Function> functions, string wrt,
                 TreeNode *root) {
    Function *functionRoot = dynamic_cast<Function *>(root);
    Operator *opRoot = dynamic_cast<Operator *>(root);
    if (functionRoot != nullptr)
        return deriveFunction(operators, functions, wrt, functionRoot);

    //It's an operator
    if (opRoot != nullptr) {
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
                if(opRoot->children.size() < 2 || opRoot->children.size() > 2)
                    cout << "Huh..?" << endl;
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

//TODO, either allow for more than two operators on ShuntingYard, or don't use it here!
TreeNode *simplify(unordered_map<string, Operator> operators, unordered_map<string, Function> functions,
                    TreeNode *root) {
    Function *f = dynamic_cast<Function *>(root);
    Operator *o = dynamic_cast<Operator *>(root);

    if (f != nullptr) {
        for (int i = 0; i < root->children.size(); ++i)
            root->children[i] = simplify(operators, functions, root->children[i]);
    }

    if(o != nullptr) {
        for(int i = 0; i < root->children.size(); ++i)
            root->children[i] = simplify(operators, functions, root->children[i]);

        if(o->toString() == "*") {
            for (int i = 0; i < o->children.size(); ++i) {
                //Identity: 1
                if(o->children[i]->toString() == "1") {
                    auto it = o->children.begin() + i;
                    delete o->children[i];
                    o->children.erase(it);
                    --i;

                    //If we were only multiplying ones, make sure we don't just lose all of them
                    if(o->children.empty()) {
                        delete o;
                        return new Expression("1");
                    }

                    continue;
                }

                //Zero product is always zero (unless infinity)
                if(o->children[i]->toString() == "0") {
                    delete root;
                    return new Expression("0");
                }
            }
        }

        //If an operator only has one child, drag that child up
        if(o->children.size() == 1){
            TreeNode * toBringUp = o->children[0];

            o->children.erase(o->children.begin());
            delete root;

            return toBringUp;
        }
    }

    return root;
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

    //cout << "Before loop" << endl;

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

        //Crash happens in these two lines
        /*cout << "Before Derive..." << endl;
        TreeNode * fTestPrime = derive(operators, functions, "x", ShuntingYard(operators, functions, f->evaluate()->toString()));
        cout << "After Derive..." << endl;
        fTestPrime = simplify(operators, functions, fTestPrime);*/

        //But not in here
        TreeNode *fPrime = ShuntingYard(operators, functions, derive(operators, functions, "x", f)->evaluate()->toString());
        //TreeNode *fPrime = derive(operators, functions, "x", ShuntingYard(operators, functions, f->evaluate()->toString()));
        fPrime = simplify(operators, functions, fPrime);

        cout << "f(x) = " << f->evaluate()->toString() << endl;
        cout << "f'(x) = " << ((fPrime == nullptr) ? "nullptr" : fPrime->evaluate()->toString()) << endl;

        delete f;
        delete fPrime;
    }

    return 0;
}