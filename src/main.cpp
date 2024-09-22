#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <math.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Define PIN
#define LAMPPIN 2
#define DHT11PIN 26
#define PUMPPIN 18
#define MOISTUREPIN 35
#define LDRPIN 34
#define ROOFPIN 5
#define FANPIN 13

LiquidCrystal_I2C lcd(0x27, 16, 2); 


//Network config
const char* ssid = "SmartGardenNet";
const char* password = "123456789";

IPAddress local_ip(192, 168, 49, 10);
IPAddress gateway(192, 168, 49, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

//Global Varible H : high L : Low
//SoidMoitureSensor1
volatile float soidMoisture1 = -1;
//SoidMoitureSensor2
volatile float soidMoisture2 = -1;
//SoidMoitureSensor3
volatile float soidMoisture3 = -1;
//SoidMoitureSensor4
volatile float soidMoisture4 = -1;

//DHT11
volatile float airTemp = -1;

volatile float airHumidity = -1;

//LightSensor
volatile float light = -1;

volatile boolean waterpumpStatus = false;
volatile boolean fanStatus = false;
volatile boolean growingLampStatus = false;
volatile boolean roofStatus = false;
volatile boolean securityStatus = false;


DHT dht(26, DHT22); // Adjust pin and DHT type accordingly


//Send HTML for Client
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <title>Smart Garden Controller</title>
        <style>
            * {
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }

            body {
                font-family: "Verdana", "Arial", sans-serif;
            }

            .header-container {
                padding: 10px;
                display: flex;
                justify-content: space-between;
                border-bottom: 3px solid black;
                flex-wrap: wrap;
            }

            .header-left h1 {
                font-size: 36px;
                font-weight: bold;
            }

            .header-right {
                display: flex;
                flex-wrap: wrap;
            }

            .header-right p {
                font-size: 18px;
                font-weight: bold;
            }

            .time {
                display: flex;
                flex-direction: column;
                margin-right: 20px;
            }

            .time-data {
                display: flex;
                flex-direction: column;
                align-items: flex-start;
            }
            .main .head {
                padding: 10px 0 10px 0;
                margin-left: 10px;
                font-size: 18px;
                font-weight: bold;
            }
            table {
                width: 80%;
                margin-left: 10%;
                margin-right: 10%;
            }

            table div {
                text-align: center;
            }
            table td {
                padding: 5px;
            }

            table th {
                padding: 5px;
            }

            .manual-control{
                display: flex;
                justify-content: flex-end;
                padding-right: 10%;
            }
            .configuration-container{
                width: 80%;
                padding-left: 10%;
                padding-right: 10%;
            }

            .configuration-container button{
                font-weight: bold;
                color: white;
                padding: 5px 10px;
                border-radius: 9px;
                background-color: #2196f3;
                border: none;
            }

            .switch {
                position: relative;
                display: inline-block;
                width: 50px;
                height: 22px;
            }

            .switch input {
                opacity: 0;
                width: 0;
                height: 0;
            }

            .slider {
                position: absolute;
                cursor: pointer;
                top: 0;
                left: 0;
                right: 0;
                bottom: 0;
                background-color: #ccc;
                -webkit-transition: 0.4s;
                transition: 0.4s;
            }

            .slider:before {
                position: absolute;
                content: "";
                height: 18px;
                width: 18px;
                left: 3px;
                bottom: 2px;
                background-color: white;
                -webkit-transition: 0.4s;
                transition: 0.4s;
            }

            input:checked + .slider {
                background-color: #2196f3;
            }

            input:focus + .slider {
                box-shadow: 0 0 1px #2196f3;
            }

            input:checked + .slider:before {
                -webkit-transform: translateX(26px);
                -ms-transform: translateX(26px);
                transform: translateX(26px);
            }

            /* Rounded sliders */
            .slider.round {
                border-radius: 34px;
            }

            .slider.round:before {
                border-radius: 50%;
            }

            #c-1 , #d-1 , #c-2, #d-2, #c-3 , #d-3{
                cursor: pointer;
            }

            #c-1:hover , #d-1 :hover, #c-2:hover, #d-2:hover, #c-3:hover , #d-3:hover{
                color: red;
            }

            @media (max-width: 768px) {
                .header-left h1 {
                    font-size: 24px;
                }

                .header-right p {
                    font-size: 16px;
                }

                .time,
                .time-data {
                    margin-right: 10px;
                }
            }
        </style>
    </head>
    <body>
        <header>
            <div class="header-container">
                <div class="header-left">
                    <h1>Smart Garden Controller</h1>
                </div>
                <div class="header-right">
                    <div class="time">
                        <p>DATE</p>
                        <p>ONTIME</p>
                    </div>
                    <div class="time-data">
                        <p>mm/dd/yyyy</p>
                        <p>00:00:00</p>
                    </div>
                </div>
            </div>
        </header>

        <div class="main">
            <div class="head">Sensor Data</div>
            <table>
                <colgroup>
                    <col
                        span="1"
                        style="
                            background-color: rgb(230, 230, 230);
                            width: 20%;
                            color: #000000;
                        "
                    />
                    <col
                        span="1"
                        style="
                            background-color: rgb(200, 200, 200);
                            width: 15%;
                            color: #000000;
                        "
                    />
                    <col
                        span="1"
                        style="
                            background-color: rgb(180, 180, 180);
                            width: 15%;
                            color: #000000;
                        "
                    />
                    <col
                        span="1"
                        style="
                            background-color: rgb(180, 180, 180);
                            width: 15%;
                            color: #000000;
                        "
                    />
                </colgroup>
                <col
                    span="2"
                    style="background-color: rgb(0, 0, 0); color: #ffffff"
                />
                <col
                    span="2"
                    style="background-color: rgb(0, 0, 0); color: #ffffff"
                />
                <col
                    span="2"
                    style="background-color: rgb(0, 0, 0); color: #ffffff"
                />
                <col
                    span="2"
                    style="background-color: rgb(0, 0, 0); color: #ffffff"
                />
                <tr>
                    <th colspan="1"><div class="heading">Sensor name</div></th>
                    <th colspan="1">
                        <div class="heading">Current Value</div>
                    </th>
                    <th colspan="1"><div class="heading">High</div></th>
                    <th colspan="1"><div class="heading">Low</div></th>
                </tr>
                <tr>
                    <td><div class="bodytext">Soid Moisture 1</div></td>
                    <td><div class="tabledata" id="b0"></div></td>
                    <td><div class="tabledata" id="c0"></div></td>
                    <td><div class="tabledata" id="d0"></div></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Soid Moisture 2</div></td>
                    <td><div class="tabledata" id="b1"></div></td>
                    <td><div class="tabledata" id="c1"></div></td>
                    <td><div class="tabledata" id="d1"></div></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Soid Moisture 3</div></td>
                    <td><div class="tabledata" id="b2"></div></td>
                    <td><div class="tabledata" id="c2"></div></td>
                    <td><div class="tabledata" id="d2"></div></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Soid Moisture 4</div></td>
                    <td><div class="tabledata" id="b3"></div></td>
                    <td><div class="tabledata" id="c3"></div></td>
                    <td><div class="tabledata" id="d3"></div></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Temperature</div></td>
                    <td><div class="tabledata" id="b4"></div></td>
                    <td><div class="tabledata" id="c4"></div></td>
                    <td><div class="tabledata" id="d4"></div></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Air Humidity</div></td>
                    <td><div class="tabledata" id="b5"></div></td>
                    <td><div class="tabledata" id="c5"></div></td>
                    <td><div class="tabledata" id="d5"></div></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Light Sensor</div></td>
                    <td><div class="tabledata" id="b6"></div></td>
                    <td><div class="tabledata" id="c6"></div></td>
                    <td><div class="tabledata" id="d6"></div></td>
                </tr>
            </table>
            <div class="head">Component Active Status</div>
            <table>
                <colgroup>
                    <col
                        span="1"
                        style="
                            background-color: rgb(230, 230, 230);
                            width: 20%;
                            color: #000000;
                        "
                    />
                    <col
                        span="1"
                        style="
                            background-color: rgb(200, 200, 200);
                            width: 15%;
                            color: #000000;
                        "
                    />
                    <col
                        span="1"
                        style="
                            background-color: rgb(180, 180, 180);
                            width: 15%;
                            color: #000000;
                        "
                    />
                    <col
                        span="1"
                        style="
                            background-color: rgb(180, 180, 180);
                            width: 15%;
                            color: #000000;
                        "
                    />
                </colgroup>
                <col
                    span="2"
                    style="background-color: rgb(0, 0, 0); color: #ffffff"
                />
                <col
                    span="2"
                    style="background-color: rgb(0, 0, 0); color: #ffffff"
                />
                <col
                    span="2"
                    style="background-color: rgb(0, 0, 0); color: #ffffff"
                />
                <col
                    span="2"
                    style="background-color: rgb(0, 0, 0); color: #ffffff"
                />
                <tr>
                    <th colspan="1"><div class="heading">Device Name</div></th>
                    <th colspan="1">
                        <div class="heading">Status</div>
                    </th>
                    <th colspan="1"><div class="heading">Click to ON</div></th>
                    <th colspan="1"><div class="heading">Click to OFF</div></th>
                </tr>
                <tr>
                    <td><div class="bodytext">Water Pump</div></td>
                    <td><div class="tabledata" id="b-1"></div></td>
                    <td id="c-1"></td>
                    <td id="d-1"></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Growing Lamp</div></td>
                    <td><div class="tabledata" id="b-2"></div></td>
                    <td id="c-2"></td>
                    <td id="d-2"></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Fan</div></td>
                    <td><div class="tabledata" id="b-3"></div></td>
                    <td id="c-3"></td>
                    <td id="d-3"></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Roof</div></td>
                    <td><div class="tabledata" id="b-4"></div></td>
                    <td id="c-4"></td>
                    <td id="d-4"></td>
                </tr>
                <tr>
                    <td><div class="bodytext">Security</div></td>
                    <td><div class="tabledata" id="b-5"></div></td>
                    <td id="c-5"></td>
                    <td id="d-5"></td>
                </tr>
            </table>
            <div class="manual-control">
                <p style="margin-right: 20px; font-weight: bold;">Allow manual control</p>
                <label class="switch">
                    <input type="checkbox" checked />
                    <span class="slider round"></span>
                </label>
            </div>
            <div class="head">Automatic Configuration</div>
            <div class="configuration-container">
                <form action="">
                    <label for="fname">Waterpump activate moisture value (Default:):</label>
                    <input type="number" id="fname" name="fname"><br><br>
                    <label for="lname">Fan activate temperature value (Default:):</label>
                    <input type="number" id="lname" name="lname"><br><br>
                    <label for="lname">Lamp activate light sensor value (Default:):</label>
                    <input type="number" id="lname" name="lname"><br><br>
                    <button type="submit">Save changes</button>
                  </form>
            </div>
        </div>
        <script>
            const soil1Current = document.getElementById("b0");
            const soil1High = document.getElementById("c0");
            const soil1Low = document.getElementById("d0");
        
            const soil2Current = document.getElementById("b1");
            const soil2High = document.getElementById("c1");
            const soil2Low = document.getElementById("d1");
        
            const soil3Current = document.getElementById("b2");
            const soil3High = document.getElementById("c2");
            const soil3Low = document.getElementById("d2");
        
            const soil4Current = document.getElementById("b3");
            const soil4High = document.getElementById("c3");
            const soil4Low = document.getElementById("d3");
        
            const temperatureCurrent = document.getElementById("b4");
            const temperatureHigh = document.getElementById("c4");
            const temperatureLow = document.getElementById("d4");
        
            const humidCurrent = document.getElementById("b5");
            const humidHigh = document.getElementById("c5");
            const humidLow = document.getElementById("d5");
        
            const lightCurrent = document.getElementById("b6");
            const lightHigh = document.getElementById("c6");
            const lightLow = document.getElementById("d6");
        
            const pumpStatus = document.getElementById("b-1");
            const pumpOn = document.getElementById("c-1");
            const pumpOff = document.getElementById("d-1");
        
            const lampStatus = document.getElementById("b-2");
            const lampOn = document.getElementById("c-2");
            const lampOff = document.getElementById("d-2");
        
            const fanStatus = document.getElementById("b-3");
            const fanOn = document.getElementById("c-3");
            const fanOff = document.getElementById("d-3");

            const roofStatus = document.getElementById("b-4");
            const roofOn = document.getElementById("c-4");
            const roofOff = document.getElementById("d-4");

            const securityStatus = document.getElementById("b-5");
            const securityOn = document.getElementById("c-5");
            const securityOff = document.getElementById("d-5");
        
            const sensorStatus = {
                airTemperature: { now: null, high: null, low: null },
                airHumidity: { now: null, high: null, low: null },
                light: { now: null, high: null, low: null },
                moisture1: { now: null, high: null, low: null },
                moisture2: { now: null, high: null, low: null },
                moisture3: { now: null, high: null, low: null },
                moisture4: { now: null, high: null, low: null }
            };
        
            const controlStatus = {                
                waterPump: null,
                growingLamp: null,
                fan: null,
                roof: null,
                security: null
            };

            function roundToTwoDecimalPlaces(num) {
                return parseFloat(num.toFixed(2));
            }
        
            function updateUI() {
                // Update sensor values
                humidCurrent.textContent = sensorStatus.airHumidity.now + " %";
                humidHigh.textContent = sensorStatus.airHumidity.high+ " %";
                humidLow.textContent = sensorStatus.airHumidity.low+ " %";
        
                temperatureCurrent.textContent = sensorStatus.airTemperature.now + " °C";
                temperatureHigh.textContent = sensorStatus.airTemperature.high+ " °C";
                temperatureLow.textContent = sensorStatus.airTemperature.low+ " °C";
        
                lightCurrent.textContent = sensorStatus.light.now;
                lightHigh.textContent = sensorStatus.light.high;
                lightLow.textContent = sensorStatus.light.low;
        
                soil1Current.textContent = sensorStatus.moisture1.now +" %";
                soil1High.textContent = sensorStatus.moisture1.high +" %";
                soil1Low.textContent = sensorStatus.moisture1.low+" %";
        
                soil2Current.textContent = sensorStatus.moisture2.now+" %";
                soil2High.textContent = sensorStatus.moisture2.high+" %";
                soil2Low.textContent = sensorStatus.moisture2.low+" %";
        
                soil3Current.textContent = sensorStatus.moisture3.now+" %";
                soil3High.textContent = sensorStatus.moisture3.high+" %";
                soil3Low.textContent = sensorStatus.moisture3.low+" %";
        
                soil4Current.textContent = sensorStatus.moisture4.now+" %";
                soil4High.textContent = sensorStatus.moisture4.high+" %";
                soil4Low.textContent = sensorStatus.moisture4.low+" %";
        
                // Update control statuses
                pumpStatus.textContent = controlStatus.waterPump ? "ON" : "OFF";
                pumpStatus.style.backgroundColor = controlStatus.waterPump ? "green" : "red";
        
                lampStatus.textContent = controlStatus.growingLamp ? "ON" : "OFF";
                lampStatus.style.backgroundColor = controlStatus.growingLamp ? "green" : "red";
        
                fanStatus.textContent = controlStatus.fan ? "ON" : "OFF";
                fanStatus.style.backgroundColor = controlStatus.fan ? "green" : "red";

                roofStatus.textContent = roofStatus.fan ? "ON" : "OFF";
                roofStatus.style.backgroundColor = roofStatus.fan ? "green" : "red";

                securityStatus.textContent = securityStatus.fan ? "ON" : "OFF";
                securityStatus.style.backgroundColor = securityStatus.fan ? "green" : "red";
            }
        
            function fetchData() {
                fetch("/data")
                    .then(response => response.json())
                    .then(data => {
                        // Update sensor values
                        sensorStatus.airHumidity.now = roundToTwoDecimalPlaces(data.airHumidity);
                        if (sensorStatus.airHumidity.high < sensorStatus.airHumidity.now || sensorStatus.airHumidity.high === null) {
                            sensorStatus.airHumidity.high = sensorStatus.airHumidity.now;
                        }
                        if (sensorStatus.airHumidity.low > sensorStatus.airHumidity.now || sensorStatus.airHumidity.low === null) {
                            sensorStatus.airHumidity.low = sensorStatus.airHumidity.now;
                        }
        
                        sensorStatus.airTemperature.now = roundToTwoDecimalPlaces(data.airTemperature);
                        if (sensorStatus.airTemperature.high < sensorStatus.airTemperature.now || sensorStatus.airTemperature.high === null) {
                            sensorStatus.airTemperature.high = sensorStatus.airTemperature.now;
                        }
                        if (sensorStatus.airTemperature.low > sensorStatus.airTemperature.now || sensorStatus.airTemperature.low === null) {
                            sensorStatus.airTemperature.low = sensorStatus.airTemperature.now;
                        }
        
                        sensorStatus.light.now = roundToTwoDecimalPlaces( data.light);
                        if (sensorStatus.light.high < sensorStatus.light.now || sensorStatus.light.high === null) {
                            sensorStatus.light.high = sensorStatus.light.now;
                        }
                        if (sensorStatus.light.low > sensorStatus.light.now || sensorStatus.light.low === null) {
                            sensorStatus.light.low = sensorStatus.light.now;
                        }
        
                        sensorStatus.moisture1.now = roundToTwoDecimalPlaces(data.moisture1);
                        if (sensorStatus.moisture1.high < sensorStatus.moisture1.now || sensorStatus.moisture1.high === null) {
                            sensorStatus.moisture1.high = sensorStatus.moisture1.now;
                        }
                        if (sensorStatus.moisture1.low > sensorStatus.moisture1.now || sensorStatus.moisture1.low === null) {
                            sensorStatus.moisture1.low = sensorStatus.moisture1.now;
                        }
        
                        sensorStatus.moisture2.now = roundToTwoDecimalPlaces(data.moisture2);
                        if (sensorStatus.moisture2.high < sensorStatus.moisture2.now || sensorStatus.moisture2.high === null) {
                            sensorStatus.moisture2.high = sensorStatus.moisture2.now;
                        }
                        if (sensorStatus.moisture2.low > sensorStatus.moisture2.now || sensorStatus.moisture2.low === null) {
                            sensorStatus.moisture2.low = sensorStatus.moisture2.now;
                        }
        
                        sensorStatus.moisture3.now = roundToTwoDecimalPlaces(data.moisture3);
                        if (sensorStatus.moisture3.high < sensorStatus.moisture3.now || sensorStatus.moisture3.high === null) {
                            sensorStatus.moisture3.high = sensorStatus.moisture3.now;
                        }
                        if (sensorStatus.moisture3.low > sensorStatus.moisture3.now || sensorStatus.moisture3.low === null) {
                            sensorStatus.moisture3.low = sensorStatus.moisture3.now;
                        }
        
                        sensorStatus.moisture4.now = roundToTwoDecimalPlaces(data.moisture4);
                        if (sensorStatus.moisture4.high < sensorStatus.moisture4.now || sensorStatus.moisture4.high === null) {
                            sensorStatus.moisture4.high = sensorStatus.moisture4.now;
                        }
                        if (sensorStatus.moisture4.low > sensorStatus.moisture4.now || sensorStatus.moisture4.low === null) {
                            sensorStatus.moisture4.low = sensorStatus.moisture4.now;
                        }
        
                        // Update control statuses
                        controlStatus.waterPump = data.pumpSts;
                        controlStatus.fan = data.fanSts;
                        controlStatus.growingLamp = data.lampSts;
                        controlStatus.roof = data.roofSts;
                        controlStatus.security = data.securitySts;
        
                        // Update UI
                        updateUI();
                    });
            }
        
            lampOn.addEventListener("click", () => {
                fetch("/lamp-on")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.growingLamp = true;
                            updateUI();
                        }
                    });
            });
        
            lampOff.addEventListener("click", () => {
                fetch("/lamp-off")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.growingLamp = false;
                            updateUI();
                        }
                    });
            });

            pumpOn.addEventListener("click", () => {
                fetch("/pump-on")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.waterPump = true;
                            updateUI();
                        }
                    });
            });
        
            pumpOff.addEventListener("click", () => {
                fetch("/pump-off")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.waterPump = false;
                            updateUI();
                        }
                    });
            });


            fanOn.addEventListener("click", () => {
                fetch("/fan-on")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.fan = true;
                            updateUI();
                        }
                    });
            });
        
            fanOff.addEventListener("click", () => {
                fetch("/fan-off")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.fan = false;
                            updateUI();
                        }
                    });
            });

            roofOn.addEventListener("click", () => {
                fetch("/roof-on")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.roof = true;
                            updateUI();
                        }
                    });
            });
        
            roofOff.addEventListener("click", () => {
                fetch("/roof-off")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.roof = false;
                            updateUI();
                        }
                    });
            });

            securityOn.addEventListener("click", () => {
                fetch("/security-on")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.security = true;
                            updateUI();
                        }
                    });
            });
        
            securityOff.addEventListener("click", () => {
                fetch("/security-off")
                    .then(response => response.text())
                    .then(text => {
                        if (text == "OK") {
                            controlStatus.security = false;
                            updateUI();
                        }
                    });
            });
        
            setInterval(fetchData, 1000); // Fetch live data every second
            fetchData();
        </script>        
    </body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}



