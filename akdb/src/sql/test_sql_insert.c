#include <assert.h>
#include "sql_insert.h"

int main() {
    const char* table = "studenti";
    const char* columns = "ime,prezime";
    const char* values = "'Ivan','Horvat'";

    int result = sql_insert(table, columns, values);
    assert(result == 0);

    return 0;
}
