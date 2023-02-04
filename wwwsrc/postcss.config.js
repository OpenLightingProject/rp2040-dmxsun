const purgecss = require('@fullhuman/postcss-purgecss');

module.exports = {
    plugins: [
        require('cssnano')({
            preset: 'advanced',
        }),
        purgecss({
            content: ['build/static/js/*.js']
          })
    ],
};
