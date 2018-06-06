#include <Controllino.h>
#include "Schneider_LMD_P84.h"
#include "Radiofrecuencia.h"

#define ID_MOTOR_1 0x610
#define ID_MOTOR_2 0x611
//#define ID_MOTOR_3 0x612
//#define ID_MOTOR_4 0x613

#define velocidad 5120
#define aceleracion 1000000
#define deceleracion 1000000

#include "IMU.h"
#include "Plataforma.h"
#include "Comunicacion_MAXI.h"


IMU IMU_fija;
Plataforma platform;
Comunicacion_MAXI com_maxi;

uint8_t globalState;
uint8_t localState;

unsigned long arrivalState_time;
unsigned long inState_time;

unsigned long ahora;
unsigned long antes;

bool errorIMU = false;
bool errorCAN = false;
bool errorMotoresON = false;
bool errorMotoresSetup = false;
bool errorComunicPLCs = false;
bool errorComunicRF = false;

bool entradaEstado = true;
bool entradaEstadoError = true;

void errorSolucionado (uint8_t estado){
  if (Serial.available()){
    char token = Serial.read()
    if (token == 'C'){
      nextState(estado);
    }
    else if (token == 'E'){ //Para pruebas
      nextState(8);    //Se salta las comprobaciones de seguridad
    }
  }
}

void nextState(uint8_t estado){
  Serial.print ("Paso al estado ");
  Serial.println (estado);
  Serial.println();
  globalState = estado;
  localState = 0;
  entradaEstado = true;
  arrivalState_time = millis();
}

void setup(){
  Serial.begin(250000);
  Serial1.begin(4800);
  Serial3.begin(115200);
  globalState = 0;
  localState = 0;

  com_maxi.setup();

  //Alimentacion motores
  pinMode(CONTROLLINO_R0, OUTPUT);
  pinMode(CONTROLLINO_R1, OUTPUT);
  digitalWrite(CONTROLLINO_R0, LOW);
  digitalWrite(CONTROLLINO_R1, LOW);
  
  //Alimentacion MAXI
  pinMode(CONTROLLINO_R4, OUTPUT);
  digitalWrite(CONTROLLINO_R4, LOW);

  Serial.println ("Setup Controllino Finalizado");
  Serial.println ("Pulse una tecla para continuar");
}

char a;
long pos;

