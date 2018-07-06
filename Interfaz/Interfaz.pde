import processing.serial.*;

Serial port;
String buff;
boolean receivedString = false;

Button button1;
Button button2;
Button button3;
Radar radar;

//Datos PLC
int globalState;
int localState;

//IMU plataforma
float orientX;
float orientY;
float orientZ;

//Objetivo radar
int id;
float velocidad;
float distancia;
float angulo;
float intensidad;

//Temperatura motores
float tempM1;
float tempM2;

void setup(){
  size(700,400);
  
  //Test String parse
  buff = "#States: 10,0;";
  stringParse(buff);
  buff = "#Orient: 178.8125,0.1250,157.3750;";
  stringParse(buff);
  buff = "#Dron: 170,4.72,50.20,3.87,25291212.00;";
  stringParse(buff);
  buff = "#Temp: 42.3,41.8";
  stringParse(buff);

  println(Serial.list());
  port = new Serial (this, Serial.list()[0], 250000);
  port.bufferUntil('\n');
  
  button1 = new Button (50,50,75,75);
  button1.text = "START";
  button1.setColor(0,255,0);
  
  button2 = new Button (150,50,75,75);
  button2.text = "NOTOCAR";
  button2.setColor(255,150,50);
  
  button3 = new Button (250,50,75,75);
  button3.text = "STOP";
  button3.setColor(255,0,0);
  
  radar = new Radar(300,250,600,40);
  
}

void draw(){
  if (receivedString){
    print(buff);
    stringParse(buff);
    buff = "";
    receivedString = false;
  }
  
  
  background(200);
  button1.draw();
  button2.draw();
  button3.draw();
  radar.draw();
}

void serialEvent(Serial port) {
  buff += trim(port.readString());
  buff += '\n';
  receivedString = true;
}

void mousePressed(){
  if(button1.isMouseOver()){
    port.write("1");
  }
  if (button2.isMouseOver()){
    port.write("E");
  }
  if (button3.isMouseOver()){
    port.write("0");
  }
}
