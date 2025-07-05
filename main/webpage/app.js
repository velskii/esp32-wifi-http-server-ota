h;
/**
 * Add gobals here
 */
var seconds = null;
var otaTimerVar = null;
var wifiConnectInterval = null;

/**
 * Initialize functions here.
 */
$(document).ready(function () {
  getUpdateStatus();
  startDHTSensorInterval();
  $("#connect_wifi").on("click", function () {
    checkCredentials();
  });
});

/**
 * Gets file name and size for display on the web page.
 */
function getFileInfo() {
  var x = document.getElementById("selected_file");
  var file = x.files[0];

  document.getElementById("file_info").innerHTML =
    "<h4>File: " + file.name + "<br>" + "Size: " + file.size + " bytes</h4>";
}

/**
 * Handles the firmware update.
 */
function updateFirmware() {
  // Form Data
  var formData = new FormData();
  var fileSelect = document.getElementById("selected_file");

  if (fileSelect.files && fileSelect.files.length == 1) {
    var file = fileSelect.files[0];
    formData.set("file", file, file.name);
    document.getElementById("ota_update_status").innerHTML =
      "Uploading " + file.name + ", Firmware Update in Progress...";

    // Http Request
    var request = new XMLHttpRequest();

    request.upload.addEventListener("progress", updateProgress);
    request.open("POST", "/OTAupdate");
    request.responseType = "blob";
    request.send(formData);
  } else {
    window.alert("Select A File First");
  }
}

/**
 * Progress on transfers from the server to the client (downloads).
 */
function updateProgress(oEvent) {
  if (oEvent.lengthComputable) {
    getUpdateStatus();
  } else {
    window.alert("total size is unknown");
  }
}

/**
 * Posts the firmware udpate status.
 */
function getUpdateStatus() {
  var xhr = new XMLHttpRequest();
  var requestURL = "/OTAstatus";
  xhr.open("POST", requestURL, false);
  xhr.send("ota_update_status");

  if (xhr.readyState == 4 && xhr.status == 200) {
    var response = JSON.parse(xhr.responseText);

    document.getElementById("latest_firmware").innerHTML =
      response.compile_date + " - " + response.compile_time;

    // If flashing was complete it will return a 1, else -1
    // A return of 0 is just for information on the Latest Firmware request
    if (response.ota_update_status == 1) {
      // Set the countdown timer time
      seconds = 10;
      // Start the countdown timer
      otaRebootTimer();
    } else if (response.ota_update_status == -1) {
      document.getElementById("ota_update_status").innerHTML =
        "!!! Upload Error !!!";
    }
  }
}

/**
 * Displays the reboot countdown.
 */
function otaRebootTimer() {
  document.getElementById("ota_update_status").innerHTML =
    "OTA Firmware Update Complete. This page will close shortly, Rebooting in: " +
    seconds;

  if (--seconds == 0) {
    clearTimeout(otaTimerVar);
    window.location.reload();
  } else {
    otaTimerVar = setTimeout(otaRebootTimer, 1000);
  }
}

/**
 * Gets the DHT11 sensor data for display on the web page.
 */
function getDHTSensorValues() {
  $.getJSON("/dhtSensor.json", function (data) {
    $("#temperature_reading").text(data["temperature"] + " °C");
    $("#humidity_reading").text(data["humidity"] + " %");
  });
}

/**
 * Sets an interval to get the DHT11 sensor values every 2 seconds.
 */
function startDHTSensorInterval() {
  setInterval(getDHTSensorValues, 5000);
}

/**
 * Clears the connection status interval.
 */
function stopWifiConnectStatusInterval() {
  if (wifiConnectInterval != null) {
    clearInterval(wifiConnectInterval);
    wifiConnectInterval = null;
    document.getElementById("wifi_connect_status").innerHTML = "";
  }
}

/**
 * Gets the WiFi connection status.
 */
function getWifiConnectStatus() {
  var xhr = new XMLHttpRequest();
  var requestURL = "/wifiConnectStatus";
  xhr.open("POST", requestURL, false);
  xhr.send("wifi_connect_status");

  if (xhr.readyState == 4 && xhr.status == 200) {
    var response = JSON.parse(xhr.responseText);
    document.getElementById("wifi_connect_status").innerHTML = "Connecting...";

    if (response.wifi_connect_status == 2) {
      document.getElementById("wifi_connect_status").innerHTML =
        "<h4 class='rd'>Failed to connect. Please check your AP credentials and compatibility.</h4>";
      stopWifiConnectStatusInterval();
    } else if (response.wifi_connect_status == 3) {
      document.getElementById("wifi_connect_status").innerHTML =
        "<h4 class='gr'>Connected to WiFi successfully!</h4>";
      stopWifiConnectStatusInterval();
    }
  }
}

/**
 * Starts the interval for checking the WiFi connection status.
 */
function startWifiConnectStatusInterval() {
  wifiConnectInterval = setInterval(getWifiConnectStatus, 2800);
}

/**
 * Connect WiFi function called using the SSID and password from the form.
 */
function connectWifi() {
  // Get the SSID and password from the form
  var selectedSSID = $("#connect_ssid").val();
  var pwd = $("#connect_pass").val();

  $.ajax({
    type: "POST",
    url: "/wifiConnect.json",
    cache: false,
    headers: {
      "my-connect-ssid": selectedSSID,
      "my-connect-pwd": pwd,
    },
    data: { timestamp: new Date().getTime() },
    contentType: "application/json",
    success: function (response) {
      // Handle success response
      $("#wifi_connect_status").html("Connecting to WiFi...");
      startWifiConnectStatusInterval();
    },
    error: function (xhr, status, error) {
      // Handle error response
      $("#wifi_connect_status").html("Error connecting to WiFi: " + error);
    },
  });
}

/**
 * Checks credentials for the WiFi connection.
 */
function checkCredentials() {
  var errorList = "";
  var credsOK = true;
  var selectedSSID = $("#connect_ssid").val();
  var pwd = $("#connect_pass").val();

  if (selectedSSID === "") {
    errorList += "SSID cannot be empty.<br>";
    credsOK = false;
  }
  if (pwd === "") {
    errorList += "Password cannot be empty.<br>";
    credsOK = false;
  }
  if (credsOK === false) {
    $("#wifi_connect_credentials_errors").html(errorList);
  } else {
    $("#wifi_connect_credentials_errors").html("");
    connectWifi();
  }
}

/**
 * Shows the WiFi password if the box is checked.
 */
function showPassword() {
  var pwdField = document.getElementById("connect_pass");
  if (pwdField.type === "password") {
    pwdField.type = "text";
  } else {
    pwdField.type = "password";
  }
}
