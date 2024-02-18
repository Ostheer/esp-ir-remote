#include <Arduino.h>
#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>
#include <ESP8266WiFi.h>

#define DISABLE_CODE_FOR_RECEIVER // Disables restarting receiver after each send. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not used.
#define EXCLUDE_EXOTIC_PROTOCOLS  // Saves around 240 bytes program memory if IrSender.write is used

// The web server stuff was inspired by https://randomnerdtutorials.com/esp8266-web-server/

// These come from https://github.com/probonopd/irdb/blob/master/codes/Philips/TV/0%2C-1.csv
#define BUT_MENU 46
#define BUT_VOL_UP 16
#define BUT_VOL_DOWN 17
#define BUT_CH_UP 32
#define BUT_CH_DOWN 33
#define BUT_POWER 12

// These come from https://github.com/probonopd/irdb/blob/master/codes/Philips/Unknown_PHILIPS/0%2C-1.csv
#define BUT_ARR_UP    80
#define BUT_ARR_DOWN  81
#define BUT_ARR_RIGHT 86
#define BUT_ARR_LEFT  85
#define BUT_OK  87

// These were found brute-forcing codes
#define BUT_ASPECT 85
#define BUT_RADIO 113
#define BUT_SOUND 36

const char *ssid = "ssid";
const char *password = "pass";
#define HOSTNAME "tvremote" // This doesn't seem to work lol

WiFiServer server(80);
String header;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

inline String buttonHtml(String text, int function) {
    return "<div class=\"grid-item\"><button onclick=\"xhr('/c/" + String(function) + "')\" class=\"button\">" + text + "</button></div>";
}

void setup() {
  IrSender.begin();
  IrSender.enableIROut(38);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(250);
  server.begin();
}

void loop()
{
  WiFiClient client = server.available();

  if (client)
  {
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    {
      currentTime = millis();
      if (client.available())
      {
        char c = client.read();
        header += c;
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /c/") >= 0)
              IrSender.sendRC5(0, header.substring(7, header.indexOf("HTTP/1.1")).toInt(), 1, true);
            else if (header.indexOf("GET /c6/") >= 0)
              // Send RC6 command with custom address/function code, e.g. "curl http://esp-373e8d/c6/4/247"
              IrSender.sendRC6(header.substring(8, 9).toInt(), header.substring(10, header.indexOf("HTTP/1.1")).toInt(), 1, true);
              
            else {
              // Preamble
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              client.println(".button { background-color: #195B6A; border: none; padding: 16px 40px; width: 100px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
              client.println(".grid-container{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}.grid-item{text-align:center}.button{padding:10px 20px;font-size:16px;border:1px solid #ccc;background-color:#f0f0f0;cursor:pointer}"); // 3x3 grid container class
              client.println("</style><script>");
              client.println("function xhr(loc,callback=Function.prototype){var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if(this.readyState==4&&this.status==200){callback(this)}};xhttp.open(\"GET\",loc);xhttp.send()}");
              client.println("</script><title>&#x1F4FA;</title></head><body><h1>&#x1F4FA;</h1>");

              // Buttons
              client.println("<div class=\"grid-container\">");
              client.println(buttonHtml("CH+", BUT_CH_UP));
              client.println(buttonHtml("&#94;", BUT_ARR_UP));
              client.println(buttonHtml("CH-", BUT_CH_DOWN));

              client.println(buttonHtml("&lt;", BUT_ARR_LEFT));
              client.println(buttonHtml("OK", BUT_OK));
              client.println(buttonHtml("&gt;", BUT_ARR_RIGHT));

              client.println(buttonHtml("Vol+", BUT_VOL_UP));
              client.println(buttonHtml("v", BUT_ARR_DOWN));
              client.println(buttonHtml("Vol-", BUT_VOL_DOWN));

              client.println(buttonHtml("Aspect", BUT_ASPECT));
              client.println(buttonHtml("FM", BUT_RADIO));
              client.println(buttonHtml("Sound", BUT_SOUND));

              client.println(buttonHtml("Menu", BUT_MENU));
              client.println(buttonHtml("Power", BUT_POWER));
              client.println("<div class=\"grid-item\"><input class=\"button\" style=\"width: 50px;\" type=\"number\" id=\"customCommand\" onkeyup=\"if (event.key ===\'Enter\') {document.getElementById('blastbutton').click()}\">");
              client.println("<button id=\"blastbutton\" onclick=\"xhr('/c/'+document.getElementById('customCommand').value)\" class=\"button\" style=\"width: 25px;\">!</button></div>");

              for (int i = 1; i < 10; i++)
                client.println(buttonHtml(String(i), i));
              client.println("<div class=\"grid-item\"></div>");
              client.println(buttonHtml(String(0), 0));
              client.println("<div class=\"grid-item\"></div>");
              
              client.println("</div>");

              // End document
              client.println("</body></html>");
              client.println();
            }

            break; // while
          }
          else
            currentLine = "";
        }
        else if (c != '\r')
          currentLine += c;
      }
    }
    header = "";
    client.stop();
  }
}
