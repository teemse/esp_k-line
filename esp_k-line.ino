#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "K-Line_Adapter";
const char* password = "12345678";

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
String serialData = "";
unsigned long lastUpdate = 0;

// Настройки OLED дисплея
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Буфер для отображения на OLED
String oledLines[8]; // 8 строк по 21 символу (128/6)
int currentLine = 0;

// HTML страница с кнопками команд KWP2000
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>K-Line Адаптер</title>
  <meta charset="UTF-8">
  <style>
    body { font-family: Arial; margin: 20px; }
    .container { max-width: 1000px; margin: 0 auto; }
    .card { border: 1px solid #ccc; border-radius: 5px; padding: 15px; margin-bottom: 15px; }
    #data { 
      height: 300px; 
      overflow-y: scroll; 
      border: 1px solid #ccc; 
      padding: 10px; 
      font-family: monospace;
      background-color: #f5f5f5;
      margin-bottom: 15px;
    }
    .input-group { display: flex; margin-top: 10px; }
    #command { flex-grow: 1; margin-right: 10px; padding: 5px; }
    button { padding: 5px 10px; margin: 2px; cursor: pointer; }
    .cmd-buttons { display: grid; grid-template-columns: repeat(auto-fill, minmax(150px, 1fr)); gap: 5px; margin-bottom: 15px; }
    .cmd-group { margin-bottom: 15px; }
    .cmd-group h3 { margin: 10px 0 5px 0; }
  </style>
  <script>
    function updateData() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("data").innerHTML = this.responseText;
          var elem = document.getElementById('data');
          elem.scrollTop = elem.scrollHeight;
        }
      };
      xhttp.open("GET", "/data", true);
      xhttp.send();
    }
    
    function sendCommand() {
      var command = document.getElementById("command").value;
      if (command) {
        var xhttp = new XMLHttpRequest();
        xhttp.open("GET", "/send?cmd=" + encodeURIComponent(command), true);
        xhttp.send();
        document.getElementById("command").value = "";
        
        var dataElem = document.getElementById("data");
        var now = new Date();
        var timeStr = now.toTimeString().split(' ')[0];
        dataElem.innerHTML += "[" + timeStr + "] TX: " + command + "<br>";
        dataElem.scrollTop = dataElem.scrollHeight;
      }
    }
    
    function sendQuickCommand(cmd) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/send?cmd=" + encodeURIComponent(cmd), true);
      xhttp.send();
      
      var dataElem = document.getElementById("data");
      var now = new Date();
      var timeStr = now.toTimeString().split(' ')[0];
      dataElem.innerHTML += "[" + timeStr + "] TX: " + cmd + "<br>";
      dataElem.scrollTop = dataElem.scrollHeight;
    }
    
    function clearData() {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/clear", true);
      xhttp.send();
      
      document.getElementById("data").innerHTML = "Лог очищен<br>";
    }
    
    // Обновление данных каждую секунду
    setInterval(updateData, 1000);
    
    // Отправка по Enter
    document.getElementById("command").addEventListener("keypress", function(e) {
      if (e.key === "Enter") {
        sendCommand();
      }
    });
    
    // Первоначальная загрузка данных
    window.onload = function() {
      updateData();
      document.getElementById("command").focus();
    };
  </script>
