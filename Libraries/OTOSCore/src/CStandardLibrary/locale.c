
#include "CStandardLibrary/locale.h"
#include "CStandardLibrary/stdlib.h"

const struct lconv cStandardLibrary_GlOBAL_VARAIABLE_Lconv_Table = {};

char * setlocale( int category, const char * locale) {
    return NULL;
}

struct lconv * localeconv() {
    return (struct lconv *)&cStandardLibrary_GlOBAL_VARAIABLE_Lconv_Table;
}
