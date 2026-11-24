#ifndef TEMP_READING_H
#define TEMP_READING_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool onewire_reset(void);
void READ_TEMP_C(void);

#ifdef __cplusplus
}
#endif

#endif // TEMP_READING_H