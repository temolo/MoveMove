/* starts from comboDump.ino Example sketch for IRLib2
    Illustrate how to create a custom decoder using only the protocols
    you wish to use.
    reviewed by usage only with NEC by APAG January 2020
    deep review of the original comboDump. ino example to add remote control pushed buttons pressed
    added rgb led june 2020

*/

//ipotesi 70 M4 e 77 M3 per compensare il fatto che a parità di duty M3 gira piu lento

//adafruit motor shield v1
//versione successiva con sensore infrarossi e montata su carro ruote lego
//prova con IRLib2 dopo che si è capito che con la libreria originaria i motori non si attivano una volta iniziato il processo di ricezione del segnale IR
/*
   All 6 analog input pins are available. They can also be used as digital pins (pins #14 thru 19)
  Digital pin 2, and 13 are not used.
  https://learn.adafruit.com/adafruit-motor-shield/faq
*/
//utilizzo di A0 e successivi analog pin come digitali A0-->14 e gli altri a seguire
//**** remote controller buttons documentation
// 1 Y output -->brown cable
// 2 R vcc 3.3 5 volt -->red cable
// 3 G gnd ground -->black cable
// on off --> FFA25D --> 16753245
//****tasti telecomando arduino nec generico
// MENU --> FFE21D --> 16769565
// TEST --> FF22DD --> 16720605
// + --> FF02FD --> 16712445
// freccia back --> FFC23D --> 16761405
// FrecceSx --> FFE01F --> 16769055
// FrecciaPlay --> FFA857 --> 16754775
// FrecceDx --> FF906F --> 16748655
// 0 --> FF6897 --> 16738455
// - --> FF9867 --> 16750695
// C --> FFB04F --> 16756815
// 1 --> FF30CF --> 16724175
// 2 --> FF18E7 --> 16718055
// 3 --> FF7A85 --> 16743045
// 4 --> FF10EF --> 16716015
// 7 --> FF42BD --> 16728765
// 8 --> FF4AB5 --> 16730805
// 9 --> FF52AD --> 16732845
// *********tasti telecomando LG blue ray disc player con protocollo NECx
// ENTER --> B4B41AE5 --> 3031702245
// FrecciaSX -->B4B49A65 --> 3031734885
// FrecciaDX --> B4B45AA5 --> 3031718565
// FrecciaUP --> B4B4E21D --> 3031753245
// FrecciaDOWN --> B4B412ED --> 3031700205
// PLAY --> B4B48C73 --> 3031731315
// STOP --> B4B49C63 --> 3031735395
// DoppiaSX --> B4B44CB3 --> 3031714995
// DoppiaDX --> B4B4CC33 --> 3031747635
// 1 --> B4B4DC23 --> 3031751715
// 2 --> B4B43CC3 --> 3031710915
// 4 --> B4B47C83 --> 3031727235
// 5 --> B4B4FC03 --> 3031759875
//*** end of remote controller buttons documentation
/*
  RGB color documentation
  RGB_color(255, 0, 0); // Red
  RGB_color(0, 255, 0); // Green
  RGB_color(0, 0, 255); // Blue
  RGB_color(255, 255, 125); // Raspberry
  RGB_color(0, 255, 255); // Cyan
  RGB_color(255, 0, 255); // Magenta
  RGB_color(255, 255, 0); // Yellow
  RGB_color(255, 255, 255); // White
  RGB_color(0, 255, 150); // Acquamarine
*/

// **** InfraredReceiver ****
#include <IRLibDecodeBase.h> // First include the decode base
//#include <IRLib_P01_NEC.h>   // con telecomando da kit arduino
#include <IRLib_P07_NECx.h> //con telecomando nuovo LG del lettore blueray

// Now declare an instance of that decoder.
IRdecodeNECx myDecoder; //with the #include <IRLibCombo.h> here IRdecode MyDecoder was used

