//
// Created by Doran on 3/29/2015.
//

#ifndef COMPUTERALGEBRASYSTEM_TREENODE_H
#define COMPUTERALGEBRASYSTEM_TREENODE_H

#include <vector>
#include <string>
#include <iostream>

using std::vector;
using std::string;

class Expression;

class TreeNode {
public:
    virtual Expression *evaluate() = 0;

    virtual ~TreeNode();
    virtual string toString() const = 0;

    vector<TreeNode *> children;
};

class Expression : public TreeNode {
public:
    Expression(string);

    Expression *evaluate() override;

    string toString() const override;

private:
    string contents;
};

#endif //COMPUTERALGEBRASYSTEM_TREENODE_H
