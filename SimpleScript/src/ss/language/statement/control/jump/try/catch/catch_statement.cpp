//
//  catch_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/17/23.
//

#include "catch_statement.h"

namespace ss {
    //  CONSTRUCTORS

    catch_statement::catch_statement(const string key, const size_t statementc, statement_t** statementv) {
        if (!is_key(key))
            expect_error("key: " + key);
        
        this->key = key;
        
        if (statementc &&
            (statementv[statementc - 1]->compare(case_t) ||
             statementv[statementc - 1]->compare(default_t) ||
             statementv[statementc - 1]->compare(else_t) ||
             statementv[statementc - 1]->compare(else_if_t)))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void catch_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool catch_statement::analyze(command_processor* cp) const {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        if (!this->statementc)
            return false;
        
        size_t i = 0;
        while (i < this->statementc - 1 && !this->statementv[i]->analyze(cp))
            ++i;
        
        if (i != this->statementc - 1)
            logger_write("Unreachable code\n");
        
        this->statementv[this->statementc - 1]->analyze(cp);
        
        return false;
    }

    bool catch_statement::compare(const statement_type value) const {
        return value == catch_t;
    }

    string catch_statement::evaluate(command_processor* cp) {
        return encode(key);
    }

    string catch_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        this->is_paused.store(false);
        
        this->return_flag.store(false);
        
        for (size_t i = 0; i < this->statementc; ++i) {
            while (this->is_paused.load());
            
            this->statementv[i]->execute(cp);
            
            if (this->return_flag.load())
                break;
        }
        
        // while (this->is_paused.load());
        
        return null();
    }

    void catch_statement::set_break() {
        this->return_flag.store(true);
        this->parent->set_break();
    }

    void catch_statement::set_continue() {
        this->return_flag.store(true);
        this->parent->set_continue();
    }

    void catch_statement::set_goto(const string key) {
        this->return_flag.store(true);
        this->parent->set_goto(key);
    }

    void catch_statement::set_return(const string value) {
        this->return_flag.store(true);
        this->parent->set_return(value);
    }
}
