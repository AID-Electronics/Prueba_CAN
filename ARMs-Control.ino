////////////////////////////////////////////// PARTE DE MOTORES
#include <mcp_can.h>
#include <SPI.h>
#include "Schneider_LMD_P84.h"
#define ID_MOTOR_1 0x610
#define ID_MOTOR_2 0x611
//#define ID_MOTOR_3 0x612
//#define ID_MOTOR_4 0x613

///////////////////////////////////////////////////////////////////////////// PARTE DE LA IMU
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <utility/imumaths.h>
                                                        // DATOS FIJOS DEL SISTEMAS DE POLEAS Y ACTUADORES
#define RESOLUCION 0.00703125  //GRADOS POR PASO
#define RADIO_POLEA 25 //mm
#define ALTURA_POLEAS 360 //mm
#define D_REF 333//mm
#define DIST 50
#define H 360
#define TOL 0.035 

double cabeceo;
double alabeo;

  long pasosMotor1;
  long pasosMotor2;
  long pasosMotor3;
  long pasosMotor4;
  unsigned long t=0;
  
 const double pi=3.141592;
 double deg2rad= pi/180;
///////////////////////////////////////////////////////////////////////////////////


Adafruit_BNO055 bno = Adafruit_BNO055(55);
void displayCalStatus (){
  /* Get the four calibration values (0..3) */
  /* Any sensor data reporting 0 should be ignored, */
  /* 3 means 'fully calibrated" */
  uint8_t sys, gyro, accel, mag;
  sys = gyro = accel = mag = 0;
  bno.getCalibration(&sys, &gyro, &accel, &mag);
 
  /* The data should be ignored until the sys calibration is > 0 */
  Serial.print("\t");
  if (sys==0){
    Serial.print("! ");
  }
 
  /* Display the individual values */
  Serial.print("Sys:");
  Serial.print(sys, DEC);
  Serial.print(" G:");
  Serial.print(gyro, DEC);
  Serial.print(" A:");
  Serial.print(accel, DEC);
  Serial.print(" M:");
  Serial.println(mag, DEC);
}


int calcularPasos1D(double cabeceo,double resolucion,double radioPolea,double distCentro)
{ 
  double PASOS;
  double tangente= tan(cabeceo);
  //PASOS=(((tangente*distCentro)/(2*pi*radioPolea))*(360/resolucion)) ;
  PASOS=51200*cabeceo/(2*pi);
  
  int aux=int(PASOS);
  double aux2=abs(PASOS)-abs(aux);
    
  if(abs(aux2)>0.5)
    {
      if (aux>0)
      aux++;
      else
      aux--;
    }
   return aux;
  
}

long calcularPasos2D(double cabeceo,double alabeo ,double resolucion,double radioPolea,double h,double posX, double posY,double Dref)
{
    double PASOS;
  double tangenteCAB= tan(cabeceo);  //ES EL ANGULO RESPECTO EL EJE X
  double tangenteAL= tan(alabeo); //ES EL ANGULO RESPECTO EL EJE Y
  double numerador= (sqrt((tangenteCAB*h-posY)*(tangenteCAB*h-posY)+(tangenteAL*h-posX)*(tangenteAL*h-posX))-Dref);
  
  PASOS=((numerador/(2*pi*radioPolea))*(360.0/resolucion)) ;

  
  long aux=(long)PASOS;
  double aux2=PASOS-aux;

    if(abs(aux2)>0.5)
    {
      if (aux>0)
      aux++;
      else
      aux--;
    }
    
  return aux;
   
}
void setup(){

  ////////////////////////////////////////////////////////////////////IMU
    Serial.begin(1000000);

    if(!bno.begin())
  {
    Serial.print("BNO055 no detectado");
    while(1);
  }

  delay(1000);

  bno.setExtCrystalUse(true);
    delay(1000);
  displayCalStatus();
//////////////////////////////////////////////////////////////////// CAN BUS

    while (CAN_OK != CAN.begin(CAN_1000KBPS))  {            // init can bus : baudrate = 1000k
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(500);
    }
    Serial.println("CAN BUS Shield init ok!");
    Serial.println();

    delay(200);
  
    setupMotor(ID_MOTOR_1,100000,100000,100,51200); //(long ID_motor,uint32_t Acel,uint32_t Decel, int current ,uint32_t MaxVel )
    setupMotor(ID_MOTOR_2,100000,100000,100,51200);
}


void imprimirDatos(sensors_event_t event){

  //Serial.print ("X: ");
  Serial.print (event.orientation.z,4);
  Serial.print (" ");
  //Serial.print ("\tY: ");
  Serial.print (event.orientation.y,4);  
  Serial.print (" ");
  //Serial.print ("\tZ: ");
  //Serial.println (event.orientation.z,4);


  //Serial.print ("Pasos motor 1: ");
  Serial.print (pasosMotor1);
  Serial.print (" ");
 // Serial.print ("        Pasos motor 2: ");
  Serial.print (pasosMotor2);
  Serial.print (" ");
  //Serial.print ("      Pasos motor 3: ");
  Serial.print (pasosMotor3);
  Serial.print (" ");
  //Serial.print ("    Pasos motor 4: ");
  Serial.print (pasosMotor4);
  Serial.println(" ");
  //delay (2);
  
}


void moverMotores() {
    
      pasosMotor1=calcularPasos2D(cabeceo,alabeo,RESOLUCION,RADIO_POLEA,H,333,0,D_REF);
      pasosMotor2=calcularPasos2D(cabeceo,alabeo,RESOLUCION,RADIO_POLEA,H,0,333,D_REF);
      pasosMotor3=calcularPasos2D(cabeceo,alabeo,RESOLUCION,RADIO_POLEA,H,-333,0,D_REF);
      pasosMotor4=calcularPasos2D(cabeceo,alabeo,RESOLUCION,RADIO_POLEA,H,0,-333,D_REF);
 
     // pasosMotor1=calcularPasos1D(cabeceo,RESOLUCION,RADIO_POLEA,H);
     // pasosMotor2=calcularPasos1D(alabeo,RESOLUCION,RADIO_POLEA,H);

      //AQUI iría la accion de movimiento
      mover(pasosMotor1,ID_MOTOR_1);
      mover(pasosMotor2,ID_MOTOR_2);
     //mover(pasosMotor3,ID_MOTOR_3);
     //mover(pasosMotor4,ID_MOTOR_4);
  }

void loop(){
      //Serial.print("Micros: ");
      //Serial.println(micros()-t);
  sensors_event_t event;
  bno.getEvent (&event);
  if(Serial.read()!=-1){
    imprimirDatos(event);
     displayCalStatus();

   // mover(0,ID_MOTOR_1);
   // mover(0,ID_MOTOR_2);
    }   

    cabeceo=event.orientation.y*deg2rad; //No estoy demasiado seguro de que sea el eje correcto
    alabeo=event.orientation.z*deg2rad;
  
     moverMotores();
  t=micros();
}

