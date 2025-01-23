//
//  stack.h
//  SimpleScript
//
//  Created by Corey Ferguson on 6/7/23.
//

#ifndef stack_h
#define stack_h

#include "functions.h"

using namespace std;

namespace ss {
    template <typename T>
    class stack {
        //  MEMBER FIELDS
        
        T* data = new T[1];
        size_t n = 0;
    public:
        //  CONSTRUCTORS
        
        ~stack() {
            delete[] this->data;
        }
        
        //  MEMBER FUNCTIONS
        
        bool empty() const {
            return !this->size();
        }
        
        T pop() {
            T _top = this->top();
            
            for (size_t i = 0; i < this->n - 1; ++i)
                std::swap(this->data[i], this->data[i + 1]);
            
            --this->n;
            
            return _top;
        }
        
        void push(const T val) {
            if (is_pow(this->n, 2)) {
                T* tmp = new T[this->n * 2];
                
                for (size_t i = 0; i < this->n; ++i)
                    tmp[i] = this->data[i];
                
                delete[] this->data;
                this->data = tmp;
            }
            
            this->data[n] = val;
            
            for (size_t i = this->n; i > 0; --i)
                std::swap(this->data[i], this->data[i - 1]);
            
            ++this->n;
        }
        
        size_t size() const {
            return this->n;
        }
        
        void swap(stack& x) {
            T* _data = new T[pow2(this->size())];
            size_t _n = this->size();
            
            for (size_t i = 0; i < this->size(); ++i)
                _data[i] = this->data[i];
            
            delete[] this->data;
            
            this->n = x.size();
            this->data = new T[pow2(this->size())];
            
            for (size_t i = 0; i < this->size(); ++i)
                this->data[i] = x.data[i];
            
            delete[] x.data;
            
            x.n = _n;
            x.data = new T[x.size()];
            
            for (size_t i = 0; i < x.size(); ++i)
                x.data[i] = _data[i];
            
            delete[] _data;
        }
        
        T top() const {
            if (empty())
                throw error("Underflow");
            
            return this->data[0];
        }
    };
}

#endif /* stack_h */
