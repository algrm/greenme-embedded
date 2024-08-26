#ifndef EOLTEST_H
#define EOLTEST_H

#include "Main.h"

void EndOfLineTestSetup( void );
void EOLTestRequest( u8 *Buf );
void EndOfLineTestTask( CubeData_t *Dat, u16 *Octaves );

#endif // EOLTEST_H
