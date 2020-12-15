/*
   MKV  has no webserver on it    11/12/2020


*/
#include <WebSocketsServer.h>
#include <LedWrite.h>
#define USE_SERIAL Serial

const int LED_DATA_PIN = 2;
WebSocketsServer webSocket = WebSocketsServer(81);
LedWrite leds = LedWrite(LED_DATA_PIN);

IPAddress ip(192, 168, 1, 220 ); // where xx is the desired IP Address
IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask

const char *ssid = "SSE Broadband BAF467";
const char *password = "q2z7Mb2R42Z2PLET";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int prevMillis; 

int MODE_SET_COUNTER = 0;
bool MODE_CHANGE = false; 
bool SHUFFLE_MODE = false;
bool RANDOM_MODE = false;

byte clientCount = 0;
bool CN_Flag = false;  // this flag is set on new websocket connection
int mode_selection = 0; //  selection of current sequence  (start on  white colour changing )

/*
   randColor1  and  randColor2  these globals store 2 colours
   they are used for the twinkle , colour phasing , swipe and LED chase effect methods of the LedWrite class
*/
byte randColor1, randColor2;
int chase_dir = 1;

bool PLAY_FLAG = true;

// setFlag is used by any part of the program when a physical LED refresh is required
bool SET_FLAG = false;

// it is possible to access the LED data array
//RGB* led = leds.led;
RGB* color = leds.getColor();

//////////////////////////////////////////////////////////////////////////////////
//  this method is called when there is text from the web socket connection   ////
//////////////////////////////////////////////////////////////////////////////////
void getTextData(String text)
{
  String msg = text.substring(0, 1);
  RANDOM_MODE = false; SHUFFLE_MODE = false;

  if (msg == "B") {       //if the message had the word 'Button'   etc.......

    MODE_CHANGE = true;

    int button = text.substring(1).toInt();            // button number pressed
    switch (button)
    {
      case 7:
        {
          mode_selection -= 1;
          if (mode_selection < 0) mode_selection = 18;
          break;
        }
      case 8:
        {
          mode_selection += 1;
          if (mode_selection > 18) mode_selection = 0;
          break;
        }
      case 11:
        { //   SHUFFLE
          SHUFFLE_MODE = true;
          break;
        }
      case 12:
        { // RANDOM
          RANDOM_MODE = true;
          break;
        }
      default:
        {
          mode_selection = button;
          break;
        }
    }
    String modeString = "Mode: " + String(mode_selection);
    if (RANDOM_MODE) modeString = "Random";
    if (SHUFFLE_MODE) modeString = "Shuffle";
    webSocket.broadcastTXT(modeString);
  }
}
//////////////////////////////////////////////////////////////////////////
//       this method occurs when there is a web socket connection     ////
//////////////////////////////////////////////////////////////////////////
void doConnectedThings(uint8_t num)
{
  IPAddress ip = webSocket.remoteIP(num);
  PLAY_FLAG = true;

  USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d \n", num, ip[0], ip[1], ip[2], ip[3]);
  CN_Flag = true;
  clientCount = num;

  leds.setColor(color[green]);
  SET_FLAG = true;
}
//==========================================================================================================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type)
  {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED:
      doConnectedThings(num);
      break;

    case WStype_TEXT:
      getTextData( ((char *)&payload[0]) );
      break;

    case WStype_BIN:
      hexdump(payload, length);
      break;
  }
}

//**************************************************************************************************************************************************************
void doSequence()    //  using choice GLOBAL variable  this updates or advances the current sequence
{

  switch (mode_selection) {

    case 0: leds.colorChange(color[white]);
      break;
    case 1: leds.colorChange(color[red]);
      break;
    case 2: leds.colorChange(color[green]);
      break;
    case 3: leds.colorChange(color[blue]);
      break;
    case 4: leds.colorChange(color[yellow]);
      break;
    case 5: leds.colorChange(color[indigo]);
      break;
    case 6: leds.colorChange(color[violet]);
      break;
    case 7:
      {
        leds.colorChange(color[white]);
        leds.chaseEffect(chase_dir, randColor1);
      }
      break;
    case 8: leds.shift(chase_dir);     // fill alternate
      break;
    case 9: leds.shift(chase_dir);     // wave effect
      break;
    case 10: leds.shift(chase_dir);     // ranbow
      break;
    case 11: leds.shift(chase_dir);     // SAME as 10
      break;
    case 12: leds.twinkle(color[yellow], color[blue]);         //yellow on blue
      break;
    case 13: leds.twinkle(color[randColor1], color[randColor2]);
      break;
    case 14: leds.shift(chase_dir);     //  rainbow
      break;
    case 15:
      {
        leds.twinkle(color[random(13)], color[none]);//   super strobe   ****************
      }
      break;
    case 16: leds.phasing(color[randColor1], color[randColor2]);
      break;
    case 17: leds.snowfall(4);
      break;
    case 18: leds.swipe(color[randColor1], color[randColor2]);
      break;
  }
  SET_FLAG = true;
}