void handleLampOnReq(){
      //TODO bật tắt chân
    digitalWrite(LAMPPIN, HIGH);
    growingLampStatus = true;
    Serial.print("LampStatus: ");
    Serial.println(growingLampStatus ? "true" : "false");
    server.send(200, "text/plain", "OK");
  
}

void handleLampOffReq(){
      //TODO bật tắt chân
    digitalWrite(LAMPPIN, LOW);
    growingLampStatus = false;
    Serial.print("LampStatus: ");
    Serial.println(growingLampStatus ? "true" : "false");
    server.send(200, "text/plain", "OK");

}

void handlePumpOnReq(){
  //TODO bật tắt chân
  digitalWrite(PUMPPIN, HIGH);
  waterpumpStatus = true;
  Serial.print("waterpumpStatus: ");
  Serial.println(waterpumpStatus ? "true" : "false");
  server.send(200, "text/plain", "OK");
}

void handlePumpOffReq(){
  //TODO bật tắt chân
  digitalWrite(PUMPPIN, LOW);
  waterpumpStatus = false;
  Serial.print("waterpumpStatus: ");
  Serial.println(waterpumpStatus ? "true" : "false");
  server.send(200, "text/plain", "OK");
}

void handleRoofOnReq(){
  //TODO bật tắt chân
  digitalWrite(ROOFPIN, HIGH);
  roofStatus = true;
  Serial.print("roofStatus: ");
  Serial.println(roofStatus ? "true" : "false");
  server.send(200, "text/plain", "OK");
}

