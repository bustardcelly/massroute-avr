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
    url = 'http://68.169.43.76:3001/routes/39/destinations/39_1_var1/stops/1129',
    CronJob = require('cron').CronJob,
    cron;

// process arguments.
if(args) {
  if(args.hasOwnProperty('port')) {
    port = args.port;
  }
}

function getPredictions() {
  board.log('requesting...');
  request({
    uri: url,
    json: true
  }, function(err, response, body) {  
    var predictions,
        i, length,
        delimiter = " ",
        predictionStr = "";

    if(err) {
      // TODO: Send error message.
      board.log('Error: ' + err);
    }
    else {
      predictions = body.predictions;
      length = (!predictions || predictions.length > 2) ? 2 : predictions.length;
      for(i = 0; i < length; i++) {
        if(i > 0) {
          predictionStr = predictionStr + delimiter;
        }
        predictionStr = predictionStr + predictions[i].seconds.toString();
      }
      lcd.write(predictionStr);
    }
  });
}

app.use(express.static(__dirname));

cron = new CronJob('1 * * * * *', function() {
  getPredictions();
}, null, true, 'America/New_York');

getPredictions();

app.listen(port);
board.log("serial textserver running on port " + port + " in " + app.settings.env + " mode");