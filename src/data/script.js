function updateData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("data").innerHTML = this.responseText;
      var elem = document.getElementById("data");
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
    var timeStr = now.toTimeString().split(" ")[0];
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
  var timeStr = now.toTimeString().split(" ")[0];
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
document.getElementById("command").addEventListener("keypress", function (e) {
  if (e.key === "Enter") {
    sendCommand();
  }
});

// Первоначальная загрузка данных
window.onload = function () {
  updateData();
  document.getElementById("command").focus();
};
