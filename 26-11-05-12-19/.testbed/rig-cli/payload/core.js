




let time = 1000;
let level = 50;
let feedback = 0;

let tape = new Array(96000).fill(0);
let write_cursor = 0;

globalThis.__linda_process = function(spb, __linda_sps, __linda_in, __linda_out) {
  for (let __linda_i = 0; __linda_i < spb; __linda_i++) {
    let X = __linda_in[0 * spb + __linda_i];
    let X2 = __linda_in[1 * spb + __linda_i];
    let Y = 0;
    let Y2 = 0;
    {

  // ==== Move the reading cursor
  let delaySamples = Math.trunc((__linda_sps * time) / 1000);

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
    __linda_out[0 * spb + __linda_i] = Y;
    __linda_out[1 * spb + __linda_i] = Y2;
  }
};