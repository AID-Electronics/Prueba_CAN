 #include "IMU.h"

IMU IMU_movil;
 int i=0;
void setup(){
    Serial1.begin(4800);     
    Serial.begin(115200);
    IMU_movil.setup();
    
}
 
void loop(){ 
   IMU_movil.update(); 
   Serial.print(IMU_movil.orientacion.z,4); 
   Serial.print(",");
   Serial.println(IMU_movil.orientacion.y,4);
  //SERIAL 1
   Serial1.print("=");
   Serial1.print(IMU_movil.orientacion.z,4); 
   Serial1.print(",");
   Serial1.println(IMU_movil.orientacion.y,4);

    if(Serial.read()!=-1){
       IMU_movil.imprimirDatos();
    //IMU_movil.displayCalStatus();
    //IMU_movil.printTemp();
    //IMU_movil.print();
    }
  }

