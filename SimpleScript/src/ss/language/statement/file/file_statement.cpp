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
            logger_write("Unreachable code");
        
        this->statementv[statementc - 1]->analyze(cp);
        
        return false;
    }

    // Begin Enhancement 1 - Thread safety - 2025-01-22
    void file_statement::check_paused() {
        if (this->is_paused.load()) {
            this->parent->set_paused();

            while (this->is_paused.load());
        }
    }
    // End Enhancement 1

    bool file_statement::compare(const statement_type value) const {
        unsupported_error("compare()");
        return false;
    }

    string file_statement::evaluate(command_processor* cp) {
        unsupported_error("evaluate()");
        return null();
    }

    string file_statement::execute(command_processor* cp) {
#if DEBUG_LEVEL
        assert(cp != NULL);
#endif
        analyze(cp);
        
        this->value = null();
        
        this->is_paused.store(false);
        
        this->return_flag.store(false);
        
        for (size_t i = 0; i < statementc; ++i) {
            // Begin Enhancement 1 - Thread safety - 2025-01-22
            this->check_paused();
            // End Enhancement 1

            this->statementv[i]->execute(cp);
            
            if (this->return_flag.load())
                break;
        }
        
        return value;
    }

    void file_statement::exit() {
        this->return_flag.store(true);
        this->parent->exit();
    }

    size_t file_statement::get_level() const {
        return this->parent->get_level();
    };

    void file_statement::kill() {
        this->return_flag.store(true);
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->kill();
    }

    void file_statement::set_break() {
        throw error("break cannot be used outside of a loop");
    }

    void file_statement::set_continue() {
        throw error("continue cannot be used outside of a loop");
    }

    void file_statement::set_goto(const string key) {
        throw error("goto cannot be used outside of a switch");
    }

    void file_statement::set_level(const size_t level) {
        this->parent->set_level(level);
    }

    void file_statement::set_parent(statement_t* parent) {
        unsupported_error("set_parent()");
    }

    // Begin Enhancement 1 - Thread safety - 2025-01-22
    void file_statement::set_paused() {
        this->parent->set_paused();
    }
    // End Enhancement 1

    void file_statement::set_paused(const bool value) {
        this->is_paused.store(value);
        
        for (size_t i = 0; i < this->statementc; ++i)
            this->statementv[i]->set_paused(value);
    }

    void file_statement::set_return(const string value) {
        this->value = value;
        this->return_flag.store(true);
    }
}