</head>
<body>
  <div class="container">
    <h1>K-Line Адаптер</h1>
    
    <div class="card">
      <h2>Быстрые команды KWP2000</h2>
      
      <div class="cmd-group">
        <h3>Инициализация</h3>
        <div class="cmd-buttons">
          <button onclick="sendQuickCommand('81')">StartCommunication</button>
          <button onclick="sendQuickCommand('10 81 0A')">StartDiagnosticSession</button>
          <button onclick="sendQuickCommand('3E 00')">TesterPresent</button>
          <button onclick="sendQuickCommand('82')">StopCommunication</button>
        </div>
      </div>
      
      <div class="cmd-group">
        <h3>Январь-5 Euro2</h3>
        <div class="cmd-buttons">
          <button onclick="sendQuickCommand('21 8C')">ReadParamSet1</button>
          <button onclick="sendQuickCommand('21 0D')">ReadParamSet2</button>
          <button onclick="sendQuickCommand('21 0F')">ReadParamSet3</button>
        </div>
      </div>
      
      <div class="cmd-group">
        <h3>Базовая диагностика</h3>
        <div class="cmd-buttons">
          <button onclick="sendQuickCommand('11 01')">ECUReset</button>
          <button onclick="sendQuickCommand('14 00 00')">ClearDTCs</button>
          <button onclick="sendQuickCommand('18 00 00')">ReadDTCs</button>
          <button onclick="sendQuickCommand('1A 80')">ReadECUIdentification</button>
        </div>
      </div>
      
      <div class="cmd-group">
        <h3>Данные</h3>
        <div class="cmd-buttons">
          <button onclick="sendQuickCommand('21 01')">AfterSalesData</button>
          <button onclick="sendQuickCommand('21 02')">EndOfLineData</button>
          <button onclick="sendQuickCommand('21 03')">FactoryTestData</button>
          <button onclick="sendQuickCommand('21 A0')">ImmobilizerData</button>
        </div>
      </div>
    </div>
    
    <div class="card">
      <h2>Данные K-Line</h2>
      <div id="data">Загрузка...</div>
      
      <div class="input-group">
        <input type="text" id="command" placeholder="Введите HEX команду (например: 01 0D)">
        <button onclick="sendCommand()">Отправить</button>
        <button onclick="clearData()">Очистить лог</button>
      </div>
    </div>
  </div>
</body>
</html>
)=====";

// Функции для работы с OLED дисплеем
void addToOledDisplay(String line) {
  if (line.length() > 21) {
    line = line.substring(0, 21);
  }

  if (currentLine >= 7) {
    for (int i = 0; i < 7; i++) {
      oledLines[i] = oledLines[i + 1];
    }
    oledLines[7] = line;
  } else {
    oledLines[currentLine] = line;
    currentLine++;
  } 
  updateOledDisplay();
}

void updateOledDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  for (int i = 0; i < 8; i++) {
    if (oledLines[i].length() > 0) {
      display.println(oledLines[i]);
    }
  }
  
  display.display();
}

void clearOledDisplay() {
  for (int i = 0; i < 8; i++) {
    oledLines[i] = "";
  }
  currentLine = 0;
  updateOledDisplay();
}

// Функции для работы с веб-сервером
void handleRoot() {
  server.send(200, "text/html", MAIN_page);
}

void handleData() {
  server.send(200, "text/html", serialData);
}

void handleSend() {
  if (server.hasArg("cmd")) {
    String command = server.arg("cmd");
    
    // Формируем KWP2000 фрейм
    String kwpFrame = createKwpFrame(command);
    
    // Отправляем команду в последовательный порт
    for (size_t i = 0; i < kwpFrame.length(); i++) {
      Serial.print(kwpFrame[i]);
    }
    
    // Добавляем в лог
    String timeStr = getTime();
    String logEntry = "[" + timeStr + "] TX: " + command + " (Frame: " + toHexString(kwpFrame) + ")";
    serialData += logEntry + "<br>";
    
    // Добавляем на OLED дисплей
    addToOledDisplay("TX: " + command);
  }
  server.send(200, "text/plain", "OK");
}

void handleClear() {
  serialData = "Лог очищен<br>";
  clearOledDisplay();
  addToOledDisplay("Log clear");
  server.send(200, "text/plain", "OK");
}

