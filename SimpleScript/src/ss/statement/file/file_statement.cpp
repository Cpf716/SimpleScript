//
//  file_statement.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 10/14/23.
//

#include "file_statement.h"

namespace ss {
    //  CONSTRUCTORS

    file_statement::file_statement(function_t* parent, const size_t statementc, statement_t** statementv) {
#if DEBUG_LEVEL
        assert(parent != NULL);
#endif
        this->parent = parent;
        
        if (statementc && is_clause(statementv[statementc - 1]))
            expect_error("expression");
        
        this->statementc = statementc;
        this->statementv = statementv;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->set_parent(this);
    }

    void file_statement::close() {
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->close();
        
        delete[] this->statementv;
        delete this;
    }

    //  MEMBER FUNCTIONS

    bool file_statement::analyze(command_processor* cp) const {
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
        
        this->statementv[statementc - 1]->analyze(cp);
        
        return false;
    }

    bool file_statement::compare(const statement_type value) const {
        unsupported_error("compare()");
        return false;
    }

    string file_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return empty();
    }

    string file_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        analyze(cp);
        
        this->result = encode(to_string(undefined_t));
        this->should_pause = false;
        this->should_return = false;
        
        for (size_t i = 0; i < statementc; ++i) {
            while (this->should_pause);
            
            this->statementv[i]->execute(cp);
            
//            while (this->should_pause);
            
            if (this->should_return)
                break;
        }
        
        return result;
    }

    void file_statement::exit() {
        this->should_return = true;
        this->parent->exit();
    }

    size_t file_statement::get_level() const {
        return this->parent->get_level();
    };

    void file_statement::kill() {
        this->should_return = true;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->kill();
    }

    void file_statement::set_break() {
        throw error("break cannot be used outside of a loop");
    }

    void file_statement::set_continue() {
        throw error("continue cannot be used outside of a loop");
    }

    void file_statement::set_level(const size_t level) {
        this->parent->set_level(level);
    }

    void file_statement::set_parent(statement_t* parent) {
        unsupported_error("set_parent()");
    }

    void file_statement::set_pause(const bool pause) {
        this->should_pause = pause;
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->set_pause(pause);
    }

    void file_statement::set_return(const string result) {
        this->result = result;
        this->should_return = true;
    }
}
