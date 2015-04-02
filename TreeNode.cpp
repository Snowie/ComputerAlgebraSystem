//
// Created by Doran on 3/29/2015.
//

#include "TreeNode.h"

TreeNode::~TreeNode(){
    std::cout << "TreeNode Deconstructor" << std::endl;
    for(int i = 0; i < children.size(); ++i){
        delete children[i];
        children[i] = nullptr;
    }
}

Expression::Expression(string e) {
    contents = e;
}

string Expression::toString() const {
    return contents;
}

Expression* Expression::evaluate() {
    return this;
}