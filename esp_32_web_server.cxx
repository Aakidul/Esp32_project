#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>

// WiFi AP credentials
const char* ssid = "UltraSonic A.i Security System";
const char* password = "12345678";

// Create a web server on port 80
WebServer server(80);

// A simple structure to hold each log entry
struct LogEntry {
  int personCount;
  String timestamp;
};

#define MAX_LOG_ENTRIES 20  // Maximum log entries stored
LogEntry logEntries[MAX_LOG_ENTRIES];
int logIndex = 0;         // Circular buffer index

// The index.html content to serve – includes HTML, CSS, and JavaScript
const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>UltraSonic A.i Security System - Home Security</title>
  <style>
    body { font-family: Arial, sans-serif; background-color: #f2f2f2; padding: 20px; }
    h1 { text-align: center; }
    table { width: 100%; border-collapse: collapse; margin-top: 20px; }
    th, td { padding: 12px; border: 1px solid #ddd; text-align: center; }
    th { background-color: #4CAF50; color: white; }
  </style>
</head>
<body>
  <h1>UltraSonic A.i Security System Log</h1>
  <table>
    <thead>
      <tr>
        <th>Time (IST)</th>
        <th>Persons Detected</th>
      </tr>
    </thead>
    <tbody id="logTable">
    </tbody>
  </table>
  <script>
    // Fetch the log entries from the server and update the table
    function fetchLog() {
      fetch('/log')
        .then(response => response.json())
        .then(data => {
          const table = document.getElementById('logTable');
          table.innerHTML = '';
          data.forEach(entry => {
            const row = document.createElement('tr');
            const timeCell = document.createElement('td');
            const personCell = document.createElement('td');
            timeCell.textContent = entry.timestamp;
            personCell.textContent = entry.person;
            row.appendChild(timeCell);
            row.appendChild(personCell);
            table.appendChild(row);
          });
        })
        .catch(error => console.error('Error fetching log:', error));
    }
    // Update the log every 5 seconds
    setInterval(fetchLog, 5000);
    fetchLog();
  </script>
</body>
</html>
)rawliteral";

// Handler for serving the index page
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// Handler for returning the log as a JSON array
void handleLog() {
  String json = "[";
  for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
    if (logEntries[i].timestamp != "") {
      json += "{\"timestamp\":\"" + logEntries[i].timestamp + "\",\"person\":" + String(logEntries[i].personCount) + "},";
    }
  }
  if (json.endsWith(",")) {
    json.remove(json.length()-1); // Remove trailing comma
  }
  json += "]";
  server.send(200, "application/json", json);
}

// Handler for POST requests to /update with JSON data (e.g., {"person":5})
void handleUpdate() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Bad Request\"}");
    return;
  }
  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  int personCount = doc["person"];

  // Get current time in IST (using NTP time set with IST offset)
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  char timeString[16];
  strftime(timeString, sizeof(timeString), "%I:%M %p", &timeinfo);

  // Save the new log entry in our circular buffer
  logEntries[logIndex].personCount = personCount;
  logEntries[logIndex].timestamp = String(timeString);
  logIndex = (logIndex + 1) % MAX_LOG_ENTRIES;
  
  server.send(200, "application/json", "{\"status\":\"OK\"}");
}

void setup() {
  Serial.begin(115200);
  
  // Start WiFi Access Point
  WiFi.softAP(ssid, password);
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Configure NTP for IST (UTC+5:30 = 19800 seconds offset, no DST)
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }

  // Define server routes/endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/log", HTTP_GET, handleLog);
  server.on("/update", HTTP_POST, handleUpdate);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}