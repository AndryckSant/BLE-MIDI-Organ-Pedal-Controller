/**

* @file    config.cpp
* @author  Andryck Santiago
* @brief   Configuration mode state implementation.
*
* @details
* Provides the Wi-Fi Access Point and WebUI used to configure
* the BLE MIDI controller.
*
* Configuration changes are stored through the configuration service.
* After saving or restoring factory defaults, the configuration state
* requests a transition back to normal operation.
  */

#include "config.h"

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

#include "../../board_pins.h"
#include "config_page.h"
#include "../../models/system_config.h"
#include "../../services/config_service/config_service.h"

namespace
{

//=============================================================================
// Private Constants
//=============================================================================

constexpr char kApSsid[] = "BLE MIDI Controller Setup";

constexpr uint8_t kPedalCount = 2U;
constexpr uint8_t kButtonCount = BoardPins::kMidiButtonCount;

constexpr uint8_t kAdcChannelMinimum = 0U;
constexpr uint8_t kAdcChannelMaximum = 3U;

constexpr uint8_t kMidiChannelMinimum = 1U;
constexpr uint8_t kMidiChannelMaximum = 16U;

constexpr uint8_t kMidiCcMinimum = 0U;
constexpr uint8_t kMidiCcMaximum = 127U;

constexpr uint8_t kGpioMinimum = 0U;
constexpr uint8_t kGpioMaximum = 39U;

//=============================================================================
// Private Variables
//=============================================================================

WebServer server(80);

bool active = false;
bool exitRequested = false;

String statusMessage;

//=============================================================================
// Private Functions
//=============================================================================

String htmlChecked(bool value)
{
return value ? " checked" : "";
}

String extractHtmlTemplate(
String &html,
const char *templateId)
{
const String openingTag =
    "<template id='" + String(templateId) + "'>";
const String closingTag =
    "</template>";
const int start =
    html.indexOf(openingTag);

if (start < 0)
{
    return String();
}

const unsigned int contentStart =
    static_cast<unsigned int>(start) + openingTag.length();
const int end =
    html.indexOf(closingTag, contentStart);

if (end < 0)
{
    return String();
}

const String fragment =
    html.substring(
        contentStart,
        static_cast<unsigned int>(end));
const unsigned int removalLength =
    static_cast<unsigned int>(end) -
    static_cast<unsigned int>(start) +
    closingTag.length();

html.remove(
    static_cast<unsigned int>(start),
    removalLength);

return fragment;
}

String renderPedalCard(
const String &cardTemplate,
uint8_t index,
const PedalModel &pedal)
{
String card = cardTemplate;
card.replace("{{INDEX}}", String(index));
card.replace("{{NUMBER}}", String(index + 1U));
card.replace("{{ENABLED}}", htmlChecked(pedal.enabled));
card.replace("{{ADC}}", String(pedal.adc_channel));
card.replace("{{MIDI_CHANNEL}}", String(pedal.midi_channel));
card.replace("{{MIDI_CC}}", String(pedal.midi_cc));
card.replace("{{CAL_MIN}}", String(pedal.calibration.minimum, 1));
card.replace("{{CAL_MAX}}", String(pedal.calibration.maximum, 1));
card.replace("{{AUTO_RANGE}}", htmlChecked(pedal.calibration.auto_range));
return card;
}

String renderButtonCard(
const String &cardTemplate,
uint8_t index,
const ButtonModel &button)
{
String card = cardTemplate;
card.replace("{{INDEX}}", String(index));
card.replace("{{NUMBER}}", String(index + 1U));
card.replace("{{ENABLED}}", htmlChecked(button.enabled));
card.replace("{{GPIO}}", String(button.gpio_pin));
card.replace("{{INVERTED}}", htmlChecked(button.inverted));
card.replace("{{MIDI_CHANNEL}}", String(button.midi_channel));
card.replace("{{MIDI_CC}}", String(button.midi_cc));
card.replace(
    "{{MODE_MOMENTARY}}",
    button.latch ? "" : " selected");
card.replace(
    "{{MODE_LATCH}}",
    button.latch ? " selected" : "");
return card;
}

int parseIntOrDefault(
const String &value,
int fallback)
{
if (value.length() == 0U)
{
return fallback;
}


return value.toInt();


}

bool parseBool(const String &value)
{
return value == "1" ||
value == "on" ||
value == "true";
}

uint8_t clampU8(
int value,
uint8_t minimum,
uint8_t maximum)
{
if (value < minimum)
{
return minimum;
}


if (value > maximum)
{
    return maximum;
}

return static_cast<uint8_t>(value);


}

float parseFloatOrDefault(
const String &value,
float fallback)
{
if (value.length() == 0U)
{
return fallback;
}


return value.toFloat();


}

//=============================================================================
// Page Generation
//=============================================================================

bool buildPage(String &html)
{
const SystemConfig &config =
    *config_service_get();

html = String(FPSTR(kConfigurationPageTemplate));
html.reserve(20000);

const String pedalTemplate =
    extractHtmlTemplate(html, "pedal-card");
const String buttonTemplate =
    extractHtmlTemplate(html, "button-card");

if (pedalTemplate.length() == 0U ||
    buttonTemplate.length() == 0U)
{
    return false;
}

String pedalCards;
pedalCards.reserve(3000);

for (uint8_t index = 0U;
     index < kPedalCount;
     ++index)
{
    pedalCards += renderPedalCard(
        pedalTemplate,
        index,
        config.pedals[index]);
}

String buttonCards;
buttonCards.reserve(8000);

for (uint8_t index = 0U;
     index < kButtonCount;
     ++index)
{
    buttonCards += renderButtonCard(
        buttonTemplate,
        index,
        config.buttons[index]);
}

html.replace("{{WIFI_SSID}}", kApSsid);
html.replace("{{AP_IP}}", WiFi.softAPIP().toString());
html.replace(
    "{{STATUS}}",
    statusMessage.length() > 0U
        ? statusMessage
        : "Pronto para configurar");
html.replace("{{PEDAL_CARDS}}", pedalCards);
html.replace("{{BUTTON_CARDS}}", buttonCards);

return true;
}

void sendConfigurationPage(int statusCode)
{
String html;

if (!buildPage(html))
{
    server.send(
        500,
        "text/plain; charset=utf-8",
        "Failed to load configuration page template.");
    return;
}

server.send(
    statusCode,
    "text/html; charset=utf-8",
    html);
}

//=============================================================================
// Configuration Update
//=============================================================================

void applyFormToConfig()
{
SystemConfig &config =
config_service_get_mutable();


//=========================================================================
// Pedals
//=========================================================================

for (uint8_t index = 0U;
     index < kPedalCount;
     ++index)
{
    PedalModel &pedal =
        config.pedals[index];

    const String prefix =
        "pedal" + String(index) + "_";

    pedal.enabled =
        parseBool(
            server.arg(
                prefix + "enabled"));

    pedal.adc_channel =
        clampU8(
            parseIntOrDefault(
                server.arg(prefix + "adc"),
                pedal.adc_channel),
            kAdcChannelMinimum,
            kAdcChannelMaximum);

    pedal.midi_channel =
        clampU8(
            parseIntOrDefault(
                server.arg(prefix + "midi_channel"),
                pedal.midi_channel),
            kMidiChannelMinimum,
            kMidiChannelMaximum);

    pedal.midi_cc =
        clampU8(
            parseIntOrDefault(
                server.arg(prefix + "midi_cc"),
                pedal.midi_cc),
            kMidiCcMinimum,
            kMidiCcMaximum);

    pedal.calibration.minimum =
        parseFloatOrDefault(
            server.arg(prefix + "cal_min"),
            pedal.calibration.minimum);

    pedal.calibration.maximum =
        parseFloatOrDefault(
            server.arg(prefix + "cal_max"),
            pedal.calibration.maximum);

    pedal.calibration.auto_range =
        parseBool(
            server.arg(prefix + "auto_range"));

    if (pedal.calibration.minimum < 0.0f)
    {
        pedal.calibration.minimum = 0.0f;
    }

    if (pedal.calibration.minimum > 32767.0f)
    {
        pedal.calibration.minimum = 32767.0f;
    }

    if (pedal.calibration.maximum < 0.0f)
    {
        pedal.calibration.maximum = 0.0f;
    }

    if (pedal.calibration.maximum > 32767.0f)
    {
        pedal.calibration.maximum = 32767.0f;
    }

    pedal.calibration.initialized =
        pedal.calibration.maximum >=
        pedal.calibration.minimum;
}

//=========================================================================
// Buttons
//=========================================================================

for (uint8_t index = 0U;
     index < kButtonCount;
     ++index)
{
    ButtonModel &button =
        config.buttons[index];

    const String prefix =
        "button" + String(index) + "_";

    button.enabled =
        parseBool(
            server.arg(
                prefix + "enabled"));

    button.gpio_pin =
        clampU8(
            parseIntOrDefault(
                server.arg(prefix + "gpio"),
                button.gpio_pin),
            kGpioMinimum,
            kGpioMaximum);

    button.inverted =
        parseBool(
            server.arg(
                prefix + "inverted"));

    button.midi_channel =
        clampU8(
            parseIntOrDefault(
                server.arg(prefix + "midi_channel"),
                button.midi_channel),
            kMidiChannelMinimum,
            kMidiChannelMaximum);

    button.midi_cc =
        clampU8(
            parseIntOrDefault(
                server.arg(prefix + "midi_cc"),
                button.midi_cc),
            kMidiCcMinimum,
            kMidiCcMaximum);

    const String mode = server.arg(prefix + "mode");
    if (mode == "momentary")
    {
        button.latch = false;
    }
    else if (mode == "latch")
    {
        button.latch = true;
    }
}


}

//=============================================================================
// HTTP Handlers
//=============================================================================

void handleRoot()
{
sendConfigurationPage(200);
}

void handleSave()
{
applyFormToConfig();


if (config_service_save())
{
    statusMessage =
        "Configurações salvas com sucesso.";

    sendConfigurationPage(200);

    exitRequested = true;
}
else
{
    statusMessage =
        "Erro ao salvar configurações.";

    sendConfigurationPage(500);
}


}

void handleFactory()
{
if (config_service_factory_reset())
{
statusMessage =
"Padrão de fábrica restaurado.";


    sendConfigurationPage(200);

    exitRequested = true;
}
else
{
    statusMessage =
        "Erro ao restaurar configuração.";

    sendConfigurationPage(500);
}


}

void handleNotFound()
{
server.send(
404,
"text/plain",
"Not found");
}

} // namespace

