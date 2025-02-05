//
//  properties.h
//  SimpleScript
//
//  Created by Corey Ferguson on 2/4/25.
//

#ifndef properties_h
#define properties_h

#include <iostream>

namespace ss {
    // NON-MEMBER FUNCTIONS

    std::string environment();

    std::string extension();

    std::string library();

    std::string library_prefix();

    std::string logs();

    std::string main();

    std::string path_separator();

    std::string separator();

    size_t      tab_length();

    std::string temporary();
}

#endif /* properties_h */
