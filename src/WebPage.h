#ifndef WebPage_h
#define WebPage_h

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include "NVMData.h"
#include "DynamicData.h"

class WebPage
{
private:
    WebServer server;
    String GenHeader(int redirectTime);
    String GenFooter();
    String GenTableStart();
    String GenTableNewColumn();
    String GenTableRows(String Content[], int Count);
    String GenTableEnd();
    String addKeyValuePair(String key, String value);
    void handleRoot();
    void handleChange();
    void handleStop();
    void handleStart();
    void handleEBrake();
    void handleNotFound();
    void handleFirmware();
    void handleUpload();
    void handleUpload2();
    void handlePowerSource();
    void handlePowerSource2();
    void handleJson();
    String setMessage(String arg, String value, String result);
public:
    WebPage();
    ~WebPage();
    void Init();
    void loop();
    bool newNetworkSet = false;
};
WebPage::WebPage()
{
}
WebPage::~WebPage()
{
}
WebServer server(80);
void WebPage::loop() {
  server.handleClient();
  if(DynamicData::get().setNewNetwork == true)
  {
    if(newNetworkSet == true)
    {
      DynamicData::get().setNewNetwork = false;
      newNetworkSet = false;
      NVMData::get().StoreNetData();
    }
  }
  else
  {
    DynamicData::get().setNewNetwork = false;
  }
}
void WebPage::Init() {
    
    server.on("/", std::bind(&WebPage::handleRoot, this));
    server.on("/change", std::bind(&WebPage::handleChange, this));
    server.on("/stop", std::bind(&WebPage::handleStop, this));
    server.on("/start", std::bind(&WebPage::handleStart, this));
    server.on("/ebrake", std::bind(&WebPage::handleEBrake, this));
    server.on("/json", std::bind(&WebPage::handleJson, this));
    // choose bin file
    server.on("/firmware", HTTP_GET, std::bind(&WebPage::handleFirmware, this));
    /*handling uploading firmware file */
    server.on("/update", HTTP_POST, std::bind(&WebPage::handleUpload, this), std::bind(&WebPage::handleUpload2, this));
    server.onNotFound(std::bind(&WebPage::handleNotFound, this));
    server.begin();
    Serial.println("HTTP server started");
}
String WebPage::GenHeader(int redirectTime)
{
  String message= "";
  message += "<html>";
  message += "<head>";
  if (redirectTime > 0)
  {
    String redirectTimeString = String(redirectTime);
    message += "<meta http-equiv=\"refresh\" content=\"" + redirectTimeString + ";url=http://" + DynamicData::get().ipaddress + "/\" />";
  }
  message += "</head>";
  message += "<body>\n";
  message += "\t<p>This may only work until 19.01.2038</p>\n";
  message += "\t<p>------------V2.1.0---------------</p>\n";
  return message;
}
String WebPage::GenFooter()
{
  String message= "";
  message += "\t<p>---------------------------</p>\n";
  message += "</body>";
  message += "</html>\n";
  return message;
}
String WebPage::GenTableStart()
{
  String message= "";
  message += "\t<table border=\"4\">";
  message += "\t<tr>\n";
  return message;
}
String WebPage::GenTableNewColumn()
{
  String message= "";
  message += "\t</tr>";
  message += "\t<tr>\n";
  return message;
}
String WebPage::GenTableRows(String Content[], int Count)
{
  String message = "";
  for (int i = 0; i < Count; i++)
  {
    message += "<td>"+Content[i]+"</td>";
  }
  message +="\n";
  return message;
}
String WebPage::GenTableEnd()
{
  String message= "";
  message += "\t</tr>";
  message += "\t</table>\n";
  return message;
}
void WebPage::handleNotFound() {
  String message= "";
  message += GenHeader(3);
  message += "File Not Found\n\n";
  message += GenFooter();
  server.send(200, "text/html", message);
}
void WebPage::handleStop() {
  DynamicData::get().timerOn = false;
  handleRoot();
}
void WebPage::handleStart() {
  DynamicData::get().timerOn = true;
  handleRoot();
}
void WebPage::handleEBrake() {
  DynamicData::get().counter = 0;
  handleRoot();
}
void WebPage::handleChange() {
  String message = "Ohh oh!\n\n";
  bool netNameSet = false;
  bool netPasswordSet = false;
  String netName = "";
  String netPassword = "";
  DynamicData::DriveData driveData;
  DynamicData::Direction direction;
  bool timeSet = false;
  bool powerLeftSet = false;
  bool powerRightSet = false;
  for (uint8_t i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "netname") {
      netNameSet = true;
      netName = server.arg(i);
    } else if (server.argName(i) == "password") {
      netPasswordSet = true;
      netPassword = server.arg(i);
    } else if (server.argName(i) == "time") {
      driveData.time = server.arg(i).toInt();
      Serial.println(driveData.time);
      Serial.println(server.arg(i));
      timeSet = true;
    } else if (server.argName(i) == "leftp") {
      driveData.leftpower = server.arg(i).toInt();
      powerLeftSet = true;
      Serial.println(driveData.leftpower);
      Serial.println(server.arg(i));
    } else if (server.argName(i) == "rightp") {
      driveData.rightpower = server.arg(i).toInt();
      powerRightSet = true;
      Serial.println(driveData.rightpower);
      Serial.println(server.arg(i));
    }
  }
  if ((netPasswordSet == true)&&(netNameSet == true))
  {
    newNetworkSet = true;
    NVMData::get().SetNetData(netName, netPassword);
    message = "You got it!";
  }
  if ((timeSet) && (powerLeftSet) && (powerRightSet)) {
    int writeP = DynamicData::get().writePointer;
    int newWriteP = writeP + 1;
    if(newWriteP == DynamicData::numberOfDriveData) {
      newWriteP = 0;
    }
    if(newWriteP != DynamicData::get().readPointer) {
      DynamicData::get().driveData[writeP].time = driveData.time;
      DynamicData::get().driveData[writeP].leftpower = driveData.leftpower;
      DynamicData::get().driveData[writeP].rightpower = driveData.rightpower;
      DynamicData::get().writePointer = newWriteP;
      message = "new data set!";
    } else {
      message = "buffer full";
    }
  }
  String returnMessage= "";
  returnMessage += GenHeader(1);
  returnMessage += message;
  returnMessage += "</body>";
  returnMessage += "</html>";
  server.send(200, "text/html", returnMessage);
}
void WebPage::handleFirmware() {
  const char* serverIndex = 
    "<script src='https://sciphy.de/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
      "<input type='file' name='update'>"
            "<input type='submit' value='Update'>"
        "</form>"
    "<div id='prg'>progress: 0%</div>"
    "<script>"
      "$('form').submit(function(e){"
      "e.preventDefault();"
      "var form = $('#upload_form')[0];"
      "var data = new FormData(form);"
      " $.ajax({"
      "url: '/update',"
      "type: 'POST',"
      "data: data,"
      "contentType: false,"
      "processData:false,"
      "xhr: function() {"
      "var xhr = new window.XMLHttpRequest();"
      "xhr.upload.addEventListener('progress', function(evt) {"
      "if (evt.lengthComputable) {"
      "var per = evt.loaded / evt.total;"
      "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
      "}"
      "}, false);"
      "return xhr;"
      "},"
      "success:function(d, s) {"
      "console.log('success!')" 
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>";
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", serverIndex);
}
void WebPage::handleUpload() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  ESP.restart();
}
void WebPage::handleUpload2() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    /* flashing firmware to ESP*/
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { //true to set the size to the current progress
      Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}