void handleRoofOffReq(){
    //TODO bật tắt chân
  digitalWrite(ROOFPIN, LOW);
  roofStatus = false;
  Serial.print("roofStatus: ");
  Serial.println(roofStatus ? "true" : "false");
  server.send(200, "text/plain", "OK");
}

void handleFanOnReq(){
    //TODO bật tắt chân
  digitalWrite(FANPIN, HIGH);
  fanStatus = true;
  Serial.print("fanStatus: ");
  Serial.println(fanStatus ? "true" : "false");
  server.send(200, "text/plain", "OK");
}

void handleFanOffReq(){
    //TODO bật tắt chân
  digitalWrite(FANPIN, LOW);
  fanStatus = false;
  Serial.print("fanStatus: ");
  Serial.println(fanStatus ? "true" : "false");
  server.send(200, "text/plain", "OK");
}

void handleSecurityOnReq(){
    //TODO bật tắt chân

  securityStatus = true;
  Serial.print("securityStatus: ");
  Serial.println(securityStatus ? "true" : "false");
  server.send(200, "text/plain", "OK");
}

void handleSecurityOffReq(){
    //TODO bật tắt chân

    fanStatus = false;
  Serial.print("securityStatus: ");
  Serial.println(securityStatus ? "true" : "false");
  server.send(200, "text/plain", "OK");
}


