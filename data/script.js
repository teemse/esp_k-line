function updateData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      // Очищаем текущий лог и добавляем новые данные
      var newData = this.responseText;
      var dataDiv = document.getElementById("data");

      // Если ответ содержит данные, парсим и отображаем их
      if (newData && newData.trim() !== "") {
        // Разделяем строки и создаем элементы лога
        var logEntries = newData.split("<br>");
        var htmlContent = "";

        logEntries.forEach(function (entry) {
          if (entry.trim() !== "") {
            // Определяем тип записи (TX, RX или информация)
            var now = new Date();
            var timeStr = now.toTimeString().split(" ")[0];

            if (entry.includes("TX:")) {
              htmlContent += `<div class="log-entry">
                <span class="log-time">[${timeStr}]</span>
                <span class="log-sent">${entry.replace(
                  "TX:",
                  "Отправлено:"
                )}</span>
              </div>`;
            } else if (entry.includes("RX:")) {
              htmlContent += `<div class="log-entry">
                <span class="log-time">[${timeStr}]</span>
                <span class="log-received">${entry.replace(
                  "RX:",
                  "Получено:"
                )}</span>
              </div>`;
            } else {
              htmlContent += `<div class="log-entry">
                <span class="log-time">[${timeStr}]</span>
                <span class="log-info">${entry}</span>
              </div>`;
            }
          }
        });

        dataDiv.innerHTML = htmlContent;
        dataDiv.scrollTop = dataDiv.scrollHeight;
      }
    }
  };
  xhttp.open("GET", "/data", true);
  xhttp.send();
}

function sendCommand() {
  var command = document.getElementById("command").value.trim();
  if (command) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/send?cmd=" + encodeURIComponent(command), true);
    xhttp.send();
    document.getElementById("command").value = "";

    var dataElem = document.getElementById("data");
    var now = new Date();
    var timeStr = now.toTimeString().split(" ")[0];

    // Добавляем красивую запись о отправленной команде
    dataElem.innerHTML += `
      <div class="log-entry">
        <span class="log-time">[${timeStr}]</span>
        <span class="log-sent">Отправлено: ${command}</span>
      </div>`;
    dataElem.scrollTop = dataElem.scrollHeight;
  }
}

function sendQuickCommand(cmd) {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/send?cmd=" + encodeURIComponent(cmd), true);
  xhttp.send();

  var dataElem = document.getElementById("data");
  var now = new Date();
  var timeStr = now.toTimeString().split(" ")[0];

  // Добавляем красивую запись о отправленной быстрой команде
  dataElem.innerHTML += `
    <div class="log-entry">
      <span class="log-time">[${timeStr}]</span>
      <span class="log-sent">Отправлено: ${cmd}</span>
    </div>`;
  dataElem.scrollTop = dataElem.scrollHeight;
}

function clearData() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/clear", true);
  xhttp.send();

  // Очищаем лог с красивым сообщением
  var now = new Date();
  var timeStr = now.toTimeString().split(" ")[0];
  document.getElementById("data").innerHTML = `
    <div class="log-entry">
      <span class="log-time">[${timeStr}]</span>
      <span class="log-info">Лог очищен</span>
    </div>`;
}

// Обновление данных каждую секунду
setInterval(updateData, 1000);

// Отправка по Enter
document.getElementById("command").addEventListener("keypress", function (e) {
  if (e.key === "Enter") {
    sendCommand();
  }
});

// Обновление статуса подключения
function updateConnectionStatus() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var status = this.responseText;
      var statusIndicator = document.querySelector(".status-indicator");
      var statusBadge = document.querySelector(".connection-status .badge");

      if (status.includes("connected") || status.includes("актив")) {
        statusIndicator.className = "status-indicator status-connected";
        statusBadge.className = "badge badge-success";
        statusBadge.textContent = "Активно";
      } else {
        statusIndicator.className = "status-indicator status-disconnected";
        statusBadge.className = "badge badge-danger";
        statusBadge.textContent = "Неактивно";
      }
    }
  };
  xhttp.open("GET", "/status", true);
  xhttp.send();
}

// Первоначальная загрузка данных
window.onload = function () {
  updateData();
  updateConnectionStatus();
  document.getElementById("command").focus();

  // Обновляем статус подключения каждые 5 секунд
  setInterval(updateConnectionStatus, 5000);
};
