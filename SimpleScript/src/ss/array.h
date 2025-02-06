//
//  array.h
//  SimpleScript
//
//  Created by Corey Ferguson on 1/31/23.
//

#ifndef array_h
#define array_h

#include <iostream>

namespace ss {
    template <typename T>
    class array {
        //  MEMBER FIELDS
        
        size_t _capacity,
               _size = 0;
        T*     _data = NULL;
    public:
        //  CONSTRUCTORS
        
        array() {
            _data = new T[this->_capacity = 1];
        }
        
        array(const ss::array<T>& data) {
            this->_capacity = data.capacity();
            this->_data = new T[this->capacity()];
            this->_size = data.size();
            
            for (size_t i = 0; i < this->size(); ++i)
                this->_data[i] = data._data[i];
        }
        
        array(const size_t length, T* source) {
            this->_capacity = length;
            this->_data = new T[this->capacity()];
            this->_size = length;
            
            for (size_t i = 0; i < this->size(); ++i)
                this->_data[i] = source[i];
        }
        
        array(const size_t capacity) {
            this->_data = new T[this->_capacity = capacity];
        }
        
        ~array() {
            delete[] this->_data;
        }
        
        //  OPERATORS
        
        T& operator[](const size_t index) {
            if (index >= this->size())
                throw std::out_of_range(std::to_string(index));
            
            return this->_data[index];
        }
        
        //  MEMBER FUNCTIONS
        
        size_t capacity() const {
            return this->_capacity;
        };
        
        void clear() {
            delete[] this->_data;
            
            this->_data = new T[this->_capacity = 1];
            this->_size = 0;
        }
        
        void ensure_capacity(const size_t new_capacity) {
            if (this->capacity() >= new_capacity)
                return;
            
            T* tmp = new T[this->_capacity = new_capacity];
            
            for (size_t i = 0; i < this->size(); ++i)
                tmp[i] = this->_data[i];
            
            delete[] this->_data;
            this->_data = tmp;
        }
        
        int index_of(const T item) const {
            int i = 0;
            while (i < this->size() && this->_data[i] != item)
                ++i;
            
            return i == this->size() ? -1 : i;
        }
        
        void insert(const size_t index, const T item) {
            if (index > this->size())
                throw std::out_of_range(std::to_string(index));
            
            if (this->size() == this->capacity())
                this->ensure_capacity(this->capacity() ? this->capacity() * 2 : 1);
            
            this->_data[this->_size] = item;
            
            for (size_t i = this->_size++; i > index; --i)
                std::swap(this->_data[i], this->_data[i - 1]);
        }
        
        int last_index_of(const T item) const {
            int i = (int)this->size() - 1;
            while (i >= 0 && this->_data[i] != item)
                --i;
            
            return i;
        }
        
        void push(const T item) {
            if (this->size() == this->capacity())
                this->ensure_capacity(this->capacity() ? this->capacity() * 2 : 1);
            
            this->_data[this->_size++] = item;
        }
        
        void remove(const size_t index) {
            if (index >= this->size())
                throw std::out_of_range(std::to_string(index));
            
            for (size_t i = index; i < this->size() - 1; ++i)
                std::swap(this->_data[i], this->_data[i + 1]);
            
            --this->_size;
        }
        
        void resize(const size_t new_size) {
            this->ensure_capacity(new_size);
            
            while (this->size() < new_size)
                this->push(std::string());
            
            while (this->size() > new_size)
                this->remove(new_size);
        }
        
        void shrink_to_fit() {
            T* tmp = new T[this->size()];
            
            for (size_t i = 0; i < this->size(); ++i)
                tmp[i] = this->_data[i];
            
            delete[] this->_data;
            
            this->_data = tmp;
            this->_capacity = this->size();
        }
        
        size_t size() const {
            return this->_size;
        };
    };
}

#endif /* array_h */
