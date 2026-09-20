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
"*{box-sizing:border-box}"
"body{font-family:Arial,sans-serif;background:#eef1f3;color:#17212b;"
"margin:0;line-height:1.45}"
"header{background:#172b3a;color:#fff;padding:28px 20px;"
"border-bottom:4px solid #e3a93b}"
"header .header-inner{max-width:1100px;margin:0 auto}"
"main{max-width:1100px;margin:0 auto;padding:20px 16px 96px}"
"section{background:#fff;border:1px solid #d6dde2;border-radius:8px;"
"padding:20px;margin-bottom:18px;box-shadow:0 3px 12px rgba(23,43,58,.06)}"
"h1,h2,h3{margin:0 0 8px 0;line-height:1.2}"
"h1{font-size:clamp(24px,4vw,34px)}h2{font-size:21px;color:#172b3a}"
".subtitle{color:#d9e4eb;margin:0}"
".section-heading{display:flex;justify-content:space-between;gap:16px;"
"align-items:flex-start;margin-bottom:16px}.section-heading p{margin:0;"
"color:#61717c;font-size:13px}.status-grid{display:grid;"
"grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:10px}"
".status-item{background:#f6f8f9;border-left:3px solid #e3a93b;"
"padding:11px 13px}.status-item strong{display:block;font-size:12px;"
"text-transform:uppercase;letter-spacing:.04em;color:#61717c}"
".status-item span{display:block;margin-top:2px;font-weight:700}"
"label{display:block;font-size:13px;font-weight:700;margin:0 0 5px}"
".field-help{display:block;color:#71808a;font-size:11px;font-weight:400;"
"margin-top:3px}input{width:100%;padding:10px;border:1px solid #bdc8cf;"
"border-radius:5px;font:inherit;color:#17212b;background:#fff}"
"input:focus{outline:3px solid rgba(227,169,59,.3);border-color:#bd7f12}"
"fieldset{border:1px solid #dce2e6;border-radius:6px;margin:0 0 14px;"
"padding:15px}legend{padding:0 8px;font-weight:700;color:#2e5369}"
".device{border:1px solid #dce2e6;border-radius:6px;margin:0 0 14px;"
"padding:0;overflow:hidden}.device:last-child{margin-bottom:0}"
".device-header{display:flex;justify-content:space-between;align-items:center;"
"gap:12px;padding:13px 15px;background:#f3f6f7;border-bottom:1px solid #dce2e6}"
".device-header strong{color:#2e5369}.device-header label{margin:0}"
".device-body{padding:15px}.field-group{border-left:3px solid #d9e3e7;"
"padding-left:12px;margin-bottom:16px}.field-group:last-child{margin-bottom:0}"
".field-group h3{color:#61717c;font-size:12px;text-transform:uppercase;"
"letter-spacing:.05em;margin-bottom:10px}.grid{display:grid;"
"grid-template-columns:repeat(auto-fit,minmax(175px,1fr));gap:14px}"
".check{display:flex;align-items:center;gap:8px;min-height:42px}"
".check input{width:auto;accent-color:#2e718b}.check label{margin:0}"
".actions{display:flex;gap:12px;flex-wrap:wrap;align-items:center}"
"button{border:0;border-radius:5px;padding:12px 18px;"
"font:700 14px Arial,sans-serif;cursor:pointer;transition:filter .15s}"
"button:hover{filter:brightness(1.08)}button:focus{outline:3px solid "
"rgba(227,169,59,.45);outline-offset:2px}button.primary{background:#1f647d;"
"color:#fff}button.secondary{background:#a9362b;color:#fff}"
".action-bar{position:sticky;bottom:12px;z-index:2;display:flex;"
"justify-content:space-between;gap:12px;align-items:center;background:#172b3a;"
"color:#fff;padding:12px 14px;border-radius:6px;box-shadow:0 4px 18px "
"rgba(23,43,58,.25)}.action-bar .small{color:#d9e4eb}"
".note{background:#f8f4e8;border:1px solid #ead8a5;border-radius:6px;"
"padding:14px;margin-bottom:18px}.small{font-size:12px;color:#61717c}"
"@media(max-width:520px){main{padding:12px 10px 88px}section{padding:15px}"
".section-heading{display:block}.section-heading p{margin-top:5px}.action-bar{"
"display:block}.action-bar button{width:100%}.action-bar .small{display:block;"
"margin-top:8px;text-align:center}}"
"</style>"
"</head>"
"<body>"
"<header>"
"<div class='header-inner'>"
"<h1>BLE MIDI Controller</h1>"
"<p class='subtitle'>Portal de configuração local</p>"
"</div>"
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
html += "<div class='status-grid'>";
html += "<div class='status-item'><strong>Rede Wi-Fi</strong><span>";
html += kApSsid;
html += "</span></div>";
html += "<div class='status-item'><strong>Endereço</strong><span>";
html += WiFi.softAPIP().toString();
html += "</span></div>";
html += "<div class='status-item'><strong>Status</strong><span>";
html += statusMessage.length() > 0U ? statusMessage : "Pronto para configurar";
html += "</span></div></div>";
html += "</section>";

