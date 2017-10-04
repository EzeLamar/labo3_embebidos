#ifndef ADC_H
#define ADC_H
typedef struct {
    int canal;
    int (*callback)(int x);
  } adc_cfg;

//inicializa driver de ADC y acepta como parametro una estructura e tipo adc_cfg.
int adc_init(adc_cfg *cfg);

//ejecuta las funciones de callback corespondientes solo cuando se obtuvo el valor.
void adc_loop();
#endif
