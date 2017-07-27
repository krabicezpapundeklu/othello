"use strict";

var Module;

var SIZEOF_BOARD_T = 16;
var SIZEOF_INT = 4;
var PLAYER_BLACK = 0;
var PLAYER_WHITE = 1;

function Board() {
  this.ptr = Module._malloc(SIZEOF_BOARD_T);
  this.outRow = Module._malloc(SIZEOF_INT);
  this.outCol = Module._malloc(SIZEOF_INT);
  this.init();
}
Board.prototype.init = function() {
  Module.ccall("othello_init", null,
               ["number"],
               [this.ptr]);
};
Board.prototype.cellState = function(row, col) {
  return Module.ccall("othello_cell_state", "number",
                      ["number", "number", "number"],
                      [this.ptr, row, col]);
};
Board.prototype.score = function(player) {
  return Module.ccall("othello_score", "number",
                      ["number", "number"],
                      [this.ptr, player]);
};
Board.prototype.hasValidMove = function(player) {
  return Module.ccall("othello_has_valid_move", "number",
                      ["number", "number"],
                      [this.ptr, player]);
};
Board.prototype.isValidMove = function(player, row, col) {
  return Module.ccall("othello_is_valid_move", "number",
                      ["number", "number", "number", "number"],
                      [this.ptr, player, row, col]);
};
Board.prototype.makeMove = function(player, row, col) {
  Module.ccall("othello_make_move", null,
               ["number", "number", "number", "number"],
               [this.ptr, player, row, col]);
};
Board.prototype.computeMove = function(player) {
  Module.ccall("othello_compute_move", null,
               ["number", "number", "number", "number"],
               [this.ptr, player, this.outRow, this.outCol]);
  return [Module.getValue(this.outRow, "i32"),
          Module.getValue(this.outCol, "i32")];
};

var BLACKS_MOVE = 0, WHITES_MOVE = 1, GAME_OVER = 2;
var state;
var board;

function postState() {
  var arr = [];
  for (var row = 0; row < 8; row++) {
    arr.push([]);
    for (var col = 0; col < 8; col++) {
      arr[row][col] = board.cellState(row, col);
    }
  }

  postMessage({state: state,
               board: arr,
               blackScore: board.score(PLAYER_BLACK),
               whiteScore: board.score(PLAYER_WHITE)});
}

onmessage = function(e) {
  if (e.data === "new game") {
    board.init();
    state = BLACKS_MOVE;
    postState();
    return;
  }

  if (state != BLACKS_MOVE ||
      !board.isValidMove(PLAYER_BLACK, e.data.row, e.data.col)) {
    postState();
    return;
  }

  board.makeMove(PLAYER_BLACK, e.data.row, e.data.col);

  if (board.hasValidMove(PLAYER_WHITE)) {
    state = WHITES_MOVE;
    do {
      postState();
      var move = board.computeMove(PLAYER_WHITE);
      board.makeMove(PLAYER_WHITE, move[0], move[1]);
    } while (!board.hasValidMove(PLAYER_BLACK) &&
             board.hasValidMove(PLAYER_WHITE));
  }

  if (board.hasValidMove(PLAYER_BLACK)) {
    state = BLACKS_MOVE;
  } else {
    state = GAME_OVER;
  }

  postState();
}

function emscriptenLoaded() {
  board = new Board();
  postMessage("loaded");
}

function start() {
  if (self.WebAssembly) {
    try {
      Module = { postRun: [function() { emscriptenLoaded(); }] };
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "wasm_othello.wasm", false);
      xhr.responseType = "arraybuffer";
      xhr.send(null);
      Module.wasmBinary = xhr.response;
      importScripts("wasm_othello.js");
      return;
    } catch (e) {
      console.error(e);
    }
  }

  try {
    Module = { postRun: [function() { emscriptenLoaded(); }] };
    importScripts("othello.asm.js");
  } catch(e) {
    console.error(e);
    postMessage("error");
  }
}

start();
