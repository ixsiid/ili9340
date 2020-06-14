const fs = require('fs');
const path = require('path');

// node generator.js ./shnm8x16r.bdf


const type = 'uint8_t';
const data_element_length = ({
    uint8_t: 2,
    uint16_t: 4,
    uint32_t: 8,
    uint64_t: 16,
})[type];

process.argv.filter((_, i) => i >= 2).forEach(arg => {
    const char_data = fs.readFileSync(arg).toString().split('STARTCHAR').filter(x => {
        const i = x.indexOf('ENCODING');
        const code = parseInt(x.substring(i + 8, x.indexOf('\n', i + 8)));
        return code >= 32 && code < (32 + 128);
    }).map(x => {
        const i = x.indexOf('ENCODING');
        const code = parseInt(x.substring(i + 8, x.indexOf('\n', i + 8))) - 32;

        const data = x.substring(x.indexOf('BITMAP') + 7, x.indexOf('ENDCHAR') - 1)
            .split('\n')
            .map(line => line.match(new RegExp(`.{${data_element_length}}`, 'g')).map(n => parseInt(n, 16)));

        return { code, data };
    });

    const font = Array(128).fill(0).map((_, i) => {
        return char_data.find(({ code }) => code == i) || ({ code: i, data: false });
    });

    const size = { width: 8, height: 16 };

    const name = path.basename(arg, '.bdf');
    const fd = fs.openSync(path.join(__dirname, '..', 'src', name + '.hpp'), 'w');
    fs.writeSync(fd, `#pragma once
    
#include "font.hpp"

namespace Font {
class ${name} : public FontBase {
    public:
    ${name}();

    private:
    static ${type} empty[]; 
${char_data.map(x => `\tstatic ${type} ${name}_${x.code}[];`).join('\n')}
};

${type} ${name}::empty[] = {${Array(size.width * size.height / 8).fill(0).join(',')}};

${char_data.filter(x => x.data).map(x => `${type} ${name}::${name}_${x.code}[] = {${x.data.map(line => line.join(', ')).join(',')}};`).join('\n')}

${name}::${name}() : FontBase() {
    width = ${size.width};
    height = ${size.height};

${font.map(x => `\t${name}::font[${x.code}] = ${x.data ? `${name}::${name}_${x.code}` : 'empty'};`).join('\n')}
}

}
`);
    console.log(`Success generate ${name}.hpp`);
});
