import processing.serial.*;

Serial port;

Button button1;
Button button2;
Button button3;

void setup(){
  size(600,300);
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
  
}

void draw(){
  background(200);
  button1.draw();
  button2.draw();
  button3.draw();
}

void serialEvent(Serial port) {
  String str = trim(port.readString());
  println(str);
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
