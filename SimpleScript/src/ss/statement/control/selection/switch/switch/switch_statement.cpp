//
//  switch_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/31/24.
//

#include "switch_statement.h"

namespace ss {
    //  CONSTRUCTORS

    switch_statement::switch_statement(const string expression, const size_t statementc, statement_t** statementv) {
        if (expression.empty())
            expect_error("expression");
        
        this->expression = expression;
        
        if (!statementc)
            expect_error("'default'");
        
        if ((!statementv[0]->compare(case_t) &&
             !statementv[0]->compare(default_t)) ||
             statementv[statementc - 1]->compare(catch_t) ||
             statementv[statementc - 1]->compare(else_t) ||
             statementv[statementc - 1]->compare(else_if_t) ||
             statementv[statementc - 1]->compare(finally_t))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
        
        std::function<void(const string, const size_t)> add_case = [&](const string key, const size_t pos) {
            if (find_case(key) != -1)
                defined_error(key);
            
            // initialize
            if (this->named_casev == NULL)
                this->named_casev = new pair<string, size_t>*[1];
                
            // resize
            else if (is_pow(this->named_casec, 2)) {
                pair<string, size_t>** tmp = new pair<string, size_t>*[this->named_casec * 2];
                
                for (size_t j = 0; j < this->named_casec; ++j)
                    tmp[j] = this->named_casev[j];
                
                delete[] this->named_casev;
                
                this->named_casev = tmp;
            }
            
            this->named_casev[this->named_casec] = new pair<string, size_t>(key, pos);
            
            // sort
            for (size_t j = named_casec++; j > 0 && this->named_casev[j]->first < this->named_casev[j - 1]->first; --j)
                swap(this->named_casev[j], this->named_casev[j - 1]);
        };
                
        for (size_t i = 0; i < this->statementc -  1; ++i) {
            case_statement* statement = (case_statement *)this->statementv[i];
            
            if (!statement->get_key().empty())
                add_case(statement->get_key(), i);
        }
        
        if (this->statementv[this->statementc - 1]->compare(case_t)) {
            case_statement* statement = (case_statement *)this->statementv[this->statementc - 1];
            
            if (!statement->get_key().empty())
                add_case(statement->get_key(), this->statementc - 1);
        }
    }

    void switch_statement::close() {
        for (size_t i = 0; i < this->named_casec; ++i)
            delete this->named_casev[i];
        
        delete[] this->named_casev;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool switch_statement::analyze(command_processor* cp) const {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->analyze(cp);
        
        return false;
    }

    bool switch_statement::compare(const statement_type value) const {
        return false;
    }

    string switch_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string switch_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        this->cp = cp;
        this->should_goto = false;
        this->return_flag = false;
        
        size_t state = this->cp->get_state();
        string value = this->cp->evaluate(this->expression);
        
        size_t i = 0;
        while (i < this->statementc - 1 && value != this->statementv[i]->evaluate(this->cp))
            ++i;
        
        if (i == this->statementc - 1) {
            size_t _state = this->cp->get_state();
            
            if (this->statementv[this->statementc - 1]->compare(default_t) || value == this->statementv[i]->evaluate(this->cp))
                this->statementv[this->statementc - 1]->execute(this->cp);
            
            this->cp->set_state(_state);
        } else {
            for (size_t j = i; j < this->statementc; ++j) {
                while (this->pause_flag);
                
                size_t _state = this->cp->get_state();
                
                this->statementv[j]->execute(this->cp);
                
                this->cp->set_state(_state);
    
                if (this->should_goto || this->return_flag)
                    break;
            }
        }
        
        cp->set_state(state);
        
        return null();
    }

    int switch_statement::find_case(const string key) {
        return this->find_case(key, 0, this->named_casec);
    }

    int switch_statement::find_case(const string key, const size_t beg, const size_t end) {
        if (beg == end)
            return -1;
        
        size_t len = floor((end - beg) / 2);
        
        if (this->named_casev[beg + len]->first == key)
            return (int)(beg + len);
        
        if (this->named_casev[beg + len]->first > key)
            return this->find_case(key, beg, beg + len);
        
        return this->find_case(key, beg + len + 1, end);
    }

    void switch_statement::set_break() {
        this->return_flag = true;
    }

    void switch_statement::set_continue() {
        this->return_flag = true;
        this->parent->set_continue();
    }

    void switch_statement::set_goto(const string key) {
        this->should_goto = true;
//        this->should_return = false;
        
        if (key == "default") {
            if (!this->statementv[this->statementc - 1]->compare(default_t))
                undefined_error("default");
            
            size_t state = this->cp->get_state();
            
            this->statementv[this->statementc - 1]->execute(this->cp);
            
            this->cp->set_state(state);
        } else {
            int pos = find_case(key);
            if (pos == -1)
                undefined_error(key);
            
            for (size_t j = this->named_casev[pos]->second; j < this->statementc; ++j) {
                while (this->pause_flag);
                
                size_t state = this->cp->get_state();
                
                this->statementv[j]->execute(this->cp);
                
                this->cp->set_state(state);
                
                if (this->return_flag)
                    break;
            }
            
        }
    }

    void switch_statement::set_return(const string value) {
        this->return_flag = true;
        this->parent->set_return(value);
    }
}
