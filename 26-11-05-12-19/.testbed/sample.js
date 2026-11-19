let X = IN();
let X2 = IN();
let Y = OUT();
let Y2 = OUT();

let time = 1000;
let level = 50;
let feedback = 0;

let tape = new Array(96000).fill(0);
let write_cursor = 0;

function fx() {
  // ==== Move the reading cursor
  let delaySamples = Math.trunc((SPS() * time) / 1000);

  let read_cursor = write_cursor - delaySamples;
  if (read_cursor < 0) {
    read_cursor += tape.length;
  }

  // ==== Write Y
  Y = X + tape[read_cursor] * (level / 100);

  // ==== Write the tape
  tape[write_cursor] = X + tape[read_cursor] * (feedback / 100);

  // ==== Roll the tape
  write_cursor = (write_cursor + 1) % tape.length;
}