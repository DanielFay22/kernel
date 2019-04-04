//
// Created by Daniel on 4/3/2019.
//

#include <string.h>


size_t
strlen(const char *str)
{
        size_t retval;
        for(retval = 0; *str != '\0'; str++) retval++;
        return retval;
}
