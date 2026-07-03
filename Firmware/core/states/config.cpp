#include "config.h"

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

#include "../../services/config_service/config_service.h"

namespace {

constexpr char kApSsid[] = "BLE MIDI Controller Setup";
constexpr char kHtmlHeader[] = "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>BLE MIDI Controller</title><style>body{font-family:Arial,sans-serif;background:#f4f4ef;color:#1f1f1f;margin:0;padding:0}header{background:#1f2d3a;color:#fff;padding:20px 16px}main{max-width:1100px;margin:0 auto;padding:16px}section{background:#fff;border:1px solid #d7d7d0;border-radius:12px;padding:16px;margin-bottom:16px;box-shadow:0 4px 18px rgba(0,0,0,.05)}h1,h2,h3{margin:0 0 12px 0}label{display:block;font-size:13px;margin:8px 0 4px}input,select{width:100%;padding:10px;border:1px solid #c8c8c0;border-radius:8px;box-sizing:border-box}fieldset{border:1px solid #ddd;border-radius:10px;margin:0 0 16px 0;padding:12px}legend{padding:0 8px;font-weight:700} .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px}.actions{display:flex;gap:12px;flex-wrap:wrap}button{border:0;border-radius:10px;padding:12px 16px;font-weight:700;cursor:pointer}button.primary{background:#17324d;color:#fff}button.secondary{background:#c93f2d;color:#fff}.note{background:#eef5ff;border:1px solid #cfdffe;border-radius:10px;padding:10px 12px;margin-bottom:16px}.small{font-size:12px;color:#555}</style></head><body><header><h1>BLE MIDI Controller</h1><div class='small'>Modo AP de configuração</div></header><main>";
constexpr char kHtmlFooter[] = "</main></body></html>";

WebServer server(80);
bool active = false;
bool exitRequested = false;
String statusMessage;

String checked(bool value) {
    return value ? " checked" : "";
}

int parsedInt(const String &value, int fallback) {
    if (value.length() == 0) {
        return fallback;
    }

    return value.toInt();
}

bool parsedBool(const String &value) {
    return value == "1" || value == "on" || value == "true";
}

uint8_t clampU8(int value, uint8_t minimum, uint8_t maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return static_cast<uint8_t>(value);
}

String buildPage() {
    const SystemConfig &config = config_service_get();
    String html;
    html.reserve(20000);
    html += kHtmlHeader;
    html += "<section class='note'><strong>SSID:</strong> ";
    html += kApSsid;
    html += "<br><strong>IP:</strong> ";
    html += WiFi.softAPIP().toString();
    html += "<br><strong>Status:</strong> ";
    html += statusMessage.length() > 0 ? statusMessage : String("Pronto para configurar");
    html += "</section>";
    html += "<form method='post' action='/save'>";

    html += "<section><h2>Pedais</h2>";
    for (uint8_t index = 0; index < 2; ++index) {
        const PedalModel &pedal = config.pedals[index];
        html += "<fieldset><legend>Pedal ";
        html += String(index + 1);
        html += "</legend><div class='grid'>";
        html += "<div><label><input type='checkbox' name='pedal" + String(index) + "_enabled'" + checked(pedal.enabled) + "> Ativado</label></div>";
        html += "<div><label>Canal ADS</label><input type='number' min='0' max='3' name='pedal" + String(index) + "_adc' value='" + String(pedal.adc_channel) + "'></div>";
        html += "<div><label>Canal MIDI</label><input type='number' min='1' max='16' name='pedal" + String(index) + "_midi_channel' value='" + String(pedal.midi_channel) + "'></div>";
        html += "<div><label>CC MIDI</label><input type='number' min='0' max='127' name='pedal" + String(index) + "_midi_cc' value='" + String(pedal.midi_cc) + "'></div>";
        html += "<div><label>Calibração mín.</label><input type='number' step='0.1' name='pedal" + String(index) + "_cal_min' value='" + String(pedal.calibration_min, 1) + "'></div>";
        html += "<div><label>Calibração máx.</label><input type='number' step='0.1' name='pedal" + String(index) + "_cal_max' value='" + String(pedal.calibration_max, 1) + "'></div>";
        html += "</div></fieldset>";
    }
    html += "</section>";

    html += "<section><h2>Botões</h2>";
    for (uint8_t index = 0; index < 8; ++index) {
        const ButtonModel &button = config.buttons[index];
        html += "<fieldset><legend>Botão ";
        html += String(index + 1);
        html += "</legend><div class='grid'>";
        html += "<div><label><input type='checkbox' name='button" + String(index) + "_enabled'" + checked(button.enabled) + "> Ativado</label></div>";
        html += "<div><label>GPIO</label><input type='number' min='0' max='39' name='button" + String(index) + "_gpio' value='" + String(button.gpio_pin) + "'></div>";
        html += "<div><label><input type='checkbox' name='button" + String(index) + "_inverted'" + checked(button.inverted) + "> Lógica invertida</label></div>";
        html += "<div><label>Canal MIDI</label><input type='number' min='1' max='16' name='button" + String(index) + "_midi_channel' value='" + String(button.midi_channel) + "'></div>";
        html += "<div><label>CC MIDI</label><input type='number' min='0' max='127' name='button" + String(index) + "_midi_cc' value='" + String(button.midi_cc) + "'></div>";
        html += "</div></fieldset>";
    }
    html += "</section>";

    html += "<section><div class='actions'><button class='primary' type='submit'>Salvar configurações</button></div></section></form>";
    html += "<form method='post' action='/factory'><section><div class='actions'><button class='secondary' type='submit'>Restaurar padrão de fábrica</button></div></section></form>";
    html += kHtmlFooter;
    return html;
}

void applyFormToConfig() {
    SystemConfig &config = config_service_get_mutable();

    for (uint8_t index = 0; index < 2; ++index) {
        PedalModel &pedal = config.pedals[index];
        const String prefix = "pedal" + String(index) + "_";
        pedal.enabled = parsedBool(server.arg(prefix + "enabled"));
        pedal.adc_channel = clampU8(parsedInt(server.arg(prefix + "adc"), pedal.adc_channel), 0, 3);
        pedal.midi_channel = clampU8(parsedInt(server.arg(prefix + "midi_channel"), pedal.midi_channel), 1, 16);
        pedal.midi_cc = clampU8(parsedInt(server.arg(prefix + "midi_cc"), pedal.midi_cc), 0, 127);
        pedal.calibration_min = static_cast<float>(server.arg(prefix + "cal_min").toFloat());
        pedal.calibration_max = static_cast<float>(server.arg(prefix + "cal_max").toFloat());
        pedal.calibration_initialized = true;
    }

    for (uint8_t index = 0; index < 8; ++index) {
        ButtonModel &button = config.buttons[index];
        const String prefix = "button" + String(index) + "_";
        button.enabled = parsedBool(server.arg(prefix + "enabled"));
        button.gpio_pin = clampU8(parsedInt(server.arg(prefix + "gpio"), button.gpio_pin), 0, 39);
        button.inverted = parsedBool(server.arg(prefix + "inverted"));
        button.midi_channel = clampU8(parsedInt(server.arg(prefix + "midi_channel"), button.midi_channel), 1, 16);
        button.midi_cc = clampU8(parsedInt(server.arg(prefix + "midi_cc"), button.midi_cc), 0, 127);
    }
}

void handleRoot() {
    server.send(200, "text/html; charset=utf-8", buildPage());
}

void handleSave() {
    applyFormToConfig();
    config_service_save();
    statusMessage = "Configuracoes salvas com sucesso";
    server.send(200, "text/html; charset=utf-8", buildPage());
    exitRequested = true;
}

void handleFactory() {
    config_service_factory_reset();
    statusMessage = "Padrao de fabrica restaurado";
    server.send(200, "text/html; charset=utf-8", buildPage());
    exitRequested = true;
}

void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

} // namespace

void config_mode_enter() {
    if (active) {
        return;
    }

    statusMessage = "Modo de configuracao ativo";
    exitRequested = false;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(kApSsid);
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/factory", HTTP_POST, handleFactory);
    server.onNotFound(handleNotFound);
    server.begin();
    active = true;
    Serial.print("AP ativo em ");
    Serial.println(WiFi.softAPIP());
}

void config_mode_run() {
    if (!active) {
        return;
    }

    server.handleClient();
}

void config_mode_exit() {
    if (!active) {
        return;
    }

    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    active = false;
    exitRequested = false;
    statusMessage = "";
}

bool config_mode_should_exit() {
    return exitRequested;
}

bool config_mode_is_active() {
    return active;
}