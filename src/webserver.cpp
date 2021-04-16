
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h> 
#include <WebServer.h>

void my_delay_ms(int msec);

extern String sta_ssid;    	//  your network SSID (name)
extern String sta_pass; 	// your network password

WebServer server(80);

void rootPageHandler(void);
void wlanPageHandler(void);
void gpioPageHandler(void);
void handleNotFound(void);

void webserver_setup() {
  /* Set page handler functions */
  server.on("/", rootPageHandler);
  server.on("/wlan_config", wlanPageHandler);
  server.on("/gpio", gpioPageHandler);
  server.onNotFound(handleNotFound);

  server.begin();
}

void webserver_loop()
{
	server.handleClient();
}

/* Root page for the webserver */
void rootPageHandler()
{
	String response_message = "<html><head><title>WWVB Emulator</title></head>";
	response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>WWVB Emulator</center></h1>";

	if (WiFi.status() == WL_CONNECTED)
	{
		response_message += "<h3><center>WLAN Status: Connected</center></h3>";
	}
	else
	{
		response_message += "<h3><center>WLAN Status: Disconnected</center></h3>";
	}

	response_message += "<h4><center><a href=\"/wlan_config\">Configure WLAN settings</a></center></h4>";
	response_message += "<h4><center><a href=\"/gpio\">Display Logo On/Off</h4></li></center></h4>";
	response_message += "</body></html>";

	server.send(200, "text/html", response_message);
}

/* WLAN page allows users to set the WiFi credentials */
void wlanPageHandler()
{
	// Check if there are any GET parameters
	if (server.hasArg("ssid"))
	{
		if (server.hasArg("password"))
		{
			WiFi.begin(server.arg("ssid").c_str(), server.arg("password").c_str());
		}
		else
		{
			WiFi.begin(server.arg("ssid").c_str());
		}

		Serial.println(F("Reconnecting... "));

		for (int i = 0; i<200; i++)
		{
			if (WiFi.status() == WL_CONNECTED)
			{
				Serial.println("WiFi reconnected");
				Serial.println("New IP address: ");
				Serial.println(WiFi.localIP());
				break;
			}
			my_delay_ms(50);
		}
		my_delay_ms(100);
		webserver_setup();
	}

	String response_message = "";
	response_message += "<html>";
	response_message += "<head><title>WWVB Emulator</title></head>";
	response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>WLAN Settings</center></h1>";

	response_message += "<ul><li><a href=\"/\">Return to main page</a></li></ul>";

	if (WiFi.status() == WL_CONNECTED)
	{
		response_message += "Status: Connected<br>";
	}
	else
	{
		response_message += "Status: Disconnected<br>";
	}

	response_message += "<p>To connect to a WiFi network, please select a network...</p>";

	// Get number of visible access points
	int ap_count = WiFi.scanNetworks();

	if (ap_count == 0)
	{
		response_message += "No access points found.<br>";
	}
	else
	{
		response_message += "<form method=\"get\">";

		// Show access points
		for (uint8_t ap_idx = 0; ap_idx < ap_count; ap_idx++)
		{
      if (String(WiFi.SSID(ap_idx))==sta_ssid)
      {
        response_message += "<input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\" checked>";
      }
      else
      {
        response_message += "<input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\">";
      }
			response_message += String(WiFi.SSID(ap_idx)) + " (RSSI: " + WiFi.RSSI(ap_idx) + ")";
                        (WiFi.encryptionType(ap_idx) == WIFI_AUTH_OPEN)
                            ? response_message += " "
                            : response_message += "*";
                        response_message += "<br><br>";
		}

		response_message += "WiFi password (if required):<br>";
    response_message += "<input type=\"text\" value=\""+sta_pass+"\" name=\"password\"><br>";
		response_message += "<input type=\"submit\" value=\"Connect\">";
		response_message += "</form>";
	}

	response_message += "</body></html>";

	server.send(200, "text/html", response_message);
}

boolean bLogo = true;

inline boolean LogoOn()
{
	return bLogo;
}

inline void SetLogo(boolean OnLogo)
{
	bLogo = OnLogo;
}


/*///////////////////////////////////////////////////////////////////////////*/

/* GPIO page allows you to control the GPIO pins */
void gpioPageHandler()
{
	// Check if there are any GET parameters
	if (server.hasArg("gpio2"))
	{
		if (server.arg("gpio2") == "1")
		{
			//digitalWrite(GPIO2, HIGH);
			bLogo = true;
		}
		else
		{
			//digitalWrite(GPIO2, LOW);
			bLogo = false;
		}
	}

	String response_message = "<html><head><title>WWVB Emulator</title></head>";
	response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>Control GPIO pins</center></h1>";
	response_message += "<form method=\"get\">";

	response_message += "<ul><li><a href=\"/\">Return to main page</a></li></ul>";

	response_message += "LOGO:<br>";

	if (bLogo == false)
	{
		response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\">On<br>";
		response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\" checked>Off<br>";
	}
	else
	{
		response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\" checked>On<br>";
		response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\">Off<br>";
	}

	response_message += "</form></body></html>";

	server.send(200, "text/html", response_message);
}

/* Called if requested page is not found */
void handleNotFound()
{
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for (uint8_t i = 0; i < server.args(); i++)
	{
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}

	server.send(404, "text/plain", message);
}

