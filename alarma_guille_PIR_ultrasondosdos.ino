#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <NewPing.h>


#define busyPin A3 
#define LEDPin 13   
#define TRIGGER_PIN 4 // pin trigger del ultrasonic sensor.    
#define ECHO_PIN 6    // pin echo del ultrasonic sensor.
#define MAX_DISTANCE 75 // distancia maxima de deteccion en cm del ultrasonic sensor
#define intermitentePin 5// pin del flash intermitente
#define PIRPin 3  //pin del sensor PIR
#define flashPin A4// pin de salida para el flash cegador
#define sirenPin A5// pin de salida de la sirena acustica

                           //pines sensores antisabotaje: 
#define clickAPin 2//el pin 2 conectado con resistencia 330ohm a vcc.
                    //despues puentear con un cable el mismo pin a gnd,
                    //cuando se corta el cable (puente) se activa HIGH
//#define clickBPin 7//        "
//#define clickCPin 8//        "

           //        dfplayer reproductor mp3
           //conectar el rx al pin 8 con resistencia 1k , 
           //y el tx al pin 7  
           
 NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPin setup de pines del ultrasonic sensor y distancia maxima de deteccion

int mp3 = 0;
int i = 0;
int pirvalue;
int c = 0;
int sabotaje = LOW;  
DFRobotDFPlayerMini myDFPlayer;

void irpir(){ // funcion que lee el PIR y el ultrasonic sensor. si alguno esta activo devuelve high
 
         pirvalue= digitalRead(PIRPin); // lee el PIR
         unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).lee ultrasonic sensor
         if (uS / US_ROUNDTRIP_CM != 0 || pirvalue == HIGH) // si alguno de los dos sensores detecta...
         {
           pirvalue = HIGH; // pone activa la señal
           delay(2);
         }
 }
 
void intermitente() { // funcion que dispara la luz intermitente y la sirena acustica
  for (c=0; c<200; c++){ // "c" es el tiempo que dura la sirena y la luz
    irpir();
    click_antisabotaje();   
    if ((pirvalue == HIGH)||(sabotaje == HIGH)){
     int busyvalue = digitalRead(busyPin); //comprueba si esta sonando el mp3
     if (busyvalue == HIGH){
      c = 0;
    myDFPlayer.play(5); //play ultima pista
    sabotaje = LOW;
     }
  }
    digitalWrite(sirenPin, HIGH);// activa la sirena acustica
    digitalWrite(intermitentePin, HIGH);//doble flash
    delay(60);
    digitalWrite(intermitentePin, LOW);
    delay(75);
    digitalWrite(intermitentePin, HIGH);
    delay(40);
    digitalWrite(intermitentePin, LOW);
    delay(400);
    
  }
  digitalWrite(sirenPin, LOW);// apaga sirena
}
void playalarma() {  //funcion play mp3. cuando es llamada reproduce la pista almacenada en la variable "mp3".
  myDFPlayer.play(mp3);
  delay(100);
}

void deteccion() { //funcion deteccion. bucle de deteccion que activa los protocolos 
  for (long i=0; i<1000; i++) { //espera xxx tiempo para hacer el ciclo de protocolos. mas abajo el delay se multiplica por i
   click_antisabotaje ();// lee sensor antisabotaje
   irpir ();
 
  if ((pirvalue == HIGH)||(sabotaje == HIGH))  //si hay deteccion
  {
    int busyvalue = digitalRead(busyPin);// comprueba que no suene ninguna pista
    if (busyvalue == HIGH){ 
    mp3 = mp3 + 1;//numero de pista mp3
    }
    
    if (i >= 500)//si hay deteccion al final del tiempo de alarma 
    {  
      i=0;          //volvemos a reiniciar el tiempo de alarma
    }  
    if ((mp3 >= 4)||(sabotaje == HIGH))
    {
    myDFPlayer.volume(20); // volumen pista 4
    mp3 = 4;
    sabotaje = LOW;//apaga señal sabotaje
    }

    //int busyvalue= digitalRead(busyPin);
    if (busyvalue == HIGH){
    playalarma();//llama a la funcion play y reproduce la pista de la variable mp3
    delay (1300);//delay para el flash
    digitalWrite(flashPin, HIGH);//doble flash
    delay(40);
    digitalWrite(flashPin, LOW);
    delay(100);
    digitalWrite(flashPin, HIGH);
    delay(60);
    digitalWrite(flashPin, LOW);
       if ((mp3 >= 4)||(sabotaje == HIGH))
   {
     intermitente(); 
   } 
  }  
 }
    delay (70);// retardo que se multiplica por "i" (mas arriba)
  }
} 
void click_antisabotaje() {// funcion antisabotaje. se activa si se corta algun cable de los pines antisabotaje.
 int val1= digitalRead(clickAPin);
// int val2= digitalRead(clickBPin);
// int val3= digitalRead(clickCPin);
 delay(10);
   //if ((val1 || val2 || val3) == HIGH ){
    if (val1 == HIGH ){
   sabotaje = HIGH;// activa señal sabotaje 
  }
} 
void setup(){
    SoftwareSerial mySoftwareSerial(7, 8); // RX, TX
    mySoftwareSerial.begin(9600);
   (!myDFPlayer.begin(mySoftwareSerial));
    myDFPlayer.setTimeOut(350);
    myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  
  delay(1000); // tiempo de inicializacion de la alarma .espera 60segundos para inicializar sensores PIR y ultrasonic
               //y evitar falsos disparos al inicio
  pinMode(PIRPin, INPUT);
  pinMode(busyPin, INPUT);
  pinMode(clickAPin, INPUT);
 // pinMode(clickBPin, INPUT);
  //pinMode(clickCPin, INPUT);
  pinMode(LEDPin, OUTPUT);
  pinMode(sirenPin, OUTPUT);
  pinMode(flashPin, OUTPUT); 
  pinMode(intermitentePin, OUTPUT);
  
}
 
void loop(){
   mp3 = 0;
   myDFPlayer.volume(17);
   digitalWrite(sirenPin, LOW);// apaga sirena
   click_antisabotaje ();// lee sensor antisabotaje
   irpir(); // lee sensores PIR y ultrasonic

  if ((pirvalue == HIGH)||(sabotaje == HIGH))
  {
    deteccion();
  }
}
