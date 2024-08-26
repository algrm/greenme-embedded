#ifndef MMA8453_H
#define MMA8453_H

#include "StdTypes.h"

bool MMA8453Cfg( void );														// Configures the accelerometer ( returns true when done )
bool MMA8453Task( void );														// sends command to the accelerometer and detect orientation ( returns true on orientation updates )
void MMA8453Orientation( float *Gx, float *Gy, float *Gz );						// returns current sensor filtered forces along each axis ( units : G )

#endif // MMA8453_H

