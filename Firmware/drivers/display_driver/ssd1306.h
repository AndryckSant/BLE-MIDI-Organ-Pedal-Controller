#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

void ssd1306_init();
void ssd1306_clear();
void ssd1306_display_text(const char* text, int x, int y);
void ssd1306_display_update();

#endif // SSD1306_DRIVER_H