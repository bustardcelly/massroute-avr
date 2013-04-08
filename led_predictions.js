/*global require:false module:false __dirname:false process:false*/
var args = require('optimist').argv,
    path = require('path'),
    request = require('request'),
    arduino = require('duino'),
    board = new arduino.Board( {
      debug: true
    }),
    LCD = require(path.resolve(process.cwd(), '../script/lcd')),
    express = require('express'),
    app = express(),
    port = 3003,
    lcd = new LCD({
      board: board,
      pin: 4
    }),
    pinIndex = 13,
    url = 'http://68.169.43.76:3001/routes/39/destinations/39_1_var1/stops/{0}',
    pingCount = 0,
    // stopIds = ['1162', '1164', '11164', '1128', '1129', '1130'],
    stopIds = ['1938', '1128', '1129'],
    pinMap = {},
    minApproach = 5 * 60, // 3 minutes
    minArrival = 2 * 60, // 2 minutes
    createLed = function() {
      return new arduino.Led({
              board: board,
              pin: pinIndex--
            });
    };

// outbound
// 13, 12
// <stop tag="1162" title="Centre St @ Lakeville Rd" lat="42.3158699" lon="-71.1141299" stopId="01162"/>
// 11, 10
// <stop tag="1164" title="Centre St @ Myrtle St" lat="42.31331" lon="-71.1141599" stopId="01164"/>
// 9, 8
// <stop tag="11164" title="Centre St @ Burroughs St" lat="42.3114699" lon="-71.1144999" stopId="11164"/>
// 
// inbound
// 13, 12
// <stop tag="1938" title="South St @ Carolina Ave" lat="42.3078" lon="-71.11546" stopId="01938"/>
// 11, 10
// <stop tag="1128" title="South St @ Sedgwick St" lat="42.3085899" lon="-71.11549" stopId="01128"/>
// 9, 8
// <stop tag="1129" title="Centre St @ Seaverns Ave" lat="42.3121999" lon="-71.11414" stopId="01129"/>
// 
// prediction
// <prediction epochTime="1365039103705" seconds="996" minutes="16" isDeparture="false" dirTag="39_0_var0" vehicle="2088" block="B31_72" tripTag="19805474"/>

// process arguments.
if(args) {
  if(args.hasOwnProperty('port')) {
    port = args.port;
  }
}

// Bi-color 3-lead leds.
var i = 0, length = stopIds.length;
for(i; i < length; i++) {
  pinMap[stopIds[i]] = {
    red: createLed(),
    green: createLed()
  };
}

function getPredictions() {
  var stopIndex = (pingCount++ % stopIds.length),
      stopId = stopIds[stopIndex];
  console.log('request: ' + url.replace('{0}', stopId));
  request({
    uri: url.replace('{0}', stopId),
    json: true
  }, function(err, response, body) {  
    var predictions,
        prediction,
        seconds,
        redPin, greenPin;

    if(err) {
      // TODO: Send error message.
      board.log('Error: ' + err);
    }
    else {
      predictions = body.predictions;
      if(predictions && predictions.length > 0) {
        prediction = predictions[0];
        seconds = prediction.seconds ? parseInt(prediction.seconds, 10) : -1;
        if(seconds > -1) {
          board.log('stop ' + stopId + ': ' + seconds);
          redPin = pinMap[stopId].red;
          greenPin = pinMap[stopId].green;
          if(seconds <= minArrival) {
            // set pin HIGH on red lead
            redPin.on();
            greenPin.off();
            // set pin LOW on green lead
          }
          else if(seconds <= minApproach) {
            // set pin LOW on red lead
            redPin.off();
            // set pin HIGH on green lead
            greenPin.on();
          }
          else {
            // set pin LOW on red lead
            redPin.off();
            // set pin LOW on green lead
            greenPin.off();
          }
        }
      }
    }
  });
}

app.use(express.static(__dirname));

// every 10-seconds per MassDOT restirctions.
setInterval(getPredictions, 10000);

app.listen(port);
board.log("serial textserver running on port " + port + " in " + app.settings.env + " mode");