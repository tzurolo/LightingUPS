var fs = require('fs');
var express = require('express');
var app = express();

var childProcess = require('child_process');
var execFile = childProcess.execFile;

var http = require('http');
var socketIo = require('socket.io');

var httpServer = http.createServer(app);
var io = socketIo(httpServer);

var SerialPort = require("serialport");
var serialPort = new SerialPort("COM5", {
  baudRate: 300
});

var serialPortRxLine = '';
var serialPortRxLineNum = 0;
var serialPortTimeoutTimer = null;
var cmdQueue = [];
var currentSWVer = null;
var currentSettings = null;
var currentTCalOffset = null;

var savedSettings = null;
var savedTCalOffset = null;

var Mode = {
    Monitoring : 'Monitoring',
    Reprogramming : 'Reprogramming',
    Exiting : 'Exiting'
};
var currentMode = Mode.Monitoring;

 function sendCommandToMicrocontroller (cmd) {
    serialPort.write(cmd + "\r");
    // start timeout timer
    serialPortTimeoutTimer = setTimeout(serialPortTimedOut, 3000);
 }

// requests info from the attached unit by queueing commands
function requestUnitInfo ()
{
    cmdQueue.push("ver");
    cmdQueue.push("settings");
    cmdQueue.push("get tcaloffset");
    cmdQueue.push("status");
    sendCommandToMicrocontroller(cmdQueue[0]);
}

function serialPortTimedOut ()
{
    console.log('Serial port timeout');
    switch (currentMode) {
        case Mode.Monitoring :
            serialPortRxLine = '';
            if ((cmdQueue.length == 0) ||
                ((cmdQueue.length == 1) && (cmdQueue[0] == 'status'))) {
                // we were only requesting status. request all info
                cmdQueue = [];
                requestUnitInfo();
            } else if (cmdQueue.length > 0) {
                // there are commands in the queue - retry sending
                sendCommandToMicrocontroller(cmdQueue[0]);
            }
            if (currentSettings !== null) {
                // clear out old settings
                currentSWVer = null;
                currentSettings = null;
                currentTCalOffset = null;
                // blank out UI fields
                sendSWVersionToClient(currentSWVer);
                sendSettingsToClient(currentSettings);
                sendTCalOffsetToClient(currentTCalOffset);
            }
            break;
        case Mode.Exiting :
            serialPort.close();
            break;
        default :
            break;
    }
}

//
//  routes
//
app.get('/', function(req, res){
    res.sendFile(__dirname + '/index.html');
});
app.get('/home', function(req, res){
    res.sendFile(__dirname + '/home.html');
});
app.get('/status', function(req, res) {
    queryStatus(res);
});
app.get('/w3.css', function(req, res){
    res.sendFile(__dirname + '/w3.css');
});

// convert temperature from Centigrade to Fahrenheit
function convertTempToF (
    degC)
{
    var degF = ((parseFloat(degC) * 9.0) / 5.0) + 32.0;
    return degF.toFixed(1) + '&#x2109';
}

function sendEventToUI (
    eventName,
    eventValue)
{
    var eventJSON =
        '{ "type": "event", "data": ' + '{' +
           '"name": "' + eventName + '", ' +
           '"value": "' + eventValue + '"}' +
        '}';
    io.emit('controllerMessage', eventJSON);
}

function defaultSettings ()
{
    return {
            "ID": "",
            "Mode": "",
            "Dark": "",
            "Auto": "",
            "Manual": ""
        };
}

function sendSettingsToClient (
    settings)
{
    if (settings == null) {
        settings = defaultSettings();
    }
    sendEventToUI('ID', settings.ID);
    sendEventToUI('Mode', settings.Mode);
    sendEventToUI('Dark', settings.Dark);
    sendEventToUI('Auto', settings.Auto);
    sendEventToUI('Manual', settings.Manual);
}

function sendSWVersionToClient (
    swver)
{
    if (swver == null) {
        swver = '';
    }
    sendEventToUI('swver', swver);
}

function sendTCalOffsetToClient (
    tcaloffset)
{
    if (tcaloffset == null) {
        tcaloffset = '';
    }
    sendEventToUI('Tcaloffset', tcaloffset);
}

