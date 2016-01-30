// TODO: create an interface to change these

const char ssid[] = "YOURSSID";  //  your network SSID (name)
const char pass[] = "YOURPASS";       // your network password

const char* ntpServerName = "ntp.example.com"; // NTP server hostname - see http://www.pool.ntp.org/en/vendors.html
#define NTP_INTERVAL 1024  // seconds between polling the NTP server - check with your ntp operator before lowering this

// TODO: more time zones
TimeChangeRule usCDT = {"CDT", Second, dowSunday, Mar, 2, -300};
TimeChangeRule usCST = {"CST", First, dowSunday, Nov, 2, -360};
Timezone usCT(usCDT, usCST);

#define TIMEZONE usCT
