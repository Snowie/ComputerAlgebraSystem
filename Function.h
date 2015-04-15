//
// Created by Doran on 4/15/2015.
//

#ifndef COMPUTERALGEBRASYSTEM_FUNCTION_H
#define COMPUTERALGEBRASYSTEM_FUNCTION_H


#include "TreeNode.h"

class Function : public TreeNode {
public:
    Function(string);

    Expression *evaluate() override;

    string toString() const override;

    Function *setArguments(vector<TreeNode *>) const;

private:
    Function(const Function *const, vector<TreeNode *>);

    string signature;
};


#endif //COMPUTERALGEBRASYSTEM_FUNCTION_H