void loop(){

  if (globalState == 0){
    if (Serial.available()){
      nextState(1);
    }
  }
  else if (globalState == 1){
    bool OK;
    OK = IMU_fija.setup();
    errorIMU = !OK;
    nextState(2);
  }
  else if (globalState == 2){
    if(entradaEstado){
      Serial.println("Setup CANbus");
      entradaEstado = false;
    }
    bool setupCAN_ok = setupCAN();
    errorCAN = !setupCAN_ok;

    if(setupCAN_ok){
      nextState(3);
    }
    else{
      nextState(5);
    }
  }
  else if (globalState == 3){
    if(entradaEstado){
      Serial.println("Encendido de motores");
      entradaEstado = false;
    }
    //Encendido de motores
    digitalWrite(CONTROLLINO_R0, HIGH);
    digitalWrite(CONTROLLINO_R1, HIGH);
    delay(1000);

    float tensionM1 = (float)requestVin(ID_MOTOR_1) / 10;
    float tensionM2 = (float)requestVin(ID_MOTOR_2) / 10;
    
    Serial.print("\tTension M1: ");
    Serial.println(tensionM1);
    Serial.print("\tTension M2: ");
    Serial.println(tensionM2);

    if (tensionM1 > 23.5 && tensionM1 < 24.5){
      if (tensionM2 > 23.5 && tensionM2 < 24.5){
        Serial.println("\tAlimentacion en rango");
        errorMotoresON = false;
        nextState(4);
      }
    }
    if(globalState != 4){
      Serial.println("\tMotores sin alimentacion");
      errorMotoresON = true;
      nextState(5);
    }
  }

  else if (globalState == 4){
    //Setup de motores
    Serial.println("Setup motores");
    setupMotor(ID_MOTOR_1,aceleracion,deceleracion,100,velocidad); //(long ID_motor,uint32_t Acel,uint32_t Decel, int current ,uint32_t MaxVel )
    setupMotor(ID_MOTOR_2,aceleracion,deceleracion,100,velocidad);

    long m1_Accel = requestAccel(ID_MOTOR_1);
    long m1_Decel = requestDecel(ID_MOTOR_1);
    long m1_Vel = requestMaxVel(ID_MOTOR_1);

    long m2_Accel = requestDecel(ID_MOTOR_2);
    long m2_Decel = requestDecel(ID_MOTOR_2);
    long m2_Vel = requestMaxVel(ID_MOTOR_2);

    
    Serial.print("\tAceleracion: ");
    Serial.println(m1_Accel);
    Serial.print("\tDeceleracion: ");
    Serial.println(m1_Decel);
    Serial.print("\tMax Velocity: ");
    Serial.println(m1_Vel);

    Serial.print("\tAceleracion: ");
    Serial.println(m2_Accel);
    Serial.print("\tDeceleracion: ");
    Serial.println(m2_Decel);
    Serial.print("\tMax Velocity: ");
    Serial.println(m2_Vel);

    if (m1_Vel == velocidad && m2_Vel == velocidad){
      //Por ahora no tenemos en cuenta aceleraciones
      errorMotoresSetup = false;
    }
    else{
      Serial.println("Fallo setup motores");
      errorMotoresSetup = true;
    }
    
    nextState(5);
  }
  
  else if (globalState == 5){
    if (entradaEstado){
      Serial.println("Test comunicacion RF");
      entradaEstado = false;
    }
    Vector3D test;
    if (getOrientRF(&test)){
      errorComunicRF = false;
      Serial.println("\tRecepcion RF OK");
      nextState(6);
    }
    else{
      //comprobar tiempo de espera
      inState_time = millis() - arrivalState_time;
      if (inState_time > 5000){
        errorComunicRF = true;
        Serial.println("\tError Recepcion RF");
        nextState(6);
      }
    }
  }

  else if (globalState == 6){
    //Test comunicacion MAXI
    if (entradaEstado){
      Serial.println("Test comunicacion PLCs");
      com_maxi.resetMsg();
      Serial.println("\tEncendido PLC MAXI");
      digitalWrite(CONTROLLINO_R4, HIGH);
      entradaEstado = false;
    }

    if (localState == 0){
      if(com_maxi.receive()){
        Serial.print("\t");
        com_maxi.printBuffer();
        if (com_maxi.buff[0] == 'E'){
          com_maxi.parseBuff();
        }
        else {
          Serial.println("\tError: menseje no esperado");
          com_maxi.errorCom = true;
        }
        localState = 1;
      }
    }
    else if (localState == 1){
      digitalWrite(pinEstado,HIGH);
      Serial.println("\tBit com - HIGH");
      delay(100);
      digitalWrite(pinEstado,LOW);
      Serial.println("\tBit com - LOW");
      localState = 2;
    }
    else if (localState == 2){
      if(com_maxi.receive()){
        Serial.print("\t");
        com_maxi.printBuffer();
        if (com_maxi.buff[0] == 'C'){
          com_maxi.parseBuff();
        }
        else{
          Serial.println("\tError: menseje no esperado");
          com_maxi.errorCom = true;
        }
        errorComunicPLCs = false;
        nextState(7);
      }
    }
    //Si no recibe nada despues de 5 segundos
    inState_time = millis() - arrivalState_time;
    if (inState_time > 5000){
      Serial.println("\tError: respuesta no recibida");
      errorComunicPLCs = true;
      nextState(7);
    }
  }

  else if (globalState == 7){
    if (entradaEstado){
      Serial.println("Errores durante configuracion");

      Serial.print("\terrorIMU: ");
      Serial.println(errorIMU);
      Serial.print("\terrorCAN: ");
      Serial.println(errorCAN);
      Serial.print("\terrorMotoresON: ");
      Serial.println(errorMotoresON);
      Serial.print("\terrorMotoresSetup: ");
      Serial.println(errorMotoresSetup);
      Serial.print("\terrorComunicPLCs: ");
      Serial.println(errorComunicPLCs);
      Serial.print("\terrorComunicRF: ");
      Serial.println(errorComunicRF);
      com_maxi.printError();

      entradaEstado = false;
    }
    if (errorIMU){
      errorSolucionado (1);
    }
    else if (errorCAN){
      errorSolucionado (2);
    }
    else if (errorMotoresON){
      errorSolucionado (3);
    }
    else if (errorMotoresSetup){
      errorSolucionado (4);
    }
    else if (errorComunicRF){
      errorSolucionado (5);
    }
    else if (errorComunicPLCs){
      errorSolucionado (6);
    }
    else {
      nextState(8);
    }
  }

  else if (globalState == 8){
    bool calibState;
    calibState = platform.calibrarPlat();

    String serialBuff;
    serialBuff += (String)calibState + " accelX: " + (String)platform.accel.x + " accelY: " + (String)platform.accel.y + " accelZ: " + (String)platform.accel.z ;
    Serial.println(serialBuff);
  
    if(calibState == 1){
      nextState(9);
    }
  }

  else if (globalState == 9){
    //Apaga el motor y vuelve a encenderlo
    digitalWrite(CONTROLLINO_R0, LOW);
    digitalWrite(CONTROLLINO_R1, LOW);
    delay(500);
    digitalWrite(CONTROLLINO_R0, HIGH);
    digitalWrite(CONTROLLINO_R1, HIGH);
      
    //Se vuelve a hacer el setup
    setupMotor(ID_MOTOR_1,1000000,1000000,100,51200);
    setupMotor(ID_MOTOR_2,1000000,1000000,100,51200);

    delay(500);
    nextState(10);
  }

  else if (globalState == 10){
    if(entradaEstado){
      Serial.println("Compensacion Plataforma");
      entradaEstado = false;
    }
    IMU_fija.update();
    moverMotores(IMU_fija.cabeceo, IMU_fija.alabeo);

    ahora = millis();
    if (ahora - antes > 500){
      com_maxi.requestData();
      com_maxi.printData();
      antes = ahora;
    }
  
    if(Serial.read()!=-1){
      IMU_fija.imprimirDatos();
      IMU_fija.displayCalStatus();
      IMU_fija.printTemp();
      IMU_fija.print();
    }
  }
  
  if (Serial.available()){
    if (Serial.read() == '0'){
      Serial.println("STOP");
      nextState(0);
    }
  }
}