//=========================================================================
// Pedals
//=========================================================================

html += "<form method='post' action='/save'>";

html += "<section>";
html += "<div class='section-heading'><div><h2>Pedais</h2>";
html += "<p>Mapeie as entradas analogicas e a mensagem MIDI de cada pedal.</p>";
html += "</div></div>";

for (uint8_t index = 0U;
     index < kPedalCount;
     ++index)
{
    const PedalModel &pedal =
        config.pedals[index];

    html += "<fieldset class='device'>";

    html += "<div class='device-header'><strong>Pedal ";
    html += String(index + 1U);
    html += "</strong><label class='check'>";
    html += "<input type='checkbox' name='pedal";
    html += String(index);
    html += "_enabled'";
    html += htmlChecked(pedal.enabled);
    html += "> Ativado</label></div>";

    html += "<div class='device-body'>";

    html += "<div class='field-group'><h3>Entrada analogica</h3>";

    html += "<div class='grid'>";

    html += "<div>";
    html += "<label>Canal ADS<span class='field-help'>Entrada do conversor analogico</span></label>";
    html += "<input type='number' min='0' max='3' name='pedal";
    html += String(index);
    html += "_adc' value='";
    html += String(pedal.adc_channel);
    html += "'>";
    html += "</div>";

    html += "</div></div>";

    html += "<div class='field-group'><h3>Mensagem MIDI</h3>";
    html += "<div class='grid'>";

    html += "<div>";
    html += "<label>Canal MIDI<span class='field-help'>Valores de 1 a 16</span></label>";
    html += "<input type='number' min='1' max='16' name='pedal";
    html += String(index);
    html += "_midi_channel' value='";
    html += String(pedal.midi_channel);
    html += "'>";
    html += "</div>";

    html += "<div>";
    html += "<label>CC MIDI<span class='field-help'>Control Change, de 0 a 127</span></label>";
    html += "<input type='number' min='0' max='127' name='pedal";
    html += String(index);
    html += "_midi_cc' value='";
    html += String(pedal.midi_cc);
    html += "'>";
    html += "</div>";

    html += "</div></div>";

    html += "<div class='field-group'><h3>Calibracao</h3>";
    html += "<div class='grid'>";

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
    html += "</div></div></fieldset>";
}

html += "</section>";

//=========================================================================
// Buttons
//=========================================================================

html += "<section>";
html += "<div class='section-heading'><div><h2>Botões</h2>";
html += "<p>Configure os pinos digitais e os comandos enviados ao dispositivo MIDI.</p>";
html += "</div></div>";

for (uint8_t index = 0U;
     index < kButtonCount;
     ++index)
{
    const ButtonModel &button =
        config.buttons[index];

    html += "<fieldset class='device'>";

    html += "<div class='device-header'><strong>Botão ";
    html += String(index + 1U);
    html += "</strong><label class='check'>";
    html += "<input type='checkbox' name='button";
    html += String(index);
    html += "_enabled'";
    html += htmlChecked(button.enabled);
    html += "> Ativado</label></div>";

    html += "<div class='device-body'>";

    html += "<div class='field-group'><h3>Entrada digital</h3>";

    html += "<div class='grid'>";

    html += "<div>";
    html += "<label>GPIO<span class='field-help'>Pino digital do ESP32</span></label>";
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

    html += "</div></div>";

    html += "<div class='field-group'><h3>Mensagem MIDI</h3>";
    html += "<div class='grid'>";

    html += "<div>";
    html += "<label>Canal MIDI<span class='field-help'>Valores de 1 a 16</span></label>";
    html += "<input type='number' min='1' max='16' name='button";
    html += String(index);
    html += "_midi_channel' value='";
    html += String(button.midi_channel);
    html += "'>";
    html += "</div>";

    html += "<div>";
    html += "<label>CC MIDI<span class='field-help'>Control Change, de 0 a 127</span></label>";
    html += "<input type='number' min='0' max='127' name='button";
    html += String(index);
    html += "_midi_cc' value='";
    html += String(button.midi_cc);
    html += "'>";
    html += "</div>";

    html += "</div>";
    html += "</div></div></fieldset>";
}

html += "</section>";

//=========================================================================
// Actions
//=========================================================================

html += "<div class='action-bar'>";
html += "<span class='small'>As alterações entram em vigor após salvar.</span>";

html += "<button class='primary' type='submit'>";
html += "Salvar configurações";
html += "</button>";

html += "</div>";

html += "</form>";

html += "<form method='post' action='/factory'>";

html += "<section>";
html += "<div class='actions'>";

html += "<button class='secondary' type='submit' "
    "onclick=\"return confirm('Restaurar todas as configurações de fábrica?')\">";
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
