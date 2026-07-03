#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

void display_show_boot();
void display_show_disconnected();
void display_show_connected(uint8_t value);
void display_show_streaming(uint8_t value);
void display_show_config_mode();
void display_show_error(const char* message);

#endif // DISPLAY_SERVICE_H