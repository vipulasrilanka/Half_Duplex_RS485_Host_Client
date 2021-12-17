/*
  Serial_bus_HOST
  Sample implementation of a shared serial bus over RS485 on ESP8266MOD
  Using modude: Nodemcu Amica (NodeMCU 1.0 ESP-12E)
  Auther: vipula Liyanaarachchi
  Date: 14th Dec 2021
  All rights reserved.

*/

#define RS485_DRIVER_EN D1

const int SERIAL_READ_TIMEOUT_MS=500;
const int SERIAL_BAUD_RATE=9600;

const int BUFFER_SIZE = 35;
char buf[BUFFER_SIZE];

unsigned int session_id = 0;
const int  SESSION_ID_MAX = 1023;

// the setup function runs once when you press reset or power the board
void setup() {

  // initialize digital pin built in LED 2 as an output.
  //This LED is active low, so set HIGH to off
  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);

  //Configure the RS485 Driver Controller, and keep it disabled.
  pinMode(RS485_DRIVER_EN,OUTPUT);
  digitalWrite(RS485_DRIVER_EN,LOW);

  //initialize the Serial port that's connected to the IDE via USB.
  //This is a Full duplex URAT, so we need to use this for the
  //main communication.
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.setTimeout(SERIAL_READ_TIMEOUT_MS); //set read time out to 500ms
  Serial.println("I:0000"); //Init BUS call..
  //We can use above to negotiate the Max packet size, Serial Time out..etc
  //They are not implemented here.

  //initialize the second, built in H/W UART.
  //This is a Tx only UART over D4
  //We will be using this to get debug messages out.
  Serial1.begin(19200); // open the port at 19200 baud, this is fast enough for debug
  Serial1.println("Debug Port");
}

// the loop function runs over and over again forever
void loop() {

  //Add some artificial delay.. this is not needed for the final implementation.
  //Add a blink patern for debugging
  digitalWrite(D0, LOW);   // turn the LED on (LED2 is active low)
  delay(200);              // wait for 100ms
  digitalWrite(D0, HIGH);  // turn the LED off 
  delay(100);              // wait for 100ms
  digitalWrite(D0, LOW);   // turn the LED on
  delay(700);              // wait for 800ms

  //Data Packet Format
  //There is one Host in the system, with multiple clients, so each packet has to have the client ID.
  //Client ID starts from 1. (Zero is reserved for Host)
  //<OwnerType:1><:><ClientID:1><:><SeesionID:1><:><ComType:1><:><DataLength:1><:><Data:DataLength><:E><CRC:2>
  //Sample 1 => H:x:y:R:z::E:cc => From Host, Read request to client x. No data
  //Sample 2 => C:x:y:W:z:ACK:E:cc => Client responce, from clinet x, write request to host. z = 3, Massage = "ACK".
  //Total length of the data packet has to be less than BUFFER_SIZE

  //Enable the Driver for transmission (Responce)
  digitalWrite(RS485_DRIVER_EN,HIGH);

  //This is not the ideal way to construct the dataBlock., just for debugging
  //The length of this data chunk has to be smaller than BUFFER_SIZE.
  Serial.write("H:"); //From Hose
  Serial.write(0x01); //To Clinet 01
  Serial.write(":");  //Seperator
  Serial.write(session_id); //Session ID
  Serial.write(":R:");  //Seperator, Read type
  Serial.write(0x00); //Data Length 0
  Serial.write(":E:");  //Seperator, EndCode
  Serial.write(0x35); //Dummy CRC
  Serial.write(0x0A); //End of line

  //Here we have to wait for the Tx to complete before disabling the driver.
  //This has to be done with Serial.availableForWrite() or some other suitable function.
  //Here we do this with a delay (for Debug only)
  delay(50); // This delay is not a random number, and carefullly chosen.
  //Disable the Driver so that the client can transmit.
  digitalWrite(RS485_DRIVER_EN,LOW);

  //Now we wait for the responce, with a time out of SERIAL_READ_TIMEOUT_MS
  int rlen = Serial.readBytes(buf, BUFFER_SIZE);

  if (rlen > 0) {
    //We come here if there is a response from Client 01
    // prints the received data
    Serial1.print("Received <");
    for(int i = 0; i < rlen; i++) {
      Serial1.print(buf[i]);
    }
    Serial1.println(">");

    //Validate the data packet here.
    //and take any action needed.
  }
  else {
    //Serial Time out, no responce from Client 01
    Serial1.print("Session <");
    Serial1.print(session_id,HEX); //Sesson ID
    Serial1.println("> TimeOut");
  }

  //Continue the LED blink patern for debugging. This adds an artificial delay 
  //So that we can read the debug data
  digitalWrite(D0, HIGH);  // turn the LED off 
  delay(1000);              // wait for 100ms

  //Increment session_id
  session_id = session_id +1 ;
  //Reset to Zero if it's over SESSION_ID_MAX
  if (session_id > SESSION_ID_MAX) session_id = 0;

}
