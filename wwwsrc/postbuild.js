const fs = require('fs');
const child_process = require('child_process');

console.log('Hello from the postbuild script :D');

const filesCss = fs.readdirSync('build/static/css');
console.log('Files in CSS: ', filesCss);
filesCss.forEach((name) => {
    if (name.endsWith('.map')) {
        fs.rmSync('build/static/css/' + name);
    }
});
filesCss.forEach((name) => {
    if (name.endsWith('.css')) {
        const ret = child_process.spawnSync('npx', ['postcss', 'build/static/css/' + name]);
        console.log('RETURNED: ', ret.output[1]);
        fs.writeFileSync('build/static/css/' + name, ret.output[1]);
    }
});

const filesJs = fs.readdirSync('build/static/js');
console.log('Files in JS: ', filesJs);
filesJs.forEach((name) => {
    if (name.endsWith('.map')) {
        fs.rmSync('build/static/js/' + name);
    }
});
filesJs.forEach((name) => {
    if (name.endsWith('.LICENSE.txt')) {
        fs.rmSync('build/static/js/' + name);
    }
});
