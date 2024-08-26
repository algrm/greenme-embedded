#ifndef I2C_STD_SENSORS_H
#define I2C_STD_SENSORS_H

typedef enum { UP, TILTED_FRONT, TILTED_BACK, TILTED_RIGHT, TILTED_LEFT, UPSIDE_DOWN, NB_CUBE_POS }CubeOrientation_t;

void I2CStdSensorsSetup( void );
CubeOrientation_t CubeOrientation( void );										// Cube orientation : 6 positions
u16 eCO2_ppm( void );															//(ppm) CO2 estimate
u16 TVoc_ppb( void );															//(ppb) VOC estimate
u16 Temperaturex100( void );													//(°C) temperature
u16 Hygrometryx100( void );														//(%) Hygrometry
u16 Lux( void );																//(Lux) Luxmeter value

#endif // I2C_STD_SENSORS_H
