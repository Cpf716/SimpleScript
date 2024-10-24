//
//  function.h
//  SimpleScript
//
//  Created by Corey Ferguson on 12/12/22.
//

#ifndef function_h
#define function_h

#include "function_t.h"

namespace ss {
    class function: public function_t {
        //  MEMBER FIELDS
        
        std::function<string(size_t, string *)> _function;
    public:
        //  CONSTRUCTORS
        
        function(const string name, const std::function<string(size_t, string *)> function) {
#if DEBUG_LEVEL
            assert(name.length());
#endif
            this->rename(name);
            this->_function = function;
            this->consume();
        }
        
        void close() {
            delete this;
        }
        
        //  MEMBER FUNCTIONS
        
        string call(const size_t argc, string* argv) {
            return this->_function(argc, argv);
        }
        
        void exit() { }
        
        size_t get_level() const {
            unsupported_error("get_level()");
            return 0;
        }
        
        void kill() { }
 
        void set_level(const size_t level) {
            unsupported_error("set_level()");
        }
        
        void set_pause(const bool value) { }
    };
}

#endif /* function_h */
