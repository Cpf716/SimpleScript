//
//  system_loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef system_loader_h
#define system_loader_h

#include "command_processor.h"

namespace ss {
    // Begin Enhancement 1-1 - Thread safety - 2025-01-23
    // TYPEDEF

    class system_loader {
        // MEMBER FIELDS
        
        int                                         stopwatchc = 0;
        vector<pair<int, time_point<steady_clock>>> stopwatchv;
        
        // MEMBER FUNCTIONS
        
        int find_stopwatch(const int key);
        
        int find_stopwatch(const int key, const size_t begin, const size_t end);
    public:
        // CONSTRUCTORS
        
        system_loader(command_processor* cp);
    };
    // End Enhancement 1-1
}

#endif /* system_loader_h */