function commenceReprogramming ()
{
    // save current settings to restore after reprogramming
    savedSettings = currentSettings;
    savedTCalOffset = currentTCalOffset;

    //  child process for re-flashing firmware with avrdude
    var child = execFile('avrdude', [
        '-p', 'attiny84',
        '-P', 'COM4',
        '-c', 'avrispv2',
        '-e',
        '-U', 'flash:w:C:\\files\\LightingUPS\\firmware\\default\\LightingUPS.hex:i'
        ]);
    child.stdin.setEncoding('utf-8');

    child.stdout.on('data', function (data) {
        console.log('child stdout: "' + data + '"');
    });

    child.stderr.on('data', function (data) {
        var logmsg = '';
        for (i = 0; i < data.length; ++i) {
            var c = data.charCodeAt(i);
            if ((c < 32) || (c > 126)) {
                logmsg += ('(' + c + ')');
            } else {
                logmsg += data[i];
            }
        }
        console.log('child stderr: "' + logmsg + '"');
    });

    child.on('close', function () {
        console.log('child exited');

        // reestablish communications with unit
        serialPortRxLine = '';
        cmdQueue.push("ver");

        // restore saved settings
        if (savedSettings !== null) {
            cmdQueue.push("set id " + savedSettings.ID);
            cmdQueue.push("set mode " + savedSettings.Mode);
            cmdQueue.push("set dark " + savedSettings.Dark);
            cmdQueue.push("set auto " + savedSettings.Auto);
            cmdQueue.push("set manual " + savedSettings.Manual);
        }
        if (savedTCalOffset !== null) {
            cmdQueue.push("set tcaloffset " + savedTCalOffset);
        }

        // read settings afresh from  unit
        cmdQueue.push("settings");
        cmdQueue.push("get tcaloffset");
        cmdQueue.push("status");
        currentMode = Mode.Monitoring;
        sendCommandToMicrocontroller(cmdQueue[0]);
    });
}

function processMicrocontrollerMessage (
    message)
{
    clearTimeout(serialPortTimeoutTimer);
    var responseIsComplete = true;  // false indicates multi-line response
    var gotExpectedResponse = true; // empty message is a valid response
    if (message.length > 0) {
        switch (message[0]) {
            case 'O' :
                // OK
                break;
            case 'U' :
                // response to status query
                console.log('got status string');
                var tokens = message.trim().split(/\s+/);
                sendEventToUI('UPS', tokens[0]);
                sendEventToUI('BV', tokens[1]);
                sendEventToUI('AC', tokens[2]);
                sendEventToUI('C', tokens[3]);
                sendEventToUI('Light', tokens[4]);
                sendEventToUI('Motion', tokens[5]);
                if (tokens.length > 6) {
                    // V1.0 firmware did not use temperature sensor
                    sendEventToUI('Temp', tokens[6]);
                }
                break;
            case 'V' :
                currentSWVer = message;
                console.log('sw version: ' + currentSWVer);
                sendSWVersionToClient(currentSWVer);
                break;
            case '{' :
                console.log('receiving settings');
                if (message.length == 1) {
                    // firmware V1.0 format settings
                    currentSettings = defaultSettings();
                    responseIsComplete = false;
                } else {
                    if (message[message.length-1] == '}') {
                        currentSettings = JSON.parse(message);
                        sendSettingsToClient(currentSettings);
                        console.log('end of settings');
                    } else {
                        gotExpectedResponse = false;
                    }
                }
                break;
            case ' ' :
                var tokens = message.trim().split(/\s+/);
                if (tokens.length == 2) {
                    // V1.0 format setting
                    responseIsComplete = false;
                    console.log('got V1.0 setting:' + message);
                    switch (tokens[0]) {
                        case 'ID:'     : currentSettings.ID     = tokens[1]; break;
                        case 'Mode:'   : currentSettings.Mode   = tokens[1]; break;
                        case 'Dark:'   : currentSettings.Dark   = tokens[1]; break;
                        case 'Auto:'   : currentSettings.Auto   = tokens[1]; break;
                        case 'Manual:' : currentSettings.Manual = tokens[1]; break;
                        case 'offset:' : currentTCalOffset      = tokens[1];
                                         responseIsComplete = true;
                                         sendTCalOffsetToClient(currentTCalOffset);
                                         break;
                        default        : console.log('unexpected setting name: "' + tokens[0] + '"');
                    }
                }
                break;
            case '}' :
                // end of V1.0 format settings
                console.log('end of settings');
                sendSettingsToClient(currentSettings);
                break;
            case 't' :
                var tokens = message.trim().split(/\s+/);
                currentTCalOffset = tokens[1];
                console.log('temp cal offset ' + currentTCalOffset);
                sendTCalOffsetToClient(currentTCalOffset);
                break;
            case 'u' :
                // unrecognized command
                if (cmdQueue[0] == 'ver') {
                    // probably a unit with V1.0 firmware
                    console.log('ver unrecognized - assuming V1.0');
                    currentSWVer = 'V1.0';
                    sendSWVersionToClient(currentSWVer);
                } else if (cmdQueue[0] == 'get tcaloffset') {
                    // probably a unit with V1.0 firmware
                    console.log('get tcaloffset unrecognized');
                    currentTCalOffset = null;
                } else {
                    gotExpectedResponse = false;
                }
                break;
            default :
                gotExpectedResponse = false;
                break;
        }
    }
    if (responseIsComplete) {
        if (gotExpectedResponse) {
            cmdQueue.shift();
        } else {
            console.log('retrying ' + cmdQueue[0]);
        }
        if (cmdQueue.length == 0) {
            // finished all queued commands.
            // determine what to do next
            switch (currentMode) {
                case Mode.Monitoring :
                    // request status
                    cmdQueue.push('status');
                    break;
                case Mode.Reprogramming :
                    // initiate reprogramming
                    console.log('commence reprogramming...');
                    clearTimeout(serialPortTimeoutTimer);
                    commenceReprogramming();
                    break;
                case Mode.Exiting :
                    serialPort.close();
                    break;
            }
        }
        if (cmdQueue.length > 0) {
            sendCommandToMicrocontroller(cmdQueue[0]);
        }
    }
}

