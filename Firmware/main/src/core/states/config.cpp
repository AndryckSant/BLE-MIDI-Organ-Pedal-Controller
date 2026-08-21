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

#include "../../models/system_config.h"
#include "../../services/config_service/config_service.h"

namespace
{

//=============================================================================
// Private Constants
//=============================================================================

constexpr char kApSsid[] = "BLE MIDI Controller Setup";

constexpr uint8_t kPedalCount = 2U;
constexpr uint8_t kButtonCount = 8U;

constexpr uint8_t kAdcChannelMinimum = 0U;
constexpr uint8_t kAdcChannelMaximum = 3U;

constexpr uint8_t kMidiChannelMinimum = 1U;
constexpr uint8_t kMidiChannelMaximum = 16U;

constexpr uint8_t kMidiCcMinimum = 0U;
constexpr uint8_t kMidiCcMaximum = 127U;

constexpr uint8_t kGpioMinimum = 0U;
constexpr uint8_t kGpioMaximum = 39U;

//=============================================================================
// Private HTML
//=============================================================================

constexpr char kHtmlHeader[] =
"<!doctype html>"
"<html>"
"<head>"
"<meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<title>BLE MIDI Controller</title>"
"<style>"
"body{font-family:Arial,sans-serif;background:#f4f4ef;color:#1f1f1f;"
"margin:0;padding:0}"
"header{background:#1f2d3a;color:#fff;padding:20px 16px}"
"main{max-width:1100px;margin:0 auto;padding:16px}"
"section{background:#fff;border:1px solid #d7d7d0;border-radius:12px;"
"padding:16px;margin-bottom:16px;box-shadow:0 4px 18px rgba(0,0,0,.05)}"
"h1,h2,h3{margin:0 0 12px 0}"
"label{display:block;font-size:13px;margin:8px 0 4px}"
"input,select{width:100%;padding:10px;border:1px solid #c8c8c0;"
"border-radius:8px;box-sizing:border-box}"
"fieldset{border:1px solid #ddd;border-radius:10px;margin:0 0 16px 0;"
"padding:12px}"
"legend{padding:0 8px;font-weight:700}"
".grid{display:grid;grid-template-columns:repeat(auto-fit,"
"minmax(180px,1fr));gap:12px}"
".actions{display:flex;gap:12px;flex-wrap:wrap}"
"button{border:0;border-radius:10px;padding:12px 16px;"
"font-weight:700;cursor:pointer}"
"button.primary{background:#17324d;color:#fff}"
"button.secondary{background:#c93f2d;color:#fff}"
".note{background:#eef5ff;border:1px solid #cfdffe;"
"border-radius:10px;padding:10px 12px;margin-bottom:16px}"
".small{font-size:12px;color:#555}"
"</style>"
"</head>"
"<body>"
"<header>"
"<h1>BLE MIDI Controller</h1>"
"<div class='small'>Modo AP de configuração</div>"
"</header>"
"<main>";

constexpr char kHtmlFooter[] =
"</main>"
"</body>"
"</html>";

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

String buildPage()
{
const SystemConfig &config =
*config_service_get();


String html;

html.reserve(20000);

html += kHtmlHeader;

html += "<section class='note'>";

html += "<strong>SSID:</strong> ";
html += kApSsid;

html += "<br><strong>IP:</strong> ";
html += WiFi.softAPIP().toString();

html += "<br><strong>Status:</strong> ";

if (statusMessage.length() > 0U)
{
    html += statusMessage;
}
else
{
    html += "Pronto para configurar";
}

html += "</section>";

//=========================================================================
// Pedals
//=========================================================================

html += "<form method='post' action='/save'>";

html += "<section>";
html += "<h2>Pedais</h2>";

for (uint8_t index = 0U;
     index < kPedalCount;
     ++index)
{
    const PedalModel &pedal =
        config.pedals[index];

    html += "<fieldset>";

    html += "<legend>Pedal ";
    html += String(index + 1U);
    html += "</legend>";

    html += "<div class='grid'>";

    html += "<div>";
    html += "<label>";
    html += "<input type='checkbox' name='pedal";
    html += String(index);
    html += "_enabled'";
    html += htmlChecked(pedal.enabled);
    html += "> Ativado";
    html += "</label>";
    html += "</div>";

    html += "<div>";
    html += "<label>Canal ADS</label>";
    html += "<input type='number' min='0' max='3' name='pedal";
    html += String(index);
    html += "_adc' value='";
    html += String(pedal.adc_channel);
    html += "'>";
    html += "</div>";

    html += "<div>";
    html += "<label>Canal MIDI</label>";
    html += "<input type='number' min='1' max='16' name='pedal";
    html += String(index);
    html += "_midi_channel' value='";
    html += String(pedal.midi_channel);
    html += "'>";
    html += "</div>";

    html += "<div>";
    html += "<label>CC MIDI</label>";
    html += "<input type='number' min='0' max='127' name='pedal";
    html += String(index);
    html += "_midi_cc' value='";
    html += String(pedal.midi_cc);
    html += "'>";
    html += "</div>";

    html += "<div>";
    html += "<label>Calibração mín.</label>";
    html += "<input type='number' step='0.1' min='0' max='32767' name='pedal";
    html += String(index);
    html += "_cal_min' value='";
    html += String(pedal.calibration.minimum, 1);
    html += "'>";
    html += "</div>";

    html += "<div>";
    html += "<label>Calibração máx.</label>";
    html += "<input type='number' step='0.1' min='0' max='32767' name='pedal";
    html += String(index);
    html += "_cal_max' value='";
    html += String(pedal.calibration.maximum, 1);
    html += "'>";
    html += "</div>";

    html += "</div>";
    html += "</fieldset>";
}

html += "</section>";

//=========================================================================
// Buttons
//=========================================================================

html += "<section>";
html += "<h2>Botões</h2>";

for (uint8_t index = 0U;
     index < kButtonCount;
     ++index)
{
    const ButtonModel &button =
        config.buttons[index];

    html += "<fieldset>";

    html += "<legend>Botão ";
    html += String(index + 1U);
    html += "</legend>";

    html += "<div class='grid'>";

    html += "<div>";
    html += "<label>";
    html += "<input type='checkbox' name='button";
    html += String(index);
    html += "_enabled'";
    html += htmlChecked(button.enabled);
    html += "> Ativado";
    html += "</label>";
    html += "</div>";

    html += "<div>";
    html += "<label>GPIO</label>";
    html += "<input type='number' min='0' max='39' name='button";
    html += String(index);
    html += "_gpio' value='";
    html += String(button.gpio_pin);
    html += "'>";
    html += "</div>";

    html += "<div>";
    html += "<label>";
    html += "<input type='checkbox' name='button";
    html += String(index);
    html += "_inverted'";
    html += htmlChecked(button.inverted);
    html += "> Lógica invertida";
    html += "</label>";
    html += "</div>";

    html += "<div>";
    html += "<label>Canal MIDI</label>";
    html += "<input type='number' min='1' max='16' name='button";
    html += String(index);
    html += "_midi_channel' value='";
    html += String(button.midi_channel);
    html += "'>";
    html += "</div>";

    html += "<div>";
    html += "<label>CC MIDI</label>";
    html += "<input type='number' min='0' max='127' name='button";
    html += String(index);
    html += "_midi_cc' value='";
    html += String(button.midi_cc);
    html += "'>";
    html += "</div>";

    html += "</div>";
    html += "</fieldset>";
}

html += "</section>";

//=========================================================================
// Actions
//=========================================================================

html += "<section>";
html += "<div class='actions'>";

html += "<button class='primary' type='submit'>";
html += "Salvar configurações";
html += "</button>";

html += "</div>";
html += "</section>";

html += "</form>";

html += "<form method='post' action='/factory'>";

html += "<section>";
html += "<div class='actions'>";

html += "<button class='secondary' type='submit'>";
html += "Restaurar padrão de fábrica";
html += "</button>";

html += "</div>";
html += "</section>";

html += "</form>";

html += kHtmlFooter;

return html;


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
}


}

//=============================================================================
// HTTP Handlers
//=============================================================================

void handleRoot()
{
server.send(
200,
"text/html; charset=utf-8",
buildPage());
}

void handleSave()
{
applyFormToConfig();


if (config_service_save())
{
    statusMessage =
        "Configurações salvas com sucesso.";

    server.send(
        200,
        "text/html; charset=utf-8",
        buildPage());

    exitRequested = true;
}
else
{
    statusMessage =
        "Erro ao salvar configurações.";

    server.send(
        500,
        "text/html; charset=utf-8",
        buildPage());
}


}

void handleFactory()
{
if (config_service_factory_reset())
{
statusMessage =
"Padrão de fábrica restaurado.";


    server.send(
        200,
        "text/html; charset=utf-8",
        buildPage());

    exitRequested = true;
}
else
{
    statusMessage =
        "Erro ao restaurar configuração.";

    server.send(
        500,
        "text/html; charset=utf-8",
        buildPage());
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

    active = false;

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

actions.log_message("Configuration AP active at ");
actions.display_show_config(WiFi.softAPIP().toString().c_str());

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
