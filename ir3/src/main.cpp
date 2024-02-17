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

inline String buttonHtml(String text, String destination) {
    return "<div class=\"grid-item\"><button onclick=\"xhr('/b/" + destination + "')\" class=\"button\">" + text + "</button></div>";
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

            if (header.indexOf("GET /b/chnmax") >= 0)
              IrSender.sendRC5(0, BUT_CH_UP, 1, true);
            else if (header.indexOf("GET /b/arrwup") >= 0)
              IrSender.sendRC5(0, BUT_ARR_UP, 1, true);
            else if (header.indexOf("GET /b/chnmin") >= 0)
              IrSender.sendRC5(0, BUT_CH_DOWN, 1, true);

            else if (header.indexOf("GET /b/arleft") >= 0)
              IrSender.sendRC5(0, BUT_ARR_LEFT, 1, true);
            else if (header.indexOf("GET /b/okokok") >= 0)
              IrSender.sendRC5(0, BUT_OK, 1, true);
            else if (header.indexOf("GET /b/aright") >= 0)

              IrSender.sendRC5(0, BUT_ARR_RIGHT, 1, true);
            else if (header.indexOf("GET /b/volmax") >= 0)
              IrSender.sendRC5(0, BUT_VOL_UP, 1, true);
            else if (header.indexOf("GET /b/ardown") >= 0)
              IrSender.sendRC5(0, BUT_ARR_DOWN, 1, true);
            else if (header.indexOf("GET /b/volmin") >= 0)
              IrSender.sendRC5(0, BUT_VOL_DOWN, 1, true);

            else if (header.indexOf("GET /b/aspect") >= 0)
              IrSender.sendRC5(0, BUT_ASPECT, 1, true);
            else if (header.indexOf("GET /b/fmradio") >= 0)
              IrSender.sendRC5(0, BUT_RADIO, 1, true);
            else if (header.indexOf("GET /b/sound") >= 0)
              IrSender.sendRC5(0, BUT_SOUND, 1, true);

            else if (header.indexOf("GET /b/menu") >= 0)
              IrSender.sendRC5(0, BUT_MENU, 1, true);
            else if (header.indexOf("GET /b/power") >= 0)
              IrSender.sendRC5(0, BUT_POWER, 1, true);  
            else if (header.indexOf("GET /c/") >= 0)
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
              client.println(".button { background-color: #195B6A; border: none; padding: 16px 40px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
              client.println(".button2 {background-color: #77878A;}");
              client.println(".grid-container{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}.grid-item{text-align:center}.button{padding:10px 20px;font-size:16px;border:1px solid #ccc;background-color:#f0f0f0;cursor:pointer}"); // 3x3 grid container class
              client.println("</style><script>");
              client.println("function xhr(loc,callback=Function.prototype){var xhttp=new XMLHttpRequest();xhttp.onreadystatechange=function(){if(this.readyState==4&&this.status==200){callback(this)}};xhttp.open(\"GET\",loc);xhttp.send()}");
              client.println("</script><title>&#x1F4FA;</title></head><body><h1>&#x1F4FA;</h1>");

              // Buttons
              client.println("<div class=\"grid-container\">");
              client.println(buttonHtml("CH+", "chnmax"));
              client.println(buttonHtml("&#94;", "arrwup"));
              client.println(buttonHtml("CH-", "chnmin"));

              client.println(buttonHtml("&lt;", "arleft"));
              client.println(buttonHtml("OK", "okokok"));
              client.println(buttonHtml("&gt;", "aright"));

              client.println(buttonHtml("Vol+", "volmax"));
              client.println(buttonHtml("v", "ardown"));
              client.println(buttonHtml("Vol-", "volmin"));

              client.println(buttonHtml("Aspect", "aspect"));
              client.println(buttonHtml("FM", "fmradio"));
              client.println(buttonHtml("Sound", "sound"));

              client.println(buttonHtml("Menu", "menu"));
              client.println(buttonHtml("Power", "power"));
              client.println("<div class=\"grid-item\"><input class=\"button\" style=\"width: 50px;\" type=\"number\" id=\"customCommand\" onkeyup=\"if (event.key ===\'Enter\') {document.getElementById('blastbutton').click()}\">");
              client.println("<button id=\"blastbutton\" onclick=\"xhr('/c/'+document.getElementById('customCommand').value)\" class=\"button\">Blast</button></div>");
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
