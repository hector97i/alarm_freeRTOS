import processing.serial.*;

PImage bg;
PShape camera1;
Serial myPort;
String val;

void setup() {
  // 1154, 1062
  size(720, 720);
  bg = loadImage("bg_image.jpeg");
  background(bg);
  printArray(Serial.list());
  myPort = new Serial(this, Serial.list()[0], 9600);
  
  camera1(0,255,0);
  camera2(0,255,0);
  camera3(0,255,0);
  camera4(0,255,0);
}

void draw() {
  if(myPort.available() > 0) {
    val = myPort.readStringUntil('\n');
    if(val != null) {
      print(val);
      JSONObject json = parseJSONObject(val);
      String sensor = json.getString("sensor");
      Boolean state = json.getBoolean("state");
      
      checkStateJson(state, Integer.valueOf(sensor));
    }
  }
}

void camera1(int r, int g, int b) {
  fill(r, g, b);
  square(430, 315, 30);
}

void camera2(int r, int g, int b) {
  fill(r, g, b);
  square(635, 280, 30);
}

void camera3(int r, int g, int b) {
  fill(r, g, b);
  square(220, 280, 30);
}

void camera4(int r, int g, int b) {
  fill(r, g, b);
  square(635, 645, 30);
}

void checkStateJson(Boolean state, int sensor) {
  switch(sensor) {
    case 1:
      if(state == false) {
        camera1(0,255,0);
        break;
      } else {
        camera1(255,0,0);
        break;
      }
     case 2:
     if(state == false) {
        camera2(0,255,0);
        break;
      } else {
        camera2(255,0,0);
        break;
      }
     case 3: 
     if(state == false) {
        camera3(0,255,0);
        break;
      } else {
        camera3(255,0,0);
        break;
      }
      case 4:
      if(state == false) {
        camera4(0,255,0);
        break;
      } else {
        camera4(255,0,0);
        break;
      }
  }
}
