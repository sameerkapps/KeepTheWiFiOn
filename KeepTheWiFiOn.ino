/*
 * Copyrights 2016 Sameer Khandekar.
 * MIT License
*/

#include <SPI.h>
#include <WiFi101.h>

char networkName[] = "";     //  your network SSID (name)
char wifiPass[] = "";   // your wifi password

// Frequency of the buzzer
const int BUZZER_FREQUENCY = 1000;
// buzzer pin
const int BUZZER_PIN = 3;
// Power Relay to turn router on/off
const int RELAY_PIN = 0;

// interval after which checking should be done When in connected state
const int CheckAfterDuration = 1*60*1000;

// interval after which disconnected should be processed. 
// This give heads up to those connected using wires prior to reboot.
const int RebootDelay = 20*1000;

// The time for which router remains off.
const int RouterOffDuration = 30*1000;

// The time for router to fully restart wifi from the time of power on.
const int WifiStartDuration = 2*60*1000;

// remote server for testing connectivity
char servername[]= "google.com"; 

// Initialize the client library
WiFiClient client;

// States of the apparatus
enum States
{
  Checking,
  Connected,
  Disconnected,
  RouterOff,
  RouterOn
};

// time at which state started
long stateStartMilliSec = 0;

// current state
enum States currentState;

void setup() 
{
  //Initialize serial and wait for port to open:
  // Serial.begin(9600);
  // while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  // }  
  // initialize Buzzer and Relay pins as output pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // connect to Wi-Fi
  ConnectToWiFi();
   
  // set initial state
  ChangeState(Checking);
}

// the loop function 
void loop() 
{
  // call process method as per state
     switch(currentState)
     {
        case Checking:
         Process_State_Checking();
         break;

        case Connected:
         Process_State_Connected();
         break;

        case Disconnected:
         Process_State_Disconnected();
         break;

        case RouterOff:
         Process_State_RouterOff();
         break;

        case RouterOn:
         Process_State_RouterOn();
         break;
     }
}

void Enter_State_Checking()
{
  // wifi.status() method does not return status correctly. It would return connected
  // even after wi-fi has been shut. To get around this, the method attempts to connect to the server.
  
  States nextState = client.connect(servername, 80) ? Connected : Disconnected;
  if(nextState == Disconnected)
  {
    // Serial.println("WiFi Disconnected");
  }
  else
  {
    client.stop(); // disconnect
    // Serial.println("WiFi Connected");
  }
  ChangeState(nextState);
}

void Process_State_Checking()
{
    // do nothing
}

void Enter_State_Connected()
{
  // reset the timer
  stateStartMilliSec = millis();  
}

void Process_State_Connected()
{
  // if time has elapsed, tranistion to Checking state
  long timeNow = millis();
  if((timeNow - stateStartMilliSec) >= CheckAfterDuration)
  {
    ChangeState(Checking);  
  }
}

void Enter_State_Disconnected()
{
  // reset the timer
  stateStartMilliSec = millis(); 
  // Turn on the speaker prior to reboot
  tone(BUZZER_PIN, BUZZER_FREQUENCY);
}

void Process_State_Disconnected()
{
  // wait for some time, for reboot
  long timeNow = millis();
  if((timeNow - stateStartMilliSec) >= RebootDelay)
  {
    ChangeState(RouterOff);  
  }
}

void Enter_State_RouterOff()
{
  // reset the timer
  stateStartMilliSec = millis(); 

  // Turn the router off here
  // router is connected in NC mode.
  // So turn off requires pin to be set to high
  digitalWrite(RELAY_PIN, HIGH);
}

void Process_State_RouterOff()
{
  // after the router is turned off for suffient duration
  // change state to On
  long timeNow = millis();
  if((timeNow - stateStartMilliSec) >= RouterOffDuration)
  {
    // Turn the router on
    ChangeState(RouterOn);  
  }
}

void Enter_State_RouterOn()
{
    // turn router on now
    // Relay is in NC mode. 
    // To turn it on, set the pin to low
    digitalWrite(RELAY_PIN, LOW);
}

void Process_State_RouterOn()
{
  // wait till router completely boots and the WiFi is ready
  long timeNow = millis();
  if((timeNow - stateStartMilliSec) >= WifiStartDuration)
  {
    // stop the speaker sound
    noTone(BUZZER_PIN);
    // now the router is really on and should be trasmitting.
    // connect and perform the check
    ConnectToWiFi();
    ChangeState(Checking);  
  }
}

void ChangeState(States newState)
{
  // call enter method as per state
    // Serial.print("Entering state: ");
    // Serial.println(newState);
    currentState = newState;
     switch(newState)
     {
        case Checking:
         Enter_State_Checking();
         break;

        case Connected:
         Enter_State_Connected();
         break;

        case Disconnected:
         Enter_State_Disconnected();
         break;

        case RouterOff:
         Enter_State_RouterOff();
         break;

        case RouterOn:
         Enter_State_RouterOn();
         break;
     }
}

// common method to connect to Wi-Fi
void ConnectToWiFi()
{
  // set up wifi
   WiFi.begin(networkName, wifiPass);

  // wait 10 seconds for connection
  delay(10000);  
}

