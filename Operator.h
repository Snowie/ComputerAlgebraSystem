//
// Created by Doran on 4/2/2015.
//

#ifndef COMPUTERALGEBRASYSTEM_OPERATOR_H
#define COMPUTERALGEBRASYSTEM_OPERATOR_H

#include "TreeNode.h"
#include "Operator.h"

class Operator : public TreeNode {
public:
    Operator(int, string, bool);

    Expression *evaluate() override;

    int getPrecedence() const;

    bool isRightAssociative() const;

    int comparePrecedence(Operator *) const;

    Operator *setOperands(vector<TreeNode *>) const;

    string toString() const override;

private:
    Operator(const Operator *const, vector<TreeNode *>);

    bool rightAssociative;
    int precedence;
    string representation;
};


#endif //COMPUTERALGEBRASYSTEM_OPERATOR_H