String getTime() {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds %= 60;
  minutes %= 60;
  char buf[10];
  sprintf(buf, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return String(buf);
}

// Функции для работы с KWP2000
String createKwpFrame(String command) {
  // Убираем пробелы из команды
  command.replace(" ", "");
  
  // Преобразуем HEX строку в байты
  int len = command.length() / 2;
  byte data[len];
  for (int i = 0; i < len; i++) {
    String byteStr = command.substring(i * 2, i * 2 + 2);
    data[i] = (byte) strtol(byteStr.c_str(), NULL, 16);
  }
  
  // Создаем KWP2000 фрейм
  String frame = "";
  
  // Добавляем байт формата (физическая адресация, длина данных)
  byte formatByte = 0xC0 | (len + 2); // +2 для адресов
  frame += (char)formatByte;
  
  // Добавляем адреса (ECU -> Tester)
  frame += (char)0x10; // Адрес ECU
  frame += (char)0xF1; // Адрес тестера
  
  // Добавляем данные
  for (int i = 0; i < len; i++) {
    frame += (char)data[i];
  }
  
  // Вычисляем контрольную сумму
  byte checksum = 0;
  for (size_t i = 0; i < frame.length(); i++) {
    checksum += frame[i];
  }
  frame += (char)checksum;
  
  return frame;
}

String toHexString(String data) {
  String result = "";
  for (size_t i = 0; i < data.length(); i++) {
    if (result.length() > 0) result += " ";
    char buf[3];
    sprintf(buf, "%02X", (unsigned char)data[i]);
    result += buf;
  }
  return result;
}

// Парсинг KWP2000 фрейма
String parseKwpFrame(String frame) {
  if (frame.length() < 5) return "Invalid frame";
  
  String result = "";
  byte checksum = 0;
  
  // // Проверяем контрольную сумму
  // for (size_t i = 0; i < frame.length() - 1; i++) {
  //   checksum += frame[i];
  // }
  
  // if ((byte)frame[frame.length() - 1] != checksum) {
  //   return "Checksum error";
  // }
  
  // Извлекаем адреса
  byte targetAddr = frame[1];
  byte sourceAddr = frame[2];
  
  result += "Target: 0x";
  char buf[3];
  sprintf(buf, "%02X", targetAddr);
  result += buf;
  
  result += " Source: 0x";
  sprintf(buf, "%02X", sourceAddr);
  result += buf;
  
  // Извлекаем данные
  result += " Data: ";
  for (size_t i = 3; i < frame.length() - 1; i++) {
    sprintf(buf, "%02X ", (unsigned char)frame[i]);
    result += buf;
  }
  
  return result;
}

void setup() {
  Serial.begin(10400);
  delay(1000);
  
  // Инициализация OLED дисплея
  Wire.begin(4, 5); // SDA = GPIO4 (D2), SCL = GPIO5 (D1)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED не инициализирован!");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("K-Line Adapter");
    display.println("Launch...");
    display.display();
  }
  
  // Настройка WiFi
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  
  // Настройка сервера
  httpUpdater.setup(&server, "/firmware", "admin", "12345");
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/send", handleSend);
  server.on("/clear", handleClear);
  server.begin();
  
  // Добавляем начальное сообщение
  serialData = "K-Line launch<br>";
  serialData += "IP адрес: " + myIP.toString() + "<br>";
  serialData += "Speed: 10400 boud<br>";
  serialData += "Protocol: KWP2000<br>";
  
  // Инициализация OLED дисплея
  clearOledDisplay();
  addToOledDisplay("K-Line Adapter");
  addToOledDisplay("IP: " + myIP.toString());
  addToOledDisplay("Speed: 10400");
  addToOledDisplay("KWP2000 ready");
}

void loop() {
  server.handleClient();
  
  // Чтение данных из последовательного порта
  if (Serial.available() > 0) {
    String data = Serial.readString();
    String timeStr = getTime();
    
    // Парсим KWP2000 фрейм
    String parsedFrame = parseKwpFrame(data);
    
    String logEntry = "[" + timeStr + "] RX: " + parsedFrame;
    serialData += logEntry + "<br>";
    
    // Добавляем на OLED дисплей
    if (parsedFrame.length() > 21) {
      addToOledDisplay("RX: " + parsedFrame.substring(0, 21));
      addToOledDisplay(parsedFrame.substring(21));
    } else {
      addToOledDisplay("RX: " + parsedFrame);
    }
    
    // Ограничиваем размер буфера
    if (serialData.length() > 5000) {
      int cutPos = serialData.indexOf('<', 1000);
      if (cutPos != -1) {
        serialData = serialData.substring(cutPos);
      }
    }
  }
  
  // Небольшая задержка для стабильности
  delay(10);
}