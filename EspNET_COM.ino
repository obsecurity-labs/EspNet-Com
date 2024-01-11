    #include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <U8g2lib.h>

const char* ssid = "espNET-COM";
const char* password = "pass@3301";
WebServer server(80);

// Define the U8g2 display object
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE);

// Define the physical button pins
const int leftButtonPin = 4;  // Change to the actual pin number
const int rightButtonPin = 5; // Change to the actual pin number

void setup() {
  Serial.begin(115200);
  delay(1000);

  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP IP address: " + IP.toString());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/message", HTTP_POST, handleMessage);
  server.on("/events", HTTP_GET, handleEvents);
  server.begin();

  // Initialize the physical buttons
  pinMode(leftButtonPin, INPUT_PULLUP); // INPUT_PULLUP for active-low buttons
  pinMode(rightButtonPin, INPUT_PULLUP);

  // Display initial intro for 4 seconds
  displayIntro();
}

void loop() {
  server.handleClient();

  // Check the state of the left button
  if (digitalRead(leftButtonPin) == LOW) {
    // Left button is pressed
    String message = "[>]: Copy that";

    // Display message with border and centered
    displayMessageWithBorder(message);

    // Send the message to the chat area using Server-Sent Events (SSE)
    sendMessageToWebChat(message);

    delay(1000); // Delay to debounce the button press
  }

  // Check the state of the right button
  if (digitalRead(rightButtonPin) == LOW) {
    // Right button is pressed
    String message = "[>]: alert";

    // Display message with border and centered
    displayMessageWithBorder(message);

    // Send the message to the chat area using Server-Sent Events (SSE)
    sendMessageToWebChat(message);

    delay(1000); // Delay to debounce the button press
  }
}

void displayIntro() {
  // Display initial intro for 4 seconds
  unsigned long introStartTime = millis();
  while (millis() - introStartTime < 4000) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 15);
    u8g2.print("espNET-COM");
    u8g2.setCursor(0, 30);
    u8g2.print("OBSECURITY-LABS");
    
    // Add animated "Server started..." message
    static const char* dots[] = {"   ", ".  ", ".. ", "..."};
    int dotIndex = (millis() / 500) % 4;
    u8g2.setCursor(0, 45);
    u8g2.print("Server started");
    u8g2.print(dots[dotIndex]);
    
    u8g2.sendBuffer();
  }
}

void displayMessageWithBorder(const String& message) {
  // Clear the display and calculate text size
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int textWidth = u8g2.getStrWidth(message.c_str());
  int textHeight = u8g2.getMaxCharHeight();
  
  // Calculate the position to center the text
  int centerX = (u8g2.getDisplayWidth() - textWidth) / 2;
  int centerY = (u8g2.getDisplayHeight() - textHeight) / 2;
  
  // Draw the border and display the centered message
  u8g2.drawFrame(0, 0, u8g2.getDisplayWidth(), u8g2.getDisplayHeight()); // Draw the border
  u8g2.setCursor(centerX, centerY);
  u8g2.print(message);
  u8g2.sendBuffer();
}

// Rest of your code for handling web interface, SSE, and button presses...
    
    
void handleRoot() {
  String html = "<html>";
  html += "<head>";
  html += "<style>";
  html += "body { background-color: #1a1a1a; color: #ffffff; font-family: Arial, sans-serif; }";
  html += "h1 { color: #ff9900; margin: 0; padding: 20px 0; text-align: center; }";
  html += ".container { max-width: 600px; margin: 0 auto; padding: 20px; }";
  html += ".message { background-color: #333333; border-radius: 8px; padding: 10px; margin: 10px 0; }";
  html += ".message p { margin: 0; }";
  html += "input[type='text'] { width: 80%; padding: 10px; border: none; border-radius: 4px; }";
  html += "input[type='submit'] { background-color: #ff9900; color: #fff; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<h1 font color='white'>espNET-COM</h1>";
  html += "<center><h3><font color='white'>OB</font><font color='red'>SEC</font><font color='white'>URITY-LABS</font></h3></center>";
    
   html += "<center><p><font color='white'>WiFi-Private Communication</p></center>";
  html += "<div class='container'>";
  html += "<div id='chatArea'>";
  html += "</div>";
  html += "<form onsubmit='event.preventDefault(); submitForm();'>";
  html += "<input type='text' id='messageInput' placeholder='Type your message here' required>";
  html += "<input type='submit' value='Send'>";
  html += "</form>";
  html += "<script>";
  html += "var eventSource = new EventSource('/events');";
  html += "eventSource.onmessage = function(event) {";
  html += "var chatArea = document.getElementById('chatArea');";
  html += "var message = event.data;";
  html += "chatArea.innerHTML += \"<div class='message'><p>\" + message + \"</p></div>\";";
  html += "};";
  html += "function submitForm() {";
  html += "var inputField = document.getElementById('messageInput');";
  html += "var message = inputField.value;";
  html += "var chatArea = document.getElementById('chatArea');";
  html += "chatArea.innerHTML += \"<div class='message'><p>[>]: \" + message + \"</p></div>\";";
  html += "fetch('/message', {";
  html += "method: 'POST',";
  html += "body: 'message=' + message,";
  html += "headers: {";
  html += "'Content-Type': 'application/x-www-form-urlencoded'";
  html += "}";
  html += "})";
  html += ".then(function(response) {";
  html += "return response.text();";
  html += "})";
  html += ".then(function(data) {";
  html += "alert(data);"; // Display the popup
  html += "inputField.value = '';"; // Clear the input field
  html += "});";
  html += "}";
  html += "</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleMessage() {
  if (server.hasArg("message")) {
    String message = server.arg("message");

    // Display message on the OLED
    u8g2.clearBuffer();
    u8g2.setCursor(0, 15);
    u8g2.print("[<]: " + message);
    u8g2.sendBuffer();

    // Send the message to the chat area using Server-Sent Events (SSE)
    sendMessageToWebChat("[<]: " + message);

    // Send a response to /message
    String response = "Done";
    server.send(200, "text/plain", response);
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void handleEvents() {
  // Empty function to handle the /events path
}

void sendMessageToWebChat(const String& message) {
  // Send the message using Server-Sent Events (SSE)
  String eventData = "data: " + message + "\n\n";
  server.send(200, "text/event-stream", eventData);
}
    