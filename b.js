const fs = require('fs');

const isFiber = typeof module === 'undefined';
const num = 1000000;

if (isFiber) {
  const co = require('coroutine');
  console.time('fiber');
  for (let i = 0; i < num; i++) {
    fs.readFile(__filename);
  }
  // co.parallel(() => fs.readFile(__filename), num, 8);
  console.timeEnd('fiber');
} else {
  const promisify = require('util').promisify;
  const readFile = promisify(fs.readFile);

  (async () => {
    console.time('async');
    for (let i = 0; i < num; i++) {
      await readFile(__filename);
    }
    console.timeEnd('async');
  })();
}
