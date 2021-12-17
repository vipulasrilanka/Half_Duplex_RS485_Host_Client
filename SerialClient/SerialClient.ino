/*
  Serial_bus_Client
  Sample implementation of a shared serial bus over RS485 on Arduino UNO
  Auther: vipula Liyanaarachchi
  Date: 15th Dec 2021
  All rights reserved.

*/

#define RS485_DRIVER_EN 3

const int SERIAL_READ_TIMEOUT_MS=100;
const int SERIAL_BAUD_RATE=9600;

const int BUFFER_SIZE = 35;
char buf[BUFFER_SIZE];

// the setup function runs once when you press reset or power the board
void setup() {

  // initialize digital pin LED_BUILTIN as an output, and turn it off
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  //Configure the RS485 Driver Controller, and keep it disabled.
  pinMode(RS485_DRIVER_EN,OUTPUT);
  digitalWrite(RS485_DRIVER_EN,LOW);

  //initialize the Serial port that's connected to the IDE via USB.
  //This is a Full duplex URAT, so we need to use this for the
  //main communication.
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.setTimeout(SERIAL_READ_TIMEOUT_MS); //set read time out to 100ms
  //We never get a full size packet so most of the time we time out at read.
  //Therefore the client timeout has to be lower than the HOST read time out
  //We are the client, so we can not Tx anything now.

  //Blink the LED for bebugging
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on (HIGH is the voltage level)
  delay(200);                         // wait..
  digitalWrite(LED_BUILTIN, LOW);     // turn the LED off by making the voltage LOW
}

// the loop function runs over and over again forever
void loop() {

  //wait for the host.
  int rlen = Serial.readBytes(buf, BUFFER_SIZE);
  //We always return after the time out, as the BUFFER_SIZE is
  //larger than the host packet.

  if (rlen > 0) {

    //Validate the data packet here.

    //We need to give some time for the Host to disable the Driver before we can enable it.

    //Here we do this with a delay, and an LED blink patern
    digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on (HIGH is the voltage level)
    delay(100);                         // This delay is not a random number.

    //Enable the Driver for transmission (Responce)
    digitalWrite(RS485_DRIVER_EN,HIGH);

    //Packet Format
    //There is one Host in the system, with multiple clients, so each packet has to have the client ID.
    //Client ID starts from 1. (Zero is reserved for Host)
    //<OwnerType:1><:><ClientID:1><:><SeesionID:1><:><ComType:1><:><DataLength:1><:><Data:DataLength><:E><CRC:2>
    //Sample 1 => H:x:y:R:z::E:cc => From Host, Read request to client x. No data
    //Sample 2 => C:x:y:W:z:ACK:E:cc => Client responce, from clinet x, write request to host. z = 3, Massage = "ACK".


    //This is not the corrrect way to construct the dataBlock., just for debugging
    //The length of this data chunk has to be smaller than BUFFER_SIZE.
    Serial.write("C:"); //From Client
    Serial.write(0x01); //Clinet 01
    Serial.write(":");  //Seperator
    Serial.write(0x00); //Session ID, static. This has to be derived from the Host call.
    Serial.write(":W:");  //Seperator, Write to host
    Serial.write(0x03); //Data Length 03
    Serial.write("ACK:E:");  //Responce, ACK, Seperator, EndCode
    Serial.write(0x43); //Dummy CRC
    Serial.write(0x0A); //End of line

    //Here we have to wait for the Tx to complete before disabling the driver.
    //This has to be done with Serial.availableForWrite() or some other suitable function.
    delay(50); // This delay is not a random number.
    //Disable the Driver for transmission
    digitalWrite(RS485_DRIVER_EN,LOW);

    //Here we add a delay for the LED blink patern (for Debug only)
    delay(100);                         // This delay is not a random number.
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

  }
  else {
  //Time out error via the LED
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on (HIGH is the voltage level)
  delay(30);                         // wait..
  digitalWrite(LED_BUILTIN, LOW);     // turn the LED off by making the voltage LOW
  delay(60);                         // wait..
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on (HIGH is the voltage level)
  delay(30);                         // wait..
  digitalWrite(LED_BUILTIN, LOW);     // turn the LED off by making the voltage LOW
  }


}
