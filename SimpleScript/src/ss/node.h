//
//  node.h
//  SimpleScript
//
//  Created by Corey Ferguson on 7/25/23.
//

#ifndef node_h
#define node_h

#include "array.h"

namespace ss {
    template <typename T>
    class node {
        //  MEMBER FIELDS
        
        ss::array<node*> children;
        T _data;
        node* _parent = NULL;
    public:
        //  CONSTRUCTORS
        
        node(const T data, node* parent) {
            this->_data = data;
            this->_parent = parent;
            
            if (this->parent() != NULL)
                this->parent()->children.push(this);
        }
        
        void close() {
            for (size_t i = 0; i < this->children.size(); ++i)
                this->children[i]->close();
            
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        T data() const {
            return this->_data;
        }
        
        node* parent() const {
            return this->_parent;
        }
    };
}

#endif /* node_h */
