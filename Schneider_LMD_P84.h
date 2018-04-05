#ifndef Schneider_LMD_P84_H
#define Schneider_LMD_P84_H

#include <Arduino.h>

#include <mcp_can.h>
#include <SPI.h>

unsigned char Flag_Recv = 0;
unsigned char len = 0;
unsigned char buffRespuesta[8];
char str[20];

union Paquete{
  byte b[4];
  long i;
};
const char ReadytoSwitch[]={0x2B,0x40,0x60,0x00,0x06,0x00,0x00,0x00};
const char SwitchON[]={0x2B,0x40,0x60,0x00,0x07,0x00,0x00,0x00};
const char OpEnable[]={0x2B,0x40,0x60,0x00,0x0F,0x00,0x00,0x00};


MCP_CAN CAN(53);                                      // Set CS to pin 53

void sending( char buff[], long ID) {
        Serial.print("(SENT)ID: ");
        Serial.print(ID,HEX);
        Serial.print(" / ");

    for(int i=0; i<8;i++){
      Serial.print(buff[i],HEX);
      Serial.print(",");
    }
    Serial.print("\n");
    CAN.sendMsgBuf(ID, 0, 8, buff);
    
}

bool receive(){
    if(CAN_MSGAVAIL == CAN.checkReceive()){          // check if data coming
    
        CAN.readMsgBuf(&len, buffRespuesta);    // read data,  len: data length, buf: data buf

        Serial.print("(RECEIVED)ID: ");

        Serial.print(CAN.getCanId(),HEX);

       Serial.print(" / ");

        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buffRespuesta[i],HEX);
            Serial.print(",");
        }
        Serial.println();

        return true;
    }else
    
    return false;
}

bool comprobarRespuesta(){
  int flag_receive=0;
  int i=0;
  
  while(!flag_receive && i<50){
    flag_receive=receive();
    i++;
  }
  
  Serial.println(i);
  
  if (flag_receive == 1){
    if(buffRespuesta[0]==0x80){
      return false;
    }
    else if(buffRespuesta[0]==0x60){
      return true;
    }
  }
  else{
    return false;
  }
}

bool EnviarMSG(char buff[], long ID){
  bool rec_OK = 0; 
  
  for (int contError = 0; contError < 3 && rec_OK == 0; contError++){
    sending(buff,ID);
    
    if(comprobarRespuesta() == 1){
      rec_OK = 1;
      Serial.println("MSG RECIBIDO CORRECTAMENTE");
      Serial.println("");
    }
    else {
      Serial.println("ERROR EN MSG");
      Serial.println("");
    }
  }

  if (rec_OK == 0){
    Serial.println ("Mensaje erroneo");
  }
  return rec_OK;
}

bool SetCurrent (int porcentaje, long ID){
  //const byte SetcurrentUSE[]={0x2F,0x04,0x22,0x00,0x50,0x00,0x00,0x00};
    Paquete p;
  p.i = porcentaje;
  char SetcurrentUSE[]={0x2F,0x04,0x22,0x00,p.b[0],0x00,0x00,0x00};
    return EnviarMSG(SetcurrentUSE,ID);
}

bool maxVelocity (long velocity, long ID){
  //const char Maxvel[]={0x23,0x81,0x60,0x00,0x00,0xC8,0x00,0x00};
  Paquete p;
  p.i = velocity;
  char Maxvel[]={0x23,0x81,0x60,0x00,p.b[0],p.b[1],p.b[2],p.b[3]};
    return EnviarMSG(Maxvel,ID);
}


bool setDeccel (uint32_t decel, long ID){
  //const char SetDecel[]={0x23,0x83,0x60,0x00,0x40,0x42,0x0F,0x00};
  Paquete p;
  p.i = decel;
  char SetDecel[]={0x23,0x84,0x60,0x00,p.b[0],p.b[1],p.b[2],p.b[3]};
    return EnviarMSG(SetDecel,ID);
}

bool SetAccel (long accel, long ID){
  //char SetAccel[]={0x23,0x84,0x60,0x00,0x40,0x42,0x0F,0x00};
  Paquete a;
  a.i = accel;
  char SetAccel[]={0x23,0x83,0x60,0x00,a.b[0],a.b[1],a.b[2],a.b[3]};
  return EnviarMSG(SetAccel,ID);
  
}

bool SetProfile(int profile, long ID ){
  byte pro;
  switch(profile){
    case 1:
     pro=0x01;
     break;
    case 2:
     pro=0x03;
     break;
    case 3:
     pro=0x06;
     break;
    case 4:
     pro=0x04;
     break;
    default:
      pro=0x00;
      break;
     
  }
   
  char ProfileSet[]={0x2F,0x60,0x60,0x00,pro,0x00,0x00,0x00};
  EnviarMSG(ProfileSet,ID);
}





void setupMotor(long ID_motor,uint32_t Acel,uint32_t Decel, int current ,uint32_t MaxVel ){

  
    int cuenta=0;

    //instrucciones de configuración
    
    SetAccel(Acel,ID_motor);
    setDeccel(Decel,ID_motor);
    maxVelocity(MaxVel, ID_motor);
    SetCurrent(current, ID_motor);
    
    
    //instrucciones de cambio de estado
    EnviarMSG(ReadytoSwitch,ID_motor);
    EnviarMSG(SwitchON,ID_motor);
    EnviarMSG(OpEnable,ID_motor);
    SetProfile(1,ID_motor); //1=modo posición, 2=modo velocidad, 3=modo homing, 4=modo torque
}

void requestPos(long ID){
  char leerPos[8]={0x40,0x62,0x60,0x00,0x00,0x00,0x00,0x00};
  EnviarMSG(leerPos,ID);
}

void requestVin(long ID){
  char leerVin[8]={0x40,0x15,0x20,0x01,0x00,0x00,0x00,0x00};
  EnviarMSG(leerVin,ID);
}

void mover (long pasos,long ID){ //pasos debe ser de tipo long para poder contar los suficientes pasos
  char polarity[8]={0x2F,0x7E,0x60,0x00,0xC0,0x00,0x00,0x00};
  if (pasos<0){
    //polarity[4]=0xFF;
  }
  else {
    //polarity[4]=0x7F;
  }

  Paquete p;
  //p.i=abs(pasos);
  p.i=pasos;
  
  char CadPos1[]={0x23,0x7A,0x60,0x00,p.b[0],p.b[1],p.b[2],p.b[3]}; //Indica la posición a la que ha de moverse
  char CadPos2[]={0x2B,0x40,0x60,0x00,0x3F,0x00,0x00,0x00};   // son cadenas complementarias para el movimiento que indican el tipo de este: 
  char CadPos3[]={0x2B,0x40,0x60,0x00,0x2F,0x00,0x00,0x00};   //El movimiento será relativo y no se espera a que acabe antes de procesar el siguiente.
//5F,4F-->movi relativo con espera
//7F,6F-->movi relativo sin espera
//3F,2F-->movi absoluto sin espera
//1F,0F-->movi absluto con espera
  //EnviarMSG(polarity,ID);
  EnviarMSG(CadPos1,ID);
  EnviarMSG(CadPos2,ID);
  EnviarMSG(CadPos3,ID);
 }
#endif
