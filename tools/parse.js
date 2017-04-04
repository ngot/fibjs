const ejs = require('ejs');
const fs = require('fs');
const path = require('path');

ejs.fileLoader = fs.readFile;
ejs.renderFile = sync(ejs.renderFile);

const res1 = ejs.renderFile(path.join(__dirname, './tpl/home.ejs'),
  {
    user: {
      name: "aaaaaaaaaaaa"
    }
  }, 
  {});

console.log(res1);