io.on('connection', function(socket){
    console.log('got a connection.');
    socket.on('serverCommand', function(msg){
        console.log('got server command from client: ' + msg);
        switch (msg) {
            case 'refresh' :
                sendSWVersionToClient(currentSWVer);
                sendSettingsToClient(currentSettings);
                sendTCalOffsetToClient(currentTCalOffset);
                break;
            case 'reprogram' :
                currentMode = Mode.Reprogramming;
                break;
            default :
                console.log('unrecognized server command');
        }
  });

    socket.on('unitCommand', function(msg){
      cmdQueue.push(msg);
      console.log('got unit command from client: ' + msg);
  });
  socket.on('disconnect', function () {
    console.log('lost connection.');
  });
});

serialPort.on("open", function () {
    console.log('serial port open');
    serialPort.on('data', function(data) {
        var dataStr = data.toString();
        for (i = 0; i < dataStr.length; i++) {
            var dataStrChar = dataStr[i];
            switch(dataStrChar) {
                case '\n' :
                    break;
                case '\r' :
                    if (serialPortRxLine.length > 0) {
                        console.log(++serialPortRxLineNum + ': ' + serialPortRxLine);
                        processMicrocontrollerMessage(serialPortRxLine);
                        serialPortRxLine = '';
                    }
                    break;
                default:
                    serialPortRxLine += dataStrChar;
                    break;
            }
        }
        // console.log('data received: ' + data);
    }
);

serialPort.on('close', function() {
    console.log('serial port closed');
  }
);

  requestUnitInfo();
});

var port = 5401;
httpServer.listen(port, function(){
  console.log('listening on *:' + port);
});

// launch child process for chrome UI
var browser = execFile('C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe', [
    '--new-window', 'http://localhost:' + port + '/home'
    ]);
browser.stdin.setEncoding('utf-8');

browser.stdout.on('data', function (data) {
    console.log('browser stdout: "' + data + '"');
});
browser.stderr.on('data', function (data) {
    console.log('browser stderr: "' + data + '"');
});

browser.on('close', function () {
    console.log('browser exited');
    currentMode = Mode.Exiting;
    io.close();
    httpServer.close();
});
