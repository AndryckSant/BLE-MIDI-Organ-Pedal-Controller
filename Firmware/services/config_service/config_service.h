#ifndef CONFIG_SERVICE_H
#define CONFIG_SERVICE_H

#include "../../models/system_config.h"

void config_service_init();
SystemConfig &config_service_get_mutable();
const SystemConfig &config_service_get();
void config_service_load();
void config_service_save();
void config_service_factory_reset();

#endif // CONFIG_SERVICE_H