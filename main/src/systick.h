#ifndef SYSTICK_H_
#define SYSTICK_H_

#define ARDUINO_SIM 0

#ifdef __cplusplus
extern "C" {
#endif

uint32_t get_ticks(void);

#ifdef __cplusplus
}
#endif

void Sleep(int ms);

/* this function is required by the modbus library */
uint32_t millis();


#endif /* SYSTICK_H_ */
