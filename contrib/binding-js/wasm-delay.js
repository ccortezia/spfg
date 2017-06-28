/**
 * Workaround to perform wasm tests
 *
 * The environment may not be ready to use wasm function as soon as the browser
 * finishes loading the declared files, specially because the .wasm binary is
 * fetched asynchronously. For this reason a hardcoded delay is added below.
 */
window.__karma__.loaded = function() {};
setTimeout(function(){
    window.__karma__.start();
}, 1000);
