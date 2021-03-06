
Estructura estruc;
Plataforma plat;
IMU imuFija;
IMU imuMovil;
Calibracion cal;

int state = 0;

void setup() {
  size(800, 600, P3D);
  estruc = new Estructura(0, 0, 0); // Z -> mitad del cubo
  plat = new Plataforma(0, 0, 100);
  imuFija = new IMU ();
  imuMovil = new IMU ();
  cal = new Calibracion();
  rectMode(CENTER);
  
  plat.orientacion(45,30);
}


void draw() {
  background(200);
  
  //Orientacion mundo
  translate(width/2, height/1.5, 0);
  rotateX(PI/2.6); //2.6
  rotateZ(PI/8);
  coordinates(200);
  
  //Orientacion estructura
  float rotX = (mouseX-width/2)/8;
  float rotY = (mouseY-height/1.5)/8;
  estruc.orientacion(rotX,rotY);
  estruc.show();
  imuFija.show(100);
  //plat.orientacion(10,0);
  plat.show();
  imuMovil.show(50);
  
  println("\t AngX: " + degrees(estruc.angX) + "\t AngY: " + degrees(estruc.angY));
  imuFija.computeAccel(estruc.angX,estruc.angY);
  println("* X: " + imuFija.accelX + "\tY: " + imuFija.accelY + "\tZ: " + imuFija.accelZ);
  imuMovil.computeAccel(estruc.angX + plat.angX, estruc.angY + plat.angY);
  println("- X: " + imuMovil.accelX + "\tY: " + imuMovil.accelY + "\tZ: " + imuMovil.accelZ);
  
  //Calibracion
  cal.doCalib();
  
  
}

void coordinates(int longitud){
  strokeWeight(4);
  stroke(255, 0, 0);    //X rojo
  line(0,0,0,longitud,0,0);
  stroke(0, 255, 0);    //Y verde
  line(0,0,0,0,longitud,0);
  stroke(0, 0, 255);    //Z azul
  line(0,0,0,0,0,longitud);
  stroke(0);
  strokeWeight(1);
}

class IMU {
  float accelX;
  float accelY;
  float accelZ;
  
  void computeAccel(float angX, float angY){
    accelX = 10*sin(angY);
    accelY = 10*sin(angX);
    accelZ = 10*cos(angX)*cos(angY);
  }
  void show(int longitud){
    coordinates(longitud);
  }
}
