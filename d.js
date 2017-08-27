const fs = require('fs');
const sync = require('util').sync;

const num = 100000;

let readFile = filename => {
  return new Promise((resolve, reject) => {
    fs.readFile(filename, (err, data) => {
      if (err) reject(err);
      else resolve(data);
    });
  });
};

readFile = sync(readFile, true);

console.time('async');
for (let i = 0; i < num; i++) {
  readFile(__filename);
}
console.timeEnd('async');
