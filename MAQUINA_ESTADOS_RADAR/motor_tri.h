#include <SPI.h>
#include <Controllino.h>

#define VEL_TO_RPM 1.25  // 1 entre 0.8m que tiene de radio



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

//                 Funciones para el movimiento del motor trifasico ///

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#define pinFrencuencia 10
#define pinMotor 6
#define MAX_VEL 24000
#define VEL_PREDET 1440




///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

class Motor_tri 
{
  float rpm;
  int pin;

  public:
  Motor_tri();
  Motor_tri(int a){pin=a;ini_motor();}
  void ini_motor();
  void moverMotor(float vel);
  void frenado();
  void frenadoGradual();
  
};


Motor_tri::Motor_tri(){

  pin=pinMotor;
  ini_motor();
  
}
void Motor_tri::ini_motor(){
  
  pinMode(pin,OUTPUT);

 
  
  
}

void Motor_tri::moverMotor(float vel){

  int velocidad=vel; //esta es la variable global en la que se almacena la velocidad del objetivo mas cercano.

  if(velocidad==0)
    velocidad=VEL_PREDET;

   velocidad=velocidad*VEL_TO_RPM; //ya que la velocidad medida por el radar es lineal y la salida del motor es velocidad angular.
   
   velocidad=map(velocidad,0,MAX_VEL,0,255);
  analogWrite(pinMotor,velocidad);
  
}


void Motor_tri::frenado(){

  

  analogWrite(pinMotor,0); // por ahora esta es la unica manera de frenar que conocemos
  
}

void Motor_tri::frenadoGradual(){
  analogWrite(pinMotor,50);
  //delay(2000);
  analogWrite(pinMotor,35);
  //delay(2000);
  analogWrite(pinMotor,20);
  //delay(2000);
  analogWrite(pinMotor,0);
  
}