// Include a receiver either this following  or IRLibRecvPCI or IRLibRecvLoop
//#include <IRLibRecv.h> //this looks for inputs every 50 ms
//IRrecv myReceiver(2);  //pin number for the receiver

#include <IRLibRecvPCI.h> //this looks for inputs every time an input signal is on the myReceiver(pin number)
IRrecvPCI myReceiver(2);  //pin number for the receiver

String  ReceivedValue; //set a variable to store received value. On this variable the controlling logic should be set
String PreviousReceivedValue;//set a variable to store the previously received value. When a value is repeated, this value is put back to the output process
String DriveDirection; //set a variable to define either forward or backward as drive direction

// #define statements have to be put before setup apparentely
// a "0x"  has to be put before the exadecimal code the receiver actually gets
//definition for LG BLU RAY Disc Player Remote control
#define MYPROTOCOL NEC
#define PLAY 0xB4B48C73 // mappato PLAY
#define STOP 0xB4B49C63 //mappato STOP
#define FrecciaUP 0xB4B4E21D //mappato FrecciaUP per andare avanti (suggerimento Margherita)
#define FrecciaDOWN 0xB4B412ED //mappato FrecciaDOWN per andare indietro (suggerimento Margherita)
#define FrecciaSX 0xB4B49A65 //mappato FrecciaSX per andare a sinistra
#define FrecciaDX 0xB4B45AA5 //mappato FrecciaDX per andare a destra
#define ENTER 0xB4B41AE5 //mappato ENTER per riportare le ruote alla stessa velocità e quindi procedere dritto
#define DoppiaSX 0xB4B44CB3 //mappato DoppiaSX per aumentare velocità
#define DoppiaDX 0xB4B4CC33 //mappato DoppiaDX per diminuire velocità
#define Uno 0xB4B4DC23 //1 e 2 per controllo puntuale velocità motore 3 --vel
#define Due 0xB4B43CC3 //1 e 2 per controllo puntuale velocità motore 3 ++vel
#define Quattro 0xB4B47C83 //4 e 5 per controllo puntuale velocità motore 4 --vel
#define Cinque 0xB4B4FC03 //4 e 5 per controllo puntuale velocità motore 4 ++vel
#define PREVIOUS 0xFFFFFFFF

//#define ON 0xFFA25D
//#define MENU 0xFFE21D
//#define TEST 0xFF22DD
//#define PLUS 0xFF02FD
//#define PREVIOUS 0xFFFFFFFF

// **** motor shield ****
#include <AFMotor.h>
//crea i motori 3 e 4 che pare non siano in conflitto con la libreria IRRemote
//punto che era emerso dalla lettura di documentazione varia
AF_DCMotor motor3(3, MOTOR34_64KHZ); // create motor #3, 64KHz pwm
AF_DCMotor motor4(4, MOTOR34_64KHZ); // create motor #4, 64KHz pwm
static int dutyM3 = 0; //Variable to change the motor speed and direction
static int dutyM4 = 0; //Variable to change the motor speed and direction

// **** RGB Led ****
#define REDLIGHTPIN 11
#define GREENLIGHTPIN 10
#define BLUELIGHTPIN 9
// **** End RGB Led ****

void setup() {
  DriveDirection = "FORWARD"; //as initial setup the way is forward
  Serial.begin(9600);
  delay(1000); while (!Serial); //delay for Arduino

  Serial.println("Motor test!");
  dutyM3 = 110;
  dutyM4 = 90;
  motor3.setSpeed(dutyM3);
  motor4.setSpeed(dutyM4);
  motor3.run(FORWARD);
  motor4.run(FORWARD);

  myReceiver.enableIRIn(); // Start the receiver here and not at the beginning of loop()

  Serial.println(F("Ready to receive IR signals"));
  //delay (2000); //delay command is key to have the motor running while delaying

  // **** RGB Led ****
  pinMode(REDLIGHTPIN, OUTPUT);
  pinMode(GREENLIGHTPIN, OUTPUT);
  pinMode(BLUELIGHTPIN, OUTPUT);
  // **** End RGB Led ****
}

