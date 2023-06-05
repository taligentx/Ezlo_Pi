#ifndef __DIGITAL_IO_H__
#define __DIGITAL_IO_H__

#include "ezlopi_actions.h"
#include "driver/gpio.h"


int device_0001_digitalOut_generic(e_ezlopi_actions_t action, s_ezlopi_device_properties_t *properties, void *arg, void *user_arg);

#endif // __DIGITAL_IO_H__
