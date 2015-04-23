//
// Created by Doran on 4/15/2015.
//

#ifndef COMPUTERALGEBRASYSTEM_FUNCTION_H
#define COMPUTERALGEBRASYSTEM_FUNCTION_H


#include "TreeNode.h"

/**
 * @class
 * A class to represent functions as TreeNodes
 */
class Function : public TreeNode {
public:
    /**
     * @function
     * Public constructor to function, only initializes representation
     */
    Function(string);

    /**
     * @function
     * Performs a DFS to make an expression containing the tree from this node
     * @return An expression node containing the Tree in string form
     */
    Expression *evaluate() override;

    /**
     * @function
     * @return The representation of the function as a string, not including arguments
     */
    string toString() const override;

    /**
     * @function
     * Set the arguments for the function
     * @param arguments Contains arguments for the new function
     * @return A pointer to the newly constructed function
     */
    Function *setArguments(vector<TreeNode *>) const;

private:
    /**
     * @function
     * Private constructor for function that returns a copy with the new arguments
     */
    Function(const Function *const, vector<TreeNode *>);

    string signature;
};


#endif //COMPUTERALGEBRASYSTEM_FUNCTION_H
