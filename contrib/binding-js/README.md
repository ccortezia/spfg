# SPFG Javascript Bindings

A thin wrapper around the core spfg public wasm ABI.

## Running the tests

To run the tests for the core spfg library using the bindings provided by this javascript package, make sure you have the wasm binary available, then run the following command:

```
npm install
npm test
```

## Getting Started

The following code snippet is roughly equivalent to the C snippet in the core library's [README](../../README.md#getting-started). To have the symbol SPFG available in the global scope, simply include the [wrapper](src/spfg.js) javascript file and the emcc loader file (available as `lib/spfg/libspfg.js` when compiled) into the browser window.

```javascript
// Prepares a runtime instance.
runtime = SPFG.create();
runtime.init('rt0');

// Prepares an evaluation scheme.
dp1 = runtime.createDP('bool', 'dp1');
dp2 = runtime.createDP('bool', 'dp2');
dp3 = runtime.createDP('bool', 'dp3');
dp4 = runtime.createDP('bool', 'dp4');
dp5 = runtime.createDP('bool', 'dp5');
fn1 = runtime.createFN('and(bool,bool)->bool', 0, [dp1, dp2], [dp4], 'fn1');
fn2 = runtime.createFN('and(bool,bool)->bool', 1, [dp4, dp3], [dp5], 'fn2');

// Runs an evaluation.
runtime.setb(dp1, true);
runtime.setb(dp2, true);
runtime.setb(dp3, true);
runtime.reset();
runtime.run(0);
data = runtime.getDP(dp3);
data.value == true;
data.emitted == true;

// Cleanup allocated memory.
runtime.finish()
runtime.destroy()
```

## Roadmap

- [ ] Support for node.js.
- [ ] Complete API (some methods are still missing).
