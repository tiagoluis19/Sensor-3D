/*  Download/Upload files to your ESP32 + SD datalogger/sensor using wifi
    
 By My Circuits 2022, based on code of David Bird 2018 
   
 Requiered libraries:
 
 ESP32WebServer - https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
 
 Information from David Birds original code - not from My Circuits contribution (with my contribution/simplification to the code do whatever you wish, just mention us!):
  
 This software, the ideas and concepts is Copyright (c) David Bird 2018. All rights to this software are reserved.
 
 Any redistribution or reproduction of any part or all of the contents in any form is prohibited other than the following:
 1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
 2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
 3. You may not, except with my express written permission, distribute or commercially exploit the content.
 4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.

 The above copyright ('as annotated') notice and this permission notice shall be included in all copies or substantial portions of the Software and where the
 software use is visible to an end-user.
 
 THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY 
 OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 See more at http://www.dsbird.org.uk
 
*/

#include <WiFi.h>            //Built-in
#include <ESP32WebServer.h>  //https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <ESPmDNS.h>

#include "CSS.h"  //Includes headers of the web and de style file
#include <SD.h>
#include <SPI.h>

#include <math.h>

ESP32WebServer server(80);

#define servername "MCserver"  //Define the name to your server...
#define SD_pin 16              //G16 in my case
#define ButtonPin 4
#define dirPinstep  35
#define stepPinstep  33
#define dirPinheight  25
#define stepPinheight  32

bool SD_present = false;  //Controls if the SD card is present or not


bool last_button_state;
bool button_pressed=0;
int reading;
long lastDebounceTime;
int debounceDelay = 50;
bool buttonstate;

bool homepress=0;
int reading2;
int homestate;
int lasthomestate;

void detect_press() {
  // check if the button was pressed
  reading = digitalRead(ButtonPin);
  button_pressed = 0;
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != last_button_state) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonstate) {
      buttonstate = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonstate == HIGH) {
        button_pressed = 1;
        Serial.println("pressed");
      }
    }
  }
  last_button_state = reading;
}

void detect_press2() {
  // check if the button was pressed
  reading2 = digitalRead(26);
  homepress = 0;
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading2 != lasthomestate) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading2 != homestate) {
      homestate = reading2;

      // only toggle the LED if the new button state is HIGH
      if (homestate == HIGH) {
        homepress = 1;
        Serial.println("pressed");
      }
    }
  }
  last_button_state = reading;
}


/*********  SETUP  **********/

void setup(void) {
  Serial.begin(9600);
  WiFi.softAP("Scanner3D", "12345678");  //Network and password for the access point genereted by ESP32
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  pinMode(26,INPUT);
  pinMode(dirPinstep,OUTPUT);
  pinMode(stepPinstep,OUTPUT);
  pinMode(dirPinheight,OUTPUT);
  pinMode(stepPinheight,OUTPUT);
    //Set your preferred server name, if you use "mcserver" the address would be http://mcserver.local/
  if (!MDNS.begin(servername)) {
    Serial.println(F("Error setting up MDNS responder!"));
    ESP.restart();
  }

  Serial.print(F("Initializing SD card..."));

  //see if the card is present and can be initialised.
  //Note: Using the ESP32 and SD_Card readers requires a 1K to 4K7 pull-up to 3v3 on the MISO line, otherwise may not work
  if (!SD.begin(SD_pin)) {
    Serial.println(F("Card failed or not present, no SD Card data logging possible..."));
    SD_present = false;
  } else {
    Serial.println(F("Card initialised... file access enabled..."));
    SD_present = true;
  }

  /*********  Server Commands  **********/
  server.on("/", SD_dir);
  server.on("/upload", File_Upload);
  server.on(
    "/fupload", HTTP_POST, []() {
      server.send(200);
    },
    handleFileUpload);

  server.begin();

  Serial.println("HTTP server started");

  writeFile(SD, "/dados.txt", "");
}
int espacamento=150;
/*********  LOOP  **********/
char distancia;
int scan = 0;
void loop(void) {
  detect_press();
  
  
  if (button_pressed) {
    if (scan == 1) {
      scan = 0;
    } else {
      scan = 1;
    }
  }
  if (scan == 0) {
    server.handleClient();  //Listen for client connections
  }
  //Serial.println(measuredist())  ;
  else {
    while(homepress==0){
      
    digitalWrite(dirPinheight, 0);
        digitalWrite(stepPinheight, HIGH);
        delayMicroseconds(2000);
        digitalWrite(stepPinheight, LOW);
        delayMicroseconds(2000); 
      if(digitalRead(26)==1){
        Serial.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        homepress=1;
        break;
        }
    };
    
    digitalWrite(dirPinheight, 1);
    for (int altura = 0; altura < 100; altura++) {
      
      for (float steps = 0; steps < 200; steps++) {
        
        int dist=espacamento-measuredist();
        if(dist>=0&&dist<=150){
            Serial.print(dist);
             appendFile(SD, "/dados.txt",dist*cos(steps*0.0314),dist*sin(steps*0.0314),2*altura );
             if(altura==0){
              for(int i=0;i<dist;i=i+10){
                appendFile(SD, "/dados.txt",i*cos(steps*0.0314),i*sin(steps*0.0314),2*altura );
              }
             }
      
        } 
        digitalWrite(stepPinstep, HIGH);
        delayMicroseconds(2000);
        digitalWrite(stepPinstep, LOW);
        delayMicroseconds(2000);
        delay(100);

      }
      for(int i =0;i<200;i++){
        digitalWrite(stepPinheight, HIGH);
        delayMicroseconds(2000);
        digitalWrite(stepPinheight, LOW);
        delayMicroseconds(2000);      
        }
      //rodar o outro stepper
    }
   scan=0;Serial.print("done");
  }
}

