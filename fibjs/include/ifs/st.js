String.prototype.trim = function() {
	return this.replace(/(^\s*)|(\s*$)/g, "");
}

var fs = require('fs');
var os = require('os');
var process = require('process');
var path = require('path');
var keywords = [
	"require(\"\")"
];

var plugin = {
	"scope": "source.js - variable.other.js",
	"completions": []
};

var cls = {};
var codes = [];
var name = '';
var tips = {};
var ps = [];


var dir = fs.readdir('.');
for (var idx = 0; idx < dir.length; idx++)
	if (path.extname(dir[idx].name) === '.idl')
		preparserIDL(dir[idx].name);

for (var k in cls) {
	name = cls[k][0][1];
	codes = cls[k].slice(2, -1);
	codes = codes.map(function(a) {
		if (a.indexOf("(") !== -1) {
			var b = a.indexOf("(") - 1;
			var c = a.indexOf(")") + 1;

			var s = name + "." + a.slice(b, c).join(" ") + ";";
			if (a.indexOf("new") !== -1) s = "new " + s;
			s = s.replace(/\s\(\s/g, "(");
			s = s.replace(/\s,/g, ",");
			s = s.replace(/\s\)/g, ")").trim();
			return s;
		} else {
			if (a[0] === 'readonly' || a[0] === 'const')
				return name + "." + a[2];
			else if (a.length > 1)
				return name + "." + a[1];

			return undefined;
		}

	});
	codes = codes.filter(function(a) {
		return !!a && (a.length > 0);
	});
	if (codes.length > 0)
		tips[name] = codes;
}

for (var k in tips) {
	add(k);
	tips[k].forEach(add);
}

keywords.forEach(add);

plugin.completions = plugin.completions.concat(ps);
fs.writeFile("../../../bin/fibjs.sublime-completions", JSON.stringify(plugin));

function add(a) {
	var o = {
		"trigger": a
	};
	if (a.indexOf("(") !== -1) {
		var r = a.slice(a.indexOf("(") + 1, a.indexOf(")"));
		if (!!r) {
			r = r.split(",");
			r = r.map(function(as, i) {
				return "${" + ++i + ":" + as + "}";
			}).join(", ");
			a = a.replace(/\(.+\)/g, "(" + r + ")");
			a = a.replace(/:\s+/g, ":");
			o['contents'] = a;
		}
	}
	ps.push(o);
}

function preparserIDL(fname) {
	var f, line = 0,
		st, isRem;

	f = fs.readFile(fname).replace(/\r/g, "").split("\n");
	f.reverse();
	fname = fname.split(".")[0];
	cls[fname] = [];
	while (f.length) {
		st = getStock();
		if (st.length > 0) cls[fname].push(st);
	}

	function getStock() {
		var pos = 0,
			n, n1, st2;
		var bString = false;
		var s = f.pop();
		line++;

		if (isRem) {
			n = s.indexOf("*/", 0);
			if (n >= 0) {
				s = s.substr(n + 2);
				isRem = false;
			}
		}

		var st = s.split("\"");
		var st1 = [];
		while (pos < st.length) {
			s = st[pos++];

			if (isRem) {
				n = s.indexOf("*/", 0);
				if (n >= 0) {
					s = s.substr(n + 2);
					isRem = false;
				}
			}

			if (!isRem) {
				if (!bString) {
					n = s.indexOf("//");
					if (n >= 0)
						s = s.substr(0, n);

					while (1) {
						n = s.indexOf("/*");
						if (n >= 0) {
							n1 = s.indexOf("*/", n + 2);
							if (n1 >= 0)
								s = s.substr(0, n) + s.substr(n1 + 2);
							else {
								isRem = true;
								s = s.substr(0, n);
							}
						} else
							break;
					}

					s = s.replace(/=/g, " = ");
					s = s.replace(/\(/g, " ( ");
					s = s.replace(/\)/g, " ) ");
					s = s.replace(/\[/g, " [ ");
					s = s.replace(/\]/g, " ] ");
					s = s.replace(/,/g, " , ");
					s = s.replace(/:/g, " : ");
					s = s.replace(/;/g, " ; ");
					s = s.replace(/\s+/g, " ").trim();

					if (s != "") {
						st2 = s.split(" ");

						for (var i = 0; i < st2.length; i++) {
							s = st2[i];
							st1.push(s);
						}
					}

					if (!isRem)
						bString = true;
				} else {
					while ((s.charAt(s.length - 1) == "\\") && pos < st.length)
						s = s + "\"" + st[pos++];

					st1.push("\"" + s + "\"")
					bString = false;
				}
			}
		}

		return st1;
	}
}