void loop() {
  //Continue looping until you get a complete signal received
  if (myReceiver.getResults()) {
    if (myDecoder.decode()) {
      switch (myDecoder.value) {

        case PLAY:
          ReceivedValue = "PLAY";
          Serial.println(ReceivedValue);
          dutyM3 = 110; //start with low speed since M4 seems stronger than M3
          dutyM4 = 90; //start with low speed
          motor3.setSpeed(dutyM3); // set the speed to 35 ok per ruota di newton 200 ok trasmissione del moto due ruote e elastico
          motor4.setSpeed(dutyM4);
          motor3.run(FORWARD);
          motor4.run(FORWARD);
          RGB_color(0, 255, 0); // Green
          break;

        case STOP:
          ReceivedValue = "STOP";
          Serial.println(ReceivedValue);
          motor3.run(RELEASE); // stopped
          motor4.run(RELEASE); // stopped
          RGB_color(0, 0, 255); // Blue

          break;

        case DoppiaDX:
          ReceivedValue = "DoppiaDX";
          Serial.println(ReceivedValue);
          dutyM3 = dutyM3 + 5;
          dutyM4 = dutyM4 + 5;
          motor3.setSpeed(dutyM3);
          motor4.setSpeed(dutyM4);
          if (DriveDirection == "FORWARD") {
            motor3.run(FORWARD);
            motor4.run(FORWARD);
          }
          if (DriveDirection == "BACKWARD") {
            motor3.run(BACKWARD);
            motor4.run(BACKWARD);
          }
          RGB_color(255, 255, 125); // Raspberry
          break;

        case DoppiaSX:
          ReceivedValue = "DoppiaSX";
          Serial.println(ReceivedValue);
          dutyM3 = dutyM3 - 5;
          dutyM4 = dutyM4 - 5;
          motor3.setSpeed(dutyM3);
          motor4.setSpeed(dutyM4);
          if (DriveDirection == "FORWARD") {
            motor3.run(FORWARD);
            motor4.run(FORWARD);
          }
          if (DriveDirection == "BACKWARD") {
            motor3.run(BACKWARD);
            motor4.run(BACKWARD);
          }
          RGB_color(255, 255, 0); // Yellow
          break;

        case FrecciaSX:
          ReceivedValue = "FrecciaSX";
          Serial.println(ReceivedValue);
          for (int k = 0; k < 5; k++) {
            dutyM3 = dutyM3 - 1;
            dutyM4 = dutyM4 + 1;
            motor3.setSpeed(dutyM3);
            motor4.setSpeed(dutyM4);
            if (DriveDirection == "FORWARD") {
              motor3.run(FORWARD);
              motor4.run(FORWARD);
            }
            if (DriveDirection == "BACKWARD") {
              motor3.run(BACKWARD);
              motor4.run(BACKWARD);
            }
          } //end of "for" repetition for 5 times
          break;

        case FrecciaDX:
          ReceivedValue = "FrecciaDX";
          Serial.println(ReceivedValue);
          for (int k = 0; k < 5; k++) {
            dutyM3 = dutyM3 + 1;
            dutyM4 = dutyM4 - 1;
            motor3.setSpeed(dutyM3);
            motor4.setSpeed(dutyM4);
            if (DriveDirection == "FORWARD") {
              motor3.run(FORWARD);
              motor4.run(FORWARD);
            }
            if (DriveDirection == "BACKWARD") {
              motor3.run(BACKWARD);
              motor4.run(BACKWARD);
            }
          } //end of "for" repetition for 5 times
          break;

        case ENTER:
          ReceivedValue = "ENTER";
          Serial.println(ReceivedValue);
          dutyM3 = dutyM4 - 7; //same speed each wheel to go straight
          motor3.setSpeed(dutyM3);
          motor4.setSpeed(dutyM4);
          if (DriveDirection == "FORWARD") {
            motor3.run(FORWARD);
            motor4.run(FORWARD);
          }
          if (DriveDirection == "BACKWARD") {
            motor3.run(BACKWARD);
            motor4.run(BACKWARD);
          }
          break;

        case Uno:
          ReceivedValue = "Uno";
          Serial.println(ReceivedValue);
          dutyM3 = dutyM3 - 1; //decreases only M3 speed
          motor3.setSpeed(dutyM3);
          motor4.setSpeed(dutyM4);
          if (DriveDirection == "FORWARD") {
            motor3.run(FORWARD);
            motor4.run(FORWARD);
          }
          if (DriveDirection == "BACKWARD") {
            motor3.run(BACKWARD);
            motor4.run(BACKWARD);
          }
          break;

        case Due:
          ReceivedValue = "Due";
          Serial.println(ReceivedValue);
          dutyM3 = dutyM3 + 1; //increases only M3 speed
          motor3.setSpeed(dutyM3);
          motor4.setSpeed(dutyM4);
          if (DriveDirection == "FORWARD") {
            motor3.run(FORWARD);
            motor4.run(FORWARD);
          }
          if (DriveDirection == "BACKWARD") {
            motor3.run(BACKWARD);
            motor4.run(BACKWARD);
          }
          break;

        case Quattro:
          ReceivedValue = "Quattro";
          Serial.println(ReceivedValue);
          dutyM4 = dutyM4 - 1; //decreases only M4 speed
          motor3.setSpeed(dutyM3);
          motor4.setSpeed(dutyM4);
          if (DriveDirection == "FORWARD") {
            motor3.run(FORWARD);
            motor4.run(FORWARD);
          }
          if (DriveDirection == "BACKWARD") {
            motor3.run(BACKWARD);
            motor4.run(BACKWARD);
          }
          break;

        case Cinque:
          ReceivedValue = "Cinque";
          Serial.println(ReceivedValue);
          dutyM4 = dutyM4 + 1; //increases only M4 speed
          motor3.setSpeed(dutyM3);
          motor4.setSpeed(dutyM4);
          if (DriveDirection == "FORWARD") {
            motor3.run(FORWARD);
            motor4.run(FORWARD);
          }
          if (DriveDirection == "BACKWARD") {
            motor3.run(BACKWARD);
            motor4.run(BACKWARD);
          }
          break;

        case FrecciaUP:
          ReceivedValue = "FrecciaUP";
          Serial.println(ReceivedValue);
          DriveDirection = "FORWARD";
          motor3.run(FORWARD);
          motor4.run(FORWARD);
          RGB_color(0, 255, 0); // Green
          break;

        case FrecciaDOWN:
          ReceivedValue = "FrecciaDOWN";
          Serial.println(ReceivedValue);
          DriveDirection = "BACKWARD" ;
          motor3.run(BACKWARD);
          motor4.run(BACKWARD);
          RGB_color(255, 0, 0); // Red
          break;

        case PREVIOUS:
          ReceivedValue = PreviousReceivedValue;
          Serial.println(ReceivedValue); break;
          PreviousReceivedValue = ReceivedValue; //stores current value into previous in order to handle situations where there is a PREVIOUS signal coming from remote control

      } //end of switch case on a complete reception
    } //end of  if (myDecoder.decode()) {

    Serial.print("dutyM3= ");
    Serial.println(dutyM3);
    Serial.print("dutyM4= ");
    Serial.println(dutyM4);
    Serial.print("DriveDirection= ");
    Serial.println(DriveDirection);

    delay(30); //After testing, without delay every one press at least on gets lost. 20 ms is to small, 30 ms works fine without continous pressure. delay() seems a crucial parameter to be fine tuned
    myReceiver.enableIRIn();      //Restart receiver because after the receiver has detect that a complete code has been received, it goes into idle mode.
    //this command myReceiver.enableIRIn() has to be always placed before the parenthesis which closes the "if myReceiver.getResults() statement
  }//end of if myReceiver.getResults()
}
