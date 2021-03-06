
class Objetivo{
  int id;
  float vel;
  float dist;
  float ang;
  float intensidad;
  
  float posX;
  float posY;
  
  PFont font;
  String data;
  int txtSize;
  
  Objetivo(){
    txtSize = 16;
    font = createFont("Arial",txtSize,true);
  }
  
  void draw(float posRadarX, float posRadarY, float escaleFactor){
    if (id != 0){
      float distX = dist*cos(radians(ang));
      float distY = dist*sin(radians(ang));
      distX /= escaleFactor;
      distY /= escaleFactor;
      
      posX = posRadarX + distX;
      posY = posRadarY + distY;
      
      fill(255, 0, 0);
      ellipse(posX, posY, 10, 10);
      
      //Text
      fill(0);
      textAlign(LEFT,TOP);
      data = "Vel: " + str(vel);
      textFont(font,txtSize);
      text(data, posX, posY+5);
      data = "Dist: " + str(dist);
      text(data, posX, posY+5 + txtSize);
    }
  }
}
