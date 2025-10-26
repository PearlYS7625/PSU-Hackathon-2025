#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "WebServer.h"
#include <DNSServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <FS.h>


// MotionApps / I2Cdev headers provide DMP-capable MPU6050 class
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "util.h"

DNSServer dnsServer;
WebServer server(80);

void handleRoot(){
  server.send(200, "text", "Test");
}

/* MPU6050 default I2C address is 0x68*/
MPU6050 mpu;
//MPU6050 mpu(0x69); //Use for AD0 high
//MPU6050 mpu(0x68, &Wire1); //Use for AD0 low, but 2nd Wire (TWI/I2C) object.

/*Conversion variables*/
#define EARTH_GRAVITY_MS2 9.80665  //m/s2
#define DEG_TO_RAD        0.017453292519943295769236907684886
#define RAD_TO_DEG        57.295779513082320876798154814105

double distance;

/*---MPU6050 Control/Status Variables---*/
bool DMPReady = false;  // Set true if DMP init was successful
uint8_t MPUIntStatus;   // Holds actual interrupt status byte from MPU
uint8_t devStatus;      // Return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // Expected DMP packet size (default is 42 bytes)
uint8_t FIFOBuffer[64]; // FIFO storage buffer

/*---MPU6050 Control/Status Variables---*/
Quaternion q;           // [w, x, y, z]         Quaternion container
VectorInt16 aa;         // [x, y, z]            Accel sensor measurements
VectorInt16 gg;         // [x, y, z]            Gyro sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            World-frame accel sensor measurements
VectorInt16 ggWorld;    // [x, y, z]            World-frame gyro sensor measurements
VectorFloat gravity;    // [x, y, z]            Gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   Yaw/Pitch/Roll container and gravity vector

#define TRIG 13
#define ECHO 12

#define aspectRatioRads 0.5123894603
double screenSize = 34.544; // diagonal screen size in cm of macbook

void calibrateMotion() {
    mpu.CalibrateAccel(6);  // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateGyro(6);
    Serial.println("These are the Active offsets: ");
    mpu.PrintActiveOffsets();
    Serial.println(F("Enabling DMP..."));   //Turning ON DMP
    mpu.setDMPEnabled(true);  
}

WebSocketsServer webSocket = WebSocketsServer(81);  // Port 81

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  String msg = String((char *)payload);
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[%u] Connected from %s\n", num, webSocket.remoteIP(num).toString().c_str());
      webSocket.sendTXT(num, "Hello from ESP32!");
      break;

    case WStype_TEXT:
      Serial.printf("[%u] Received text: %s\n", num, payload);
      if(msg.startsWith("screenSize")) {
          Serial.println("Screen size command received.");
          // Handle screen size command if needed
          screenSize = msg.substring(11).toDouble();
      }
      else if(msg.equals("calibrate")) {
          Serial.println("Calibration command received.");
          calibrateMotion();
      }
      break;

    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
  }
}

void calculateCoordinates(float* ypr, double distance, int16_t* x, int16_t* y) {
    // Example calculation: map distance to x and y coordinates
    // Adjust the mapping logic as per your requirements
    double screenWidth = screenSize * sin(aspectRatioRads);
    double screenHeight = screenSize * cos(aspectRatioRads);
    double tempX, tempY;
    tempX = (distance * sin(ypr[0])); // find the cm from center were pointing
    tempX = (abs(tempX) > screenWidth/2) ? ( (tempX > 0) ? screenWidth/2 : -screenWidth/2 ) : tempX; // Clamp x to screen bounds, W readability, the code documents itself type shiiii
    *x = mapDouble(tempX, -screenWidth/2, screenWidth/2, 0, 1593); // map to screen coordinates assuming 600px width
    tempY = (distance * sin(ypr[1] * 2)); // find the cm from center were pointing
    tempY = (abs(tempY) > screenHeight/2) ? ( (tempY > 0) ? screenHeight/2 : -screenHeight/2 ) : tempY; // Clamp y to screen bounds
    *y = mapDouble(tempY, -screenHeight/2, screenHeight/2, (float)800, (float)0); // map to screen coordinates assuming 400px height

}