void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}


void sendData() {
  StaticJsonDocument<200> doc;
  doc["airHumidity"] = airHumidity;
  doc["airTemperature"] = airTemp;
  doc["light"] = light;
  doc["moisture1"] = soidMoisture1;
  doc["moisture2"] = soidMoisture2;
  doc["moisture3"] = soidMoisture3;
  doc["moisture4"] = soidMoisture4;

  doc["pumpSts"] = waterpumpStatus;
  doc["fanSts"] = fanStatus;
  doc["lampSts"] = growingLampStatus;
  doc["roofSts"] = roofStatus;
  doc["securitySts"] = securityStatus;
  //continue

  String json;
  serializeJson(doc, json);

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  pinMode(LAMPPIN, OUTPUT);
  pinMode(LDRPIN, INPUT);

  lcd.init();
  lcd.backlight();

  
  lcd.setCursor(0, 0);
  lcd.print("Hello, World!");

  lcd.setCursor(0, 1);
  lcd.print("ESP32 I2C LCD");

  WiFi.softAP(ssid, password);
  delay(100);

  dht.begin();
  delay(2000);

  if (!WiFi.softAPConfig(local_ip, gateway, subnet)) {
    Serial.println("Failed to configure soft AP");
  }

  server.on("/", handleRoot);

  server.on("/pump-on", handlePumpOnReq);
  server.on("/pump-off", handlePumpOffReq);

  server.on("/lamp-on", handleLampOnReq);
  server.on("/lamp-off", handleLampOffReq);

    server.on("/fan-on", handleFanOnReq);
  server.on("/fan-off", handleFanOffReq);

  server.on("/roof-on", handleRoofOnReq);
  server.on("/roof-off", handleRoofOffReq);

  server.on("/security-on", handleSecurityOnReq);
  server.on("/security-off", handleSecurityOffReq);

  server.on("/data",sendData);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Server started");

  

}


void loop() {
  server.handleClient();
  airTemp = dht.readTemperature();
  airHumidity = dht.readHumidity();
  light = analogRead(LDRPIN);
//   if (light < 100) {
//     growingLampStatus = true;
//     digitalWrite(LAMPPIN, HIGH);
//   } else {
//     growingLampStatus = false;
//     digitalWrite(LAMPPIN, LOW);
//   }
}
