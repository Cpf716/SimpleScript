//
//  mysql_loader.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#ifndef mysql_loader_h
#define mysql_loader_h

#include "command_processor.h"
#include "mysql.h"

namespace ss {
    //  NON-MEMBER FUNCTIONS

    void load_mysql(command_processor* cp);

    void unload_mysql();
}

#endif /* mysql_loader_h */
