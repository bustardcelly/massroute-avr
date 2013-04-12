/*global require:false module:false*/
var ShiftLed = function(options) {
      this.board = options.board;
      this.redPin = this.board.normalizePin(options.redPin);
      this.greenPin = this.board.normalizePin(options.greenPin);
      this.highValue = this.board.normalizeVal(this.board.HIGH);
      this.lowValue = this.board.normalizeVal(this.board.LOW);
    };

ShiftLed.prototype.red = function() {
  this.board.write('96' + this.greenPin + this.lowValue);
  this.board.write('96' + this.redPin + this.highValue);
};

ShiftLed.prototype.green = function() {
  this.board.write('96' + this.redPin + this.lowValue);
  this.board.write('96' + this.greenPin + this.highValue);
};

ShiftLed.prototype.off = function() {
  this.board.write('96' + this.greenPin + this.lowValue);
  this.board.write('96' + this.redPin + this.lowValue);
};

module.exports = ShiftLed;