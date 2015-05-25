//
// Created by Doran on 5/25/2015.
//
#ifndef COMPUTERALGEBRASYSTEM_CAS_H
#define COMPUTERALGEBRASYSTEM_CAS_H

#include "TreeNode.h"
#include "Operator.h"
#include "Function.h"
#include <unordered_map>
#include <stack>
#include <regex>

/**
 * @file cas.h
 * Definitions of relevant CAS utility functions
 */

namespace cas {
    /**
    *  @function
    *  A function to tokenize and split a string on operators
    *  @param toTokenize The string to be tokenized
    *  @return A vector of string tokens
    */
    vector<string> tokenize(string);

    TreeNode *ShuntingYard(std::unordered_map<string, Operator>, std::unordered_map<string, Function>, string);

    /**
    * @function
    * A function for computing the derivative of an expression
    * @param operators An unordered map of operators
    * @param functions An unordered map of functions
    * @param wrt A string containing the variable for which we should derive with respect to
    * @param root A pointer to the root of the equation
    * @return A TreeNode pointer that represents the derivative of the supplied expression wrt 'wrt'
    */
    TreeNode *derive(std::unordered_map<string, Operator>, std::unordered_map<string, Function>, string, TreeNode *);

    /**
    * @function
    * A helper function that extends derive for the purposes of functions
    * @param operators An unordered map of operators
    * @param functions An unordered map of functions
    * @param wrt A string containing the variable for which we should derive with respect to
    * @param root A pointer to the root of the function
    * @return A TreeNode pointer representing the derivative of the requested function
    */
    TreeNode *deriveFunction(std::unordered_map<string, Operator>, std::unordered_map<string, Function>, string, TreeNode *);

    /**
    * @function
    * A function to simplify mathematical expressions
    * @param operators An unordered map of operators
    * @param functions An unordered map of functions
     * @param root A TreeNode that represents an expression
    * @return A simplified version of the TreeNode passed
    */
    TreeNode *simplify(std::unordered_map<string, Operator>, std::unordered_map<string, Function>, TreeNode *);

    /**
    * @function
    * A function to create nodes for operators and their operands
    * @param operand_stack a stack of TreeNode pointers.
    * @param o An operator to apply.
    * @return A copy of the stack with the new operator on it
    */
    std::stack<TreeNode *> addNode(std::stack<TreeNode *>, Operator *);
}
#endif //COMPUTERALGEBRASYSTEM_CAS_H