void sendFile()
{
  String url = server.uri();
  String type = "text/";
  if(url.indexOf('.') >= 0) type.concat(url.substring(url.indexOf('.')+1));
  else type.concat("html");
  if(LittleFS.exists(url)) {
    File file = LittleFS.open(url, "r");
    server.streamFile(file, type);
    file.close();
  }
  else server.send(404, "text", url+" not found");
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens
  Wire.setPins(2, 1);
  Wire.begin();
  // Try to initialize!
  mpu.initialize();
  mpu.setDMPEnabled(true);
  Serial.println("MPU6050 Found!");


//Start of the website on the 32
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Draw");
    //WiFi.AP.enableDhcpCaptivePortal();


    dnsServer.start(80, "*", WiFi.softAPIP());
    server.on("/", handleRoot);
    server.onNotFound(sendFile);
    server.begin();

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    pinMode(ECHO, INPUT);
    pinMode(TRIG, OUTPUT);

    LittleFS.begin(true);


  //setupt motion detection
 Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  /* Supply your gyro offsets here, scaled for min sensitivity */
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);

  /* Making sure it worked (returns 0 if so) */ 
  if (devStatus == 0) {
    calibrateMotion();

    MPUIntStatus = mpu.getIntStatus();

    /* Set the DMP Ready flag so the main loop() function knows it is okay to use it */
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    DMPReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize(); //Get expected DMP packet size for later comparison
  }

  Serial.println("");
  delay(100);
}

void loop() {

    dnsServer.processNextRequest();
    server.handleClient();


    if (mpu.dmpGetCurrentFIFOPacket(FIFOBuffer)) { // Get the Latest packet 
    /*Display quaternion values in easy matrix form: w x y z */
    mpu.dmpGetQuaternion(&q, FIFOBuffer);
    Serial.print("quat\t");
    Serial.print(q.w);
    Serial.print("\t");
    Serial.print(q.x);
    Serial.print("\t");
    Serial.print(q.y);
    Serial.print("\t");
    Serial.println(q.z);

    mpu.dmpGetGravity(&gravity, &q);

    /* Display initial world-frame acceleration, adjusted to remove gravity
    and rotated based on known orientation from Quaternion */
    mpu.dmpGetAccel(&aa, FIFOBuffer);
    mpu.dmpConvertToWorldFrame(&aaWorld, &aa, &q);
    Serial.print("aworld\t");
    Serial.print(aaWorld.x * mpu.get_acce_resolution() * EARTH_GRAVITY_MS2);
    Serial.print("\t");
    Serial.print(aaWorld.y * mpu.get_acce_resolution() * EARTH_GRAVITY_MS2);
    Serial.print("\t");
    Serial.println(aaWorld.z * mpu.get_acce_resolution() * EARTH_GRAVITY_MS2);

    /* Display initial world-frame acceleration, adjusted to remove gravity
    and rotated based on known orientation from Quaternion */
    mpu.dmpGetGyro(&gg, FIFOBuffer);
    mpu.dmpConvertToWorldFrame(&ggWorld, &gg, &q);
    Serial.print("ggWorld\t");
    Serial.print(ggWorld.x * mpu.get_gyro_resolution() * DEG_TO_RAD);
    Serial.print("\t");
    Serial.print(ggWorld.y * mpu.get_gyro_resolution() * DEG_TO_RAD);
    Serial.print("\t");
    Serial.println(ggWorld.z * mpu.get_gyro_resolution() * DEG_TO_RAD);

    /* Display Euler angles in degrees */
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    Serial.print("ypr\t");
    Serial.print(ypr[0]);
    Serial.print("\t");
    Serial.print(ypr[1]);
    Serial.print("\t");
    Serial.println(ypr[2] );
    
    Serial.println();
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.println(distance);

    int16_t cursorX, cursorY = 0;
    calculateCoordinates(ypr, distance, &cursorX, &cursorY);

    String payload = String(cursorX) + " " + String(cursorY) + " " + String(aa.z);
    Serial.println("Payload: " + payload);

    webSocket.sendTXT(0, payload);
    delay(20);
  }
  
  // Clears the trigPin
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  // Reads the ECHO pin, returns the sound wave travel time in microseconds
  unsigned long duration = pulseIn(ECHO, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  
  webSocket.loop();
}