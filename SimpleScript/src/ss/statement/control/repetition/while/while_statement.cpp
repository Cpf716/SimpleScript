//
//  while_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 6/12/23.
//

#include "while_statement.h"

namespace ss {
    //  CONSTRUCTORS

    while_statement::while_statement(const string expression, const size_t statementc, statement_t** statementv) {
        if (expression.empty())
            expect_error("expression");
        
        this->expression = expression;
        
        if (statementc && is_clause(statementv[statementc - 1]))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
    }

    void while_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool while_statement::analyze(command_processor* cp) const {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        if (!this->statementc) {
            logger_write("'while' statement has empty body\n");
            
            return false;
        }
        
        size_t i = 0;
        while (i < this->statementc - 1 && !this->statementv[i]->analyze(cp))
            ++i;
        
        if (i != this->statementc - 1)
            logger_write("Unreachable code\n");
                
        if (this->statementv[i]->analyze(cp) &&
            (this->statementv[i]->compare(break_t) ||
             this->statementv[i]->compare(exit_t) ||
             this->statementv[i]->compare(return_t)))
            logger_write("'while' statement will execute at most once\n");
        
        return false;
    }

    string while_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string while_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        this->pause_flag = false;
        this->return_flag = false;
        
        while (1) {
            while (this->pause_flag);
            
            size_t state = cp->get_state();
            
            if (!ss::evaluate(cp->evaluate(this->expression))) {
                cp->set_state(state);
                break;
            }
            
            this->continue_flag = false;
            
            for (size_t i = 0; i < this->statementc; ++i) {
                while (this->pause_flag);
                
                this->statementv[i]->execute(cp);
                
//                while (this->should_pause);
                
                if (this->return_flag || this->continue_flag)
                    break;
            }
            
//            while (this->should_pause);
            
            if (this->return_flag) {
                cp->set_state(state);
                break;
            }
            
            cp->set_state(state);
        }
        
        return null();
    }

    bool while_statement::compare(const statement_type value) const {
        return false;
    }

    void while_statement::set_break() {
        this->return_flag = true;
    }

    void while_statement::set_continue() {
        this->continue_flag = true;
    }

    void while_statement::set_goto(const string key) {
        this->return_flag = true;
        this->parent->set_goto(key);
    }

    void while_statement::set_return(const string value) {
        this->return_flag = true;
        this->parent->set_return(value);
    }
}
