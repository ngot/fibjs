const fs = require('fs');

const num = 100000;

const readFile = filename => {
  return new Promise((resolve, reject) => {
    fs.readFile(filename, (err, data) => {
      if (err) reject(err);
      else resolve(data);
    });
  });
};

(async () => {
  console.time('async');
  for (let i = 0; i < num; i++) {
    await readFile(__filename);
  }
  console.timeEnd('async');
})();
