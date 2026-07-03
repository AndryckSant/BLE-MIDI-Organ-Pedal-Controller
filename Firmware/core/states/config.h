#ifndef CONFIG_STATE_H
#define CONFIG_STATE_H

void config_mode_enter();
void config_mode_run();
void config_mode_exit();
bool config_mode_should_exit();
bool config_mode_is_active();

#endif // CONFIG_STATE_H