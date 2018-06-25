#include <Controllino.h>
#include "Schneider_LMD_P84.h"
#include "Radiofrecuencia.h"

#define ID_MOTOR_1 0x610
#define ID_MOTOR_2 0x611
//#define ID_MOTOR_3 0x612
//#define ID_MOTOR_4 0x613

//Para calibracion
#define velCal 5120
#define acelCal 10000
#define decelCal 10000

//Para Compensacion
#define velocidad 512000
#define aceleracion 1000
#define deceleracion 1000

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
unsigned long antesC1 = 0;
unsigned long antesC2 = 0;
unsigned long ahoraC3;
unsigned long antesC3 = 0;

bool errorIMU = false;
bool errorCAN = false;
bool errorMotoresON = false;
bool errorMotoresSetup = false;
bool errorComunicPLCs = false;
bool errorComunicRF = false;

bool entradaEstado = true;
bool entradaEstadoError = true;

char serialToken;
bool serialIn = false;

// Offsets
double offset_alabeo;
double offset_cabeceo;

void errorSolucionado (uint8_t estado){
  if (serialIn){
    if (serialToken == 'C'){
      nextState(estado);
    }
    else if (serialToken == 'E'){ //Para pruebas
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

  //Lectura del serial PC
  if (Serial.available()){
    serialToken = Serial.read();
    serialIn = true;
  }
  else{
    serialIn = false;
  }

  //Máquina de estados
  if (globalState == 0){
    if (serialIn){
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

    limpiaBuffer();

    float tensionM1 = (float)requestVin(ID_MOTOR_1) / 10;
    float tensionM2 = (float)requestVin(ID_MOTOR_2) / 10;
    
    Serial.print("\tTension M1: ");
    Serial.println(tensionM1);
    Serial.print("\tTension M2: ");
    Serial.println(tensionM2);

    if (tensionM1 > 47.5 && tensionM1 < 48.5){ //24.5
      if (tensionM2 > 47.5 && tensionM2 < 48.5){
        Serial.println("\tAlimentacion en rango");
        errorMotoresON = false;
        nextState(4);
      }
    }
    if(globalState != 4){
      Serial.println("\tMotores fuera de rango");
      errorMotoresON = true;
      nextState(5);
    }
  }

  else if (globalState == 4){
    //Setup de motores
    Serial.println("Setup motores");
    Serial.println("\tMotor 1");
    bool m1 = setupMotor(ID_MOTOR_1,acelCal,decelCal,100,velCal); //(long ID_motor,uint32_t Acel,uint32_t Decel, int current ,uint32_t MaxVel )
    Serial.println("\tMotor 2");
    bool m2 = setupMotor(ID_MOTOR_2,acelCal,decelCal,100,velCal);

    if (m1 && m2){
      Serial.println("Setup correcto");
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
    else if (com_maxi.getError()){
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
    delay(1000);

    limpiaBuffer();
    
    //Se vuelve a hacer el setup
    Serial.println("Setup motores");
    Serial.println("\tMotor 1");
    bool m1 = setupMotor(ID_MOTOR_1,aceleracion,deceleracion,100,velocidad); //(long ID_motor,uint32_t Acel,uint32_t Decel, int current ,uint32_t MaxVel )
    Serial.println("\tMotor 2");
    bool m2 = setupMotor(ID_MOTOR_2,aceleracion,deceleracion,100,velocidad);

    if (m1 && m2){
      Serial.println("Setup correcto");
      errorMotoresSetup = false;
    }
    else{
      Serial.println("Fallo setup motores");
      errorMotoresSetup = true;
    }
    delay(500);

    if (!errorMotoresSetup){
      nextState(10);
    }
  }

  else if (globalState == 10){
    if(entradaEstado){
      Serial.println("Compensacion Plataforma");
      entradaEstado = false;

      //Medicion del offset respecto del horizonte de la estructura
      IMU_fija.update();
      offset_alabeo = IMU_fija.alabeo;
      offset_cabeceo = IMU_fija.cabeceo;
      Serial.print("\toffset_alabeo:");
      Serial.println(offset_alabeo);
      Serial.print("\toffset_cabeceo:");
      Serial.println(offset_cabeceo);

      //Set temporizadores de ciclo
      antesC1 = millis();
      antesC2 = millis();
    }

    ahora = millis();

    if (ahora - antesC1 > 20){
      antesC1 = ahora;

      IMU_fija.update();
      moverMotores(IMU_fija.cabeceo - offset_cabeceo, IMU_fija.alabeo - offset_alabeo);
    }

    if (ahora - antesC2 > 500){
      antesC2 = ahora;

      com_maxi.requestData();
      //com_maxi.printData();
    }
    
    if (serialIn) {
      if (serialToken == '1'){
        IMU_fija.imprimirDatos();
        IMU_fija.displayCalStatus();
        IMU_fija.printTemp();
        IMU_fija.print();
      }
    }
    
  }

  else if (globalState == 20){
     if(entradaEstado){
      Serial.println("Estado error CAN");
      entradaEstado = false;
    }
    bool resuelto = compruebaCAN();
    if (resuelto){
      Serial.println("Setup motores");
      Serial.println("\tMotor 1");
      bool m1 = setupMotor(ID_MOTOR_1,aceleracion,deceleracion,100,velocidad); //(long ID_motor,uint32_t Acel,uint32_t Decel, int current ,uint32_t MaxVel )
      Serial.println("\tMotor 2");
      bool m2 = setupMotor(ID_MOTOR_2,aceleracion,deceleracion,100,velocidad);

      if (m1 && m2){
        Serial.println("Setup correcto");
        errorMotoresSetup = false;
      }
      else{
        Serial.println("Fallo setup motores");
        errorMotoresSetup = true;
      }
      delay(1000);
      
      if (!errorMotoresSetup){
        nextState(8);
      }
    }
  }

  //En paralelo al proceso principal
  if (serialIn){
    if (serialToken == '0'){
      Serial.println("STOP");
      nextState(0);
    }
  }
  
  if (emergCAN){
    nextState(20);
  }

  //Ciclo 3 - Ejecucion cada 1 segundo
  ahoraC3 = millis();
  if (ahoraC3 - antesC3 >= 1000){
    antesC3 = ahoraC3;

    //Datos para interfaz
    if (globalState > 7){
      //Temperaturas motores
      Serial.print("#Temp: ");
      Serial.print(requestBoardTemp(ID_MOTOR_1));
      Serial.print(",");
      Serial.print(requestBoardTemp(ID_MOTOR_2));
      Serial.println(";");
      
      //IMU fija
      Serial.print("#Orient: ");
      Serial.print (IMU_fija.orientacion.x, 4);
      Serial.print(",");
      Serial.print (IMU_fija.orientacion.y, 4);
      Serial.print(",");
      Serial.print (IMU_fija.orientacion.z, 4);
      Serial.println(";");

      //Datos del dron 
      com_maxi.sendData2Interface();
    }

    //Estados de la máquina de estados
    Serial.print("#States: ");
    Serial.print (globalState);
    Serial.print(",");
    Serial.print (localState);
    Serial.println(";");
  }
  
}