/*********  FUNCTIONS  **********/
//Initial page of the server web, list directory and give you the chance of deleting and uploading
void SD_dir() {
  if (SD_present) {
    //Action acording to post, dowload or delete, by MC 2022
    if (server.args() > 0)  //Arguments were received, ignored if there are not arguments
    {
      Serial.println(server.arg(0));

      String Order = server.arg(0);
      Serial.println(Order);

      if (Order.indexOf("download_") >= 0) {
        Order.remove(0, 9);
        SD_file_download(Order);
        Serial.println(Order);
      }

      if ((server.arg(0)).indexOf("delete_") >= 0) {
        Order.remove(0, 7);
        SD_file_delete(Order);
        Serial.println(Order);
      }
    }

    File root = SD.open("/");
    if (root) {
      root.rewindDirectory();
      SendHTML_Header();
      webpage += F("<table align='center'>");
      webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
      printDirectory("/", 0);
      webpage += F("</table>");
      SendHTML_Content();
      root.close();
    } else {
      SendHTML_Header();
      webpage += F("<h3>No Files Found</h3>");
    }
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();  //Stop is needed because no content length was sent
  } else ReportSDNotPresent();
}

//Upload a file to the SD
void File_Upload() {
  append_page_header();
  webpage += F("<h3>Select File to Upload</h3>");
  webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
  webpage += F("<input class='buttons' style='width:25%' type='file' name='fupload' id = 'fupload' value=''>");
  webpage += F("<button class='buttons' style='width:10%' type='submit'>Upload File</button><br><br>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  server.send(200, "text/html", webpage);
}

//Prints the directory, it is called in void SD_dir()
void printDirectory(const char *dirname, uint8_t levels) {

  File root = SD.open(dirname);

  if (!root) {
    return;
  }
  if (!root.isDirectory()) {
    return;
  }
  File file = root.openNextFile();

  int i = 0;
  while (file) {
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    if (file.isDirectory()) {
      webpage += "<tr><td>" + String(file.isDirectory() ? "Dir" : "File") + "</td><td>" + String(file.name()) + "</td><td></td></tr>";
      printDirectory(file.name(), levels - 1);
    } else {
      webpage += "<tr><td>" + String(file.name()) + "</td>";
      webpage += "<td>" + String(file.isDirectory() ? "Dir" : "File") + "</td>";
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024) fsize = String(bytes) + " B";
      else if (bytes < (1024 * 1024)) fsize = String(bytes / 1024.0, 3) + " KB";
      else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
      else fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
      webpage += "<td>" + fsize + "</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>");
      webpage += F("<button type='submit' name='download'");
      webpage += F("' value='");
      webpage += "download_" + String(file.name());
      webpage += F("'>Download</button>");
      webpage += "</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>");
      webpage += F("<button type='submit' name='delete'");
      webpage += F("' value='");
      webpage += "delete_" + String(file.name());
      webpage += F("'>Delete</button>");
      webpage += "</td>";
      webpage += "</tr>";
    }
    file = root.openNextFile();
    i++;
  }
  file.close();
}

//Download a file from the SD, it is called in void SD_dir()
void SD_file_download(String filename) {
  if (SD_present) {
    File download = SD.open("/" + filename);
    if (download) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download");
  } else ReportSDNotPresent();
}

//Handles the file upload a file to the SD
File UploadFile;
//Upload a new file to the Filing system
void handleFileUpload() {
  HTTPUpload &uploadfile = server.upload();  //See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                             //For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
  if (uploadfile.status == UPLOAD_FILE_START) {
    String filename = uploadfile.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("Upload File Name: ");
    Serial.println(filename);
    SD.remove(filename);                         //Remove a previous version, otherwise data is appended the file again
    UploadFile = SD.open(filename, FILE_WRITE);  //Open the file for writing in SD (create it, if doesn't exist)
    filename = String();
  } else if (uploadfile.status == UPLOAD_FILE_WRITE) {
    if (UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize);  // Write the received bytes to the file
  } else if (uploadfile.status == UPLOAD_FILE_END) {
    if (UploadFile)  //If the file was successfully created
    {
      UploadFile.close();  //Close the file again
      Serial.print("Upload Size: ");
      Serial.println(uploadfile.totalSize);
      webpage = "";
      append_page_header();
      webpage += F("<h3>File was successfully uploaded</h3>");
      webpage += F("<h2>Uploaded File Name: ");
      webpage += uploadfile.filename + "</h2>";
      webpage += F("<h2>File Size: ");
      webpage += file_size(uploadfile.totalSize) + "</h2><br><br>";
      webpage += F("<a href='/'>[Back]</a><br><br>");
      append_page_footer();
      server.send(200, "text/html", webpage);
    } else {
      ReportCouldNotCreateFile("upload");
    }
  }
}

//Delete a file from the SD, it is called in void SD_dir()
void SD_file_delete(String filename) {
  if (SD_present) {
    SendHTML_Header();
    File dataFile = SD.open("/" + filename, FILE_READ);  //Now read data from SD Card
    if (dataFile) {
      if (SD.remove("/" + filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '" + filename + "' has been erased</h3>";
        webpage += F("<a href='/'>[Back]</a><br><br>");
      } else {
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='/'>[Back]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSDNotPresent();
}

//SendHTML_Header
void SendHTML_Header() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");  //Empty content inhibits Content-length header so we have to close the socket ourselves.
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}

//SendHTML_Content
void SendHTML_Content() {
  server.sendContent(webpage);
  webpage = "";
}

//SendHTML_Stop
void SendHTML_Stop() {
  server.sendContent("");
  server.client().stop();  //Stop is needed because no content length was sent
}

//ReportSDNotPresent
void ReportSDNotPresent() {
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//ReportFileNotPresent
void ReportFileNotPresent(String target) {
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>");
  webpage += F("<a href='/");
  webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//ReportCouldNotCreateFile
void ReportCouldNotCreateFile(String target) {
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>");
  webpage += F("<a href='/");
  webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//File size conversion
String file_size(int bytes) {
  String fsize = "";
  if (bytes < 1024) fsize = String(bytes) + " B";
  else if (bytes < (1024 * 1024)) fsize = String(bytes / 1024.0, 3) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
  else fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
  return fsize;
}

int measuredist() {
  int t = pulseIn(2, 1);
    //delayMicroseconds(1000);
  if (t == 0) {
    Serial.println("timeout");
  } 
  
  else if (t > 1850) {
    return -1;
  } 
  else {
    int d = (t - 1000) * 3 / 4;
    if (d < 0) { d = 0; }

    return d;
  }
}
void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}
void appendFile(fs::FS &fs, const char *path, float x,float y,float z) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(x,3)&&file.print(";")&&file.print(y,3)&&file.print(";")&&file.println(z,3)) {
    Serial.print(x,3);Serial.print(";");Serial.print(y,3);Serial.print(";");Serial.println(z,3);
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