void WebPage::handleRoot() {
  String uri = DynamicData::get().ipaddress;
  String message = "";
  message += GenHeader(0);
  
  if( DynamicData::get().setNewNetwork == true )
  {
    message += "\t<form action=\"change\">\n";
    message += "\t<label class=\"h2\" form=\"networkdata\">Network name</label>\n";
    message += "\t<div>\n";
    message += "\t<label for=\"netname\">netname</label>\n";
    message += "\t<input type=\"text\" name=\"netname\" maxlength=\"30\">\n";
    message += "\t</div>\n";
    message += "\t<div>\n";
    message += "\t<label for=\"password\">password</label>\n";
    message += "\t<input type=\"text\" name=\"password\" maxlength=\"40\">\n";
    message += "\t</div>\n";
    message += "\t<div>\n";
    message += "\t<button type=\"reset\">clear</button>\n";
    message += "\t<button type=\"submit\">set</button>\n";
    message += "\t</div>\n";
    message += "\t</form>\n";
  }
  message += "\t<form action=\"change\">\n";
  message += "\t<label class=\"h2\" form=\"scommand\">set command</label>\n";
  message += "\t<div>\n";
  message += "\t<label>time</label>\n";
  message += "\t<input type=\"text\" name=\"time\" maxlength=\"100\">\n";
  message += "\t</div>\n";
  message += "\t<div>\n";
  message += "\t<label>left power</label>\n";
  message += "\t<input type=\"text\" name=\"leftp\" maxlength=\"10\">\n";
  message += "\t</div>\n";
  message += "\t<div>\n";
  message += "\t<label>right power</label>\n";
  message += "\t<input type=\"text\" name=\"rightp\" maxlength=\"10\">\n";
  message += "\t</div>\n";
  message += "\t<div>\n";
  message += "\t<button type=\"reset\">clear</button>\n";
  message += "\t<button type=\"submit\">set</button>\n";
  message += "\t</div>\n";
  message += "\t</form>\n";
  message += "\t<p>---------------------------</p>\n";
  message += "\t<form action=\"activation\">\n";
  if(DynamicData::get().timerOn) {
    message += "\t<label class=\"h2\" form=\"activation\">timer status on </label>\n";
  } else {
    message += "\t<label class=\"h2\" form=\"activation\">timer status off </label>\n";
  }
  message += "\t<div>\n";
  message += "\t<button type=\"submit\" formaction=\"stop\">stop</button>\n";
  message += "\t<button type=\"submit\" formaction=\"start\">start</button>\n";
  message += "\t<button type=\"submit\" formaction=\"ebrake\">emergency brake</button>\n";
  message += "\t</div>\n";
  message += "\t</form>\n";
  message += "\t<p>---------------------------</p>\n";
  message += "\t<p>Time: " + DynamicData::get().getFormattedTime() + "</p>\n";
  message += "\t<p>ipaddress:  " + DynamicData::get().ipaddress + "</p>\n";
  message += "\t<p>errorCounter: " + String(DynamicData::get().getErrorCounter()) + "</p>\n";
  for (int i = 0; i < DynamicData::get().numberOfErrorMessageHist; i++)
  {
    message += "\t<p>errorHistory: " + DynamicData::get().getErrorHist(i) + "</p>\n";
  }
  message += "\t<p>operatedHours: " + String(NVMData::get().getOHC()) + "</p>\n";
  message += "\t<p>uptimeHours: " + String(DynamicData::get().uptimeHours) + "</p>\n";
  message += "\t<p>connections: " + String(DynamicData::get().connections) + "</p>\n";
  message += "\t<p>RSSIText: " + DynamicData::get().RSSIText + "</p>\n";
  message += "\t<p>---------------------------</p>\n";
  message += "\t<p>readPointer        : " + String(DynamicData::get().readPointer) + "</p>\n";
  message += "\t<p>writePointer       : " + String(DynamicData::get().writePointer) + "</p>\n";
  message += "\t<p>counter            : " + String(DynamicData::get().counter) + "</p>\n";
  message += GenFooter();
  server.send(200, "text/html", message);
}

String WebPage::addKeyValuePair(String key, String value)
{
  String retVal = "";
  retVal += "\"";
  retVal += key;
  retVal += "\" :\"";
  retVal += value;
  retVal += "\"";
  return retVal;
}
void WebPage::handleJson()
{
  String message = "";
  String key = "";
  message += "{ ";
  
  message += "}";
  server.send(200, "text/plain", message);
}
#endif