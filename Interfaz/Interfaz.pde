import processing.serial.*;

Serial port;
String buff;

Button button1;
Button button2;
Button button3;
Button button4;
Button button5;
Button button6;
Radar radar;
Objetivo dron;
Telemetria telemetria;
Alarmas alarmas;
Gauge gauge1;
BarIndicator bar1;
BarIndicator bar2;
BarIndicator bar3;

PImage logoAID;

//Datos PLC
int globalState;
int localState;

//IMU plataforma
float orientX;
float orientY;
float orientZ;

//Temperatura motores
float tempM1;
float tempM2;

//Plataforma
int state; //del maxi
float velGiro;
float accelX = 0;
float accelY = 0;
float accelZ = 0;

void setup(){
  size(1100,600);
  

  boolean serialError = false;
  try{
    println(Serial.list());
    port = new Serial (this, Serial.list()[0], 250000);
    port.bufferUntil('\n');
  }
  catch(ArrayIndexOutOfBoundsException a){
    println("PLC no conectado al PC");
    serialError = true;
  }
  
  button1 = new Button (50,50,75,75);
  button1.text = "START";
  button1.setColor(0,255,0);
  
  button2 = new Button (150,50,75,75);
  button2.text = "CONTINUE";
  button2.setColor(255,150,50);
  
  button3 = new Button (250,50,75,75);
  button3.text = "STOP";
  button3.setColor(255,0,0);
  
  button4 = new Button (350,50,75,75);
  button4.text = "ATERRIZAJE";
  button4.setColor(0,100,255);
  
  button5 = new Button (450,50,75,75);
  button5.text = "NOTOCAR";
  button5.setColor(0,100,255);
  
  button6 = new Button (550,50,75,75);
  button6.text = "State10";
  button6.setColor(0,100,100);
  
  radar = new Radar(600,250,400,40,150);
  dron = new Objetivo();
  telemetria = new Telemetria(20,130);
  alarmas = new Alarmas(300,130);
  gauge1 = new Gauge(500,500);
  gauge1.setLimits(0,100);
  gauge1.setColor(185,92,200);
  gauge1.setName("VELOCIDAD GIRO");
  
  bar1 = new BarIndicator(700,450,200,15);
  bar1.setLimits(-10,10);
  bar1.setColor(255,164,032);
  bar2 = new BarIndicator(700,500,200,15);
  bar2.setLimits(-10,10);
  bar2.setColor(255,164,032);
  bar3 = new BarIndicator(700,550,200,15);
  bar3.setLimits(-10,10);
  bar3.setColor(255,164,032);
  
  logoAID = loadImage("C:/Users/AID_1/Desktop/AID-Logo.png");
}

void draw(){  
  background(200);
  //tint(255, 127);
  //image(logoAID,600,0);
  button1.draw();
  button2.draw();
  button3.draw();
  button4.draw();
  button5.draw();
  button6.draw();
  radar.draw(dron);
  telemetria.draw();
  alarmas.draw();
  gauge1.draw(dron.vel);
  bar1.draw(accelX);
  bar2.draw(accelY);
  bar3.draw(accelZ);
}

void serialEvent(Serial port) {
  buff += port.readString();
  //print(buff);
  stringParse(buff);
  buff = "";
}

void mousePressed(){
  if(button1.isMouseOver()){
    port.write("1");
  }
  if (button2.isMouseOver()){
    port.write("C");
  }
  if (button3.isMouseOver()){
    port.write("0");
  }
  if (button5.isMouseOver()){
    port.write("E");
  }
  if (button6.isMouseOver()){
    port.write("F");
  }
}
