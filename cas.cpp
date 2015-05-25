//
// Created by Doran on 5/25/2015.
//
#include "cas.h"

vector<string> cas::tokenize(std::string toTokenize) {
    std::regex pattern("([-+*^/()]|[^-+*^/()\\s]+)");
    std::sregex_token_iterator first{toTokenize.begin(), toTokenize.end(), pattern}, last;
    return {first, last};
}

TreeNode * cas::ShuntingYard(std::unordered_map<string, Operator> operators, std::unordered_map<string, Function> functions, string toParse) {
    std::stack<TreeNode *> operator_stack;
    std::stack<TreeNode *> operand_stack;
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

TreeNode * cas::derive(std::unordered_map<string, Operator> operators, std::unordered_map<string, Function> functions, string wrt, TreeNode * root) {
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
                Operator *multiplyOp1 = operators.find("*")->second.setOperands(
                        {derive(operators, functions, wrt, opRoot->children[0]),
                         ShuntingYard(operators, functions, opRoot->children[1]->evaluate()->toString())});

                Operator *multiplyOp2 = operators.find("*")->second.setOperands(
                        {ShuntingYard(operators, functions, opRoot->children[0]->evaluate()->toString()),
                         derive(operators, functions, wrt, opRoot->children[1])});

                Operator *subOp = operators.find("-")->second.setOperands({multiplyOp1, multiplyOp2});
                Operator *powOp = operators.find("^")->second.setOperands(
                        {ShuntingYard(operators, functions, opRoot->children[1]->evaluate()->toString()),
                         new Expression("2")});
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

TreeNode * cas::deriveFunction(std::unordered_map<string, Operator> operators, std::unordered_map<string, Function> functions, string wrt, TreeNode * root) {
    //D/dx(sin(x)) = cos(x) * x'
    if (root->toString() == "sin") {
        return operators.find("*")->second.setOperands(
                {functions.find("cos")->second.setArguments(
                        {ShuntingYard(operators, functions, root->children[0]->evaluate()->toString())}),
                 derive(operators, functions, wrt, root->children[0])});
    }
    //D/dx(cos(x)) = -sin(x) * x'
    if (root->toString() == "cos") {
        return operators.find("*")->second.setOperands(
                {functions.find("sin")->second.setArguments(
                        {ShuntingYard(operators, functions, root->children[0]->evaluate()->toString())}),
                 derive(operators, functions, wrt, root->children[0]), new Expression("-1")});
    }
    //D/dx(tan(x)) = sec(x)^2 * x'
    if (root->toString() == "tan") {
        Operator *powerOperator = operators.find("^")->second.setOperands(
                {functions.find("sec")->second.setArguments(
                        {ShuntingYard(operators, functions, root->children[0]->evaluate()->toString())}),
                 new Expression("2")});
        Operator *multiplyOperator = operators.find("*")->second.setOperands(
                {powerOperator, derive(operators, functions, wrt, root->children[0])});

        return multiplyOperator;
    }
    //D/dx(log(x)) = 1/x * x'
    if (root->toString() == "log") {
        Operator *divisionOperator = operators.find("/")->second.setOperands(
                {new Expression("1"), ShuntingYard(operators, functions, root->children[0]->evaluate()->toString())});
        Operator *multiplyOperator = operators.find("*")->second.setOperands(
                {divisionOperator, derive(operators, functions, wrt, root->children[0])});
        return multiplyOperator;
    }
    //TODO: simplify doesn't play nice  with more than two operands, fix this so we can fix this function.
    //D/dx(sec(x)) = sec(x) * tan(x) * x'
    if (root->toString() == "sec") {
        TreeNode *toSanitize = operators.find("*")->second.setOperands(
                {functions.find("sec")->second.setArguments(
                        {ShuntingYard(operators, functions, root->children[0]->evaluate()->toString())}),
                 functions.find("tan")->second.setArguments(
                         {ShuntingYard(operators, functions, root->children[0]->evaluate()->toString())}),
                 derive(operators, functions, wrt, root->children[0])});

        TreeNode *toRet = ShuntingYard(operators, functions, toSanitize->evaluate()->toString());
        delete toSanitize;
        return toRet;
    }

    Function failSafe("D/d" + wrt);
    return failSafe.setArguments({ShuntingYard(operators, functions, root->evaluate()->toString())});
}

TreeNode * cas::simplify(std::unordered_map<string, Operator> operators, std::unordered_map<string, Function> functions, TreeNode * root) {
    Function *f = dynamic_cast<Function *>(root);
    Operator *o = dynamic_cast<Operator *>(root);

    if (o != nullptr) {
        for (int i = 0; i < root->children.size(); ++i)
            root->children[i] = simplify(operators, functions, root->children[i]);

        //Debug?
        /*for (int i = 0; i < root->children.size(); ++i)
            if (root->children[i] == nullptr)
                std::cout << "PANIC" << std::endl;*/

        if (o->toString() == "*") {
            for (int i = 0; i < root->children.size(); ++i) {
                //Useless, 1 provides no information in multiplication unless by itself.
                if (root->children[i]->toString() == "1") {
                    //TODO: DIRTY HACK, FIX
                    TreeNode *otherOne;
                    if (i == 0) {
                        otherOne = root->children[1];
                        root->children[1] = nullptr;
                    }
                    else {
                        otherOne = root->children[0];
                        root->children[0] = nullptr;
                    }
                    delete root;
                    root = otherOne;
                    return root;
                }
                //Terminal, zero dominates expression.
                if (root->children[i]->toString() == "0") {
                    delete root;
                    root = new Expression("0");
                    return root;
                }
            }
        }
    }
    return root;
}

std::stack<TreeNode*> cas::addNode(std::stack<TreeNode *> operand_stack, Operator * o) {
    TreeNode *r = operand_stack.top();
    operand_stack.pop();
    TreeNode *l = operand_stack.top();
    operand_stack.pop();
    operand_stack.push(o->setOperands({l, r}));
    return operand_stack;
}