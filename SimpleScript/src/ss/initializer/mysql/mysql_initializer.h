//
//  mysql_initializer.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef mysql_initializer_h
#define mysql_initializer_h

#include "command_processor.h"
#include "mysql.h"

namespace ss {
    //  NON-MEMBER FUNCTIONS

    void init_mysql();

    void deinit_mysql();

    void set_mysql(command_processor* cp);
}

#endif /* mysql_initializer_h */