//////////////////////////////////////////////////////////
//          Mode eight is two coloured chaser          //
////////////////////////////////////////////////////////
void modeEight()
{
  // pick two random colours
  randColor1 = random(7); randColor2 = random(7);

  // set one of them white if they are the same
  if (randColor1 == randColor2) randColor1 = white;

  //  fill LED array with two alternate colors ,  random spacing
  leds.fillAlternate(random(4, 8), color[randColor1], color[randColor2]);       //  0 to 7 are rainbow  colours   32     params: mod, color 1, color 2

}
//////////////////////////////////////////////////////////
//          Mode nine is sine wave effect         //
////////////////////////////////////////////////////////
void modeNine()
{
  randColor1 = random(7);
  // sine wave effect is actually modified to resemble a softened triangle using the 3rd harmonic of a cosine wave
  leds.wave(randColor1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//          every now and then a sequence will be selected                  //
//    when a sequence is selected the LED data array is filled with         //
//           data to determine  the color and brightness of each LED        //
//////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setSequence()   //  uses the above modes
{
  chase_dir = random(2);
  switch (mode_selection) {
    case 7:
      randColor1 = random(7);
      break;

    case 8:
      modeEight(); //Mode eight is a two coloured chaser // fill alternate
      break;

    case 9:
      modeNine();// sine wave effect     actually modified to resemble a softened triangle using the 3rd harmonic of a cosine wave
      break;

    case 10:
      leds.fillRandom();
      break;

    case 11:
      leds.fillRandom();
      break;

    case 12:
      leds.spread = random(0, 2);  //twinkle   //yellow on blue
      break;

    case 13:      // twinkle random
      {
        randColor1 = random(7); randColor2 = random(7);
        leds.spread = random(0, 2);
        break;
      }
    case 14:
      leds.fillRainbow();
      break;

    case 15:
      leds.spread = random(0, 2);
      break;

    case 16:
      randColor1 = random(7); randColor2 = random(7);// the brightness of every two alternate leds will have brightness modulated by a triangular waveform
      break;

    case 17:    //snow
      leds.setColor(color[none]);
      break;

    case 18:  //swipe
      randColor1 = random(7); randColor2 = random(7);
      break;
  }
}

//**************************************************************************************************************************************************************
void PINsetup()
{
  pinMode(LED_DATA_PIN, OUTPUT);
  digitalWrite(LED_DATA_PIN, HIGH);    // that's the RX pin  pinMode(LED_PIN, OUTPUT);  //gpio 2

}

//**************************************************************************************************************************************************************
void setup() {

  USE_SERIAL.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  PINsetup();

  // Connect to WiFi network
  // the ip address is fixed along with the webSocket address in the JavaScript in the webpage
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // USE_SERIAL.print(".");
  }

  /*
    USE_SERIAL.println("");
    USE_SERIAL.print("Connected to ");
    USE_SERIAL.println(ssid);
    USE_SERIAL.print("IP address: ");
    USE_SERIAL.println(WiFi.localIP());
  */

  // Start TCP (WS) server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  clientCount = 0;
  leds.setColor(color[none]);

  SET_FLAG = true;
  leds.dimColors();

  prevMillis = millis();
}

//******************************************************************************************************************************************************************8
void  loop ()                                           // keep this loop lean //
{
  webSocket.loop();// service the websocket

  // one tenth of a second events - stuff happens with a kind of semaphore system
  if (millis() > prevMillis + 100)   // 100 makes this whole thing one tenth of a second
  {
    prevMillis = millis();
    if (RANDOM_MODE || SHUFFLE_MODE) // if random mode is selected then increment the counter
    { // if the counter times out then a MODE CHANGE will be triggered and the counter will be reset
      MODE_SET_COUNTER++;
      if (MODE_SET_COUNTER > 50)  // 50 gives 5 seconds so the counter times out and a MODE change will happen
      {
        MODE_SET_COUNTER = 0;
        if (!SHUFFLE_MODE) mode_selection = random(7, 18);  //  MODE change may only shuffle the mode with new random values
        MODE_CHANGE = true;
      }
    }

    delay(1);
    if (MODE_CHANGE) {
      MODE_CHANGE = false;
      setSequence();
    } else if (PLAY_FLAG) doSequence();

    delay(1);
    if (SET_FLAG) {  // SET_FLAG can be used by any part of the program when a physical LED refresh is required
      SET_FLAG = false;
      leds.refresh();
    }
  }//end of one nth of a second events

}