//=============================================================================
// Public Functions
//=============================================================================

void config_mode_enter(const StateActions &actions)
{
if (active)
{
return;
}


actions.set_led(true);

exitRequested = false;

WiFi.mode(WIFI_AP);

if (!WiFi.softAP(kApSsid))
{
    actions.log_message(
        "Failed to start configuration AP.");

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    active = false;

    if (actions.display_show_error != nullptr)
    {
        actions.display_show_error("AP failed");
    }

    delay(5000UL);
    ESP.restart();
    return;
}

server.on(
    "/",
    HTTP_GET,
    handleRoot);

server.on(
    "/save",
    HTTP_POST,
    handleSave);

server.on(
    "/factory",
    HTTP_POST,
    handleFactory);

server.onNotFound(
    handleNotFound);

server.begin();

active = true;

const String apIp = WiFi.softAPIP().toString();

actions.log_message("Configuration AP active. Connect to the AP and open:");
actions.log_message(apIp.c_str());
actions.display_show_config(apIp.c_str());

}

void config_mode_run()
{
if (!active)
{
return;
}

server.handleClient();

}

void config_mode_exit()
{
if (!active)
{
return;
}

server.stop();

WiFi.softAPdisconnect(true);
WiFi.mode(WIFI_OFF);

active = false;
exitRequested = false;
statusMessage = "";

}

bool config_mode_should_exit()
{
return exitRequested;
}

bool config_mode_is_active()
{
return active;
}
