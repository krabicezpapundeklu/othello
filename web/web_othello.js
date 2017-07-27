"use strict";

var WAITING = -1, BLACKS_MOVE = 0, WHITES_MOVE = 1, GAME_OVER = 2;
var state = WAITING;
var cells;
var worker;
var selectedRow = -1;
var selectedCol = -1;

function setStatus(text) {
  document.querySelector("#status").innerHTML = text;
}

function setCellListeners(cell, row, col) {
  cell.addEventListener("click", function() { onCellClick(row, col); });
  cell.addEventListener("mousemove", function() { selectCell(row, col); });
  cell.addEventListener("mouseleave", function() { selectCell(-1, -1); });
}

function init() {
  setStatus("Loading...");

  // Fill in the table.
  var table = document.querySelector("table");
  var tr, th, td, div;
  cells = [];

  table.appendChild(tr = document.createElement("tr"));
  tr.appendChild(document.createElement("th"));
  for (var col = 0; col < 8; col++) {
    tr.appendChild(th = document.createElement("th"));
    th.appendChild(document.createTextNode("ABCDEFGH"[col]));
  }

  for (var row = 0; row < 8; row++) {
    cells.push([]);
    table.appendChild(tr = document.createElement("tr"));
    tr.appendChild(th = document.createElement("th"));
    th.appendChild(document.createTextNode(row));

    for (var col = 0; col < 8; col++) {
      tr.appendChild(td = document.createElement("td"));
      td.appendChild(div = document.createElement("div"));
      cells[row][col] = td;
      setCellListeners(td, row, col);
    }
  }

  // Start the worker.
  if (!window.Worker) {
    setStatus("Error: Web Workers not supported.");
    return;
  }
  worker = new Worker("web_othello_worker.js");

  worker.onmessage = function(e) { onMessageReceived(e.data); };
  addEventListener("keydown", onKeyDown);
  document.querySelector("button").addEventListener("click", newGame);
}

function newGame() {
  worker.postMessage("new game");
  state = WAITING;
}

function onMessageReceived(msg) {
  if (msg === "loaded") {
    newGame();
    return;
  }
  if (msg === "error") {
    setStatus("Error :-(");
    return;
  }

  state = msg.state;
  switch(state) {
  case BLACKS_MOVE:
    setStatus("Human's move.");
    break;
  case WHITES_MOVE:
    setStatus("Computer's move..");
    break;
  case GAME_OVER:
    var black = msg.blackScore;
    var white = msg.whiteScore;
    if (black > white) {
      setStatus("Human wins " + black + "&ndash;" + white + "!");
    } else if (white > black) {
      setStatus("Computer wins " + white + "&ndash;" + black + "!");
    } else {
      setStatus("Draw!");
    }
    break;
  }

  document.querySelector("button").disabled =
    !(state == BLACKS_MOVE || state == GAME_OVER);

  updateDisks(msg.board);
  selectCell(selectedRow, selectedCol);
}

function updateDisks(board) {
  var CELL_BLACK = 0;
  var CELL_WHITE = 1;
  var CELL_EMPTY = 2;

  for (var row = 0; row < 8; row++) {
    for (var col = 0; col < 8; col++) {
      var disk = cells[row][col].firstElementChild;
      switch (board[row][col]) {
      case CELL_BLACK:
        disk.style.background = "black";
        break;
      case CELL_WHITE:
        disk.style.background = "white";
        break;
      case CELL_EMPTY:
        disk.style.background = "transparent";
        break;
      }
    }
  }
}

function selectCell(row, col) {
  if (selectedRow >= 0 && selectedCol >= 0) {
    cells[selectedRow][selectedCol].className = "";
  }

  if (row >= 0 && col >= 0 && state == BLACKS_MOVE) {
    cells[row][col].className = "selected";
  }

  selectedRow = row;
  selectedCol = col;
}

function onCellClick(row, col) {
  if (state != BLACKS_MOVE) {
    return;
  }

  worker.postMessage({row: row, col: col});
  state = WAITING;
}

function onKeyDown(e) {
  if (state != BLACKS_MOVE ||
      document.activeElement == document.querySelector("button")) {
    return;
  }

  var row = selectedRow;
  var col = selectedCol;

  switch (e.key) {
  case " ":
  case "Enter":
    if (row >= 0 && col >= 0) {
      onCellClick(selectedRow, selectedCol);
    }
    return;

  case "ArrowRight": col++; break;
  case "ArrowLeft":  col--; break;
  case "ArrowDown":  row++; break;
  case "ArrowUp":    row--; break;

  case "a": case "b": case "c": case "d":
  case "e": case "f": case "g": case "h":
    col = e.key.charCodeAt(0) - "a".charCodeAt(0);
    break;

  case "0": case "1": case "2": case "3":
  case "4": case "5": case "6": case "7":
    row = e.key.charCodeAt(0) - "0".charCodeAt(0);
    break;

  default:
    return;
  }

  selectCell(Math.max(0, Math.min(row, 7)), Math.max(0, Math.min(col, 7)));
}

init();
