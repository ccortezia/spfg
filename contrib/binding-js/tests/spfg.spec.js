
describe("SPFG initialization", function() {

  beforeEach(naiveFinish);

  it("when init is called for the first time should not fail", function() {
    expect(SPFG.init()).toBe(undefined);
  });

  it("when init is called a second time without finishing first should fail", function() {
    expect(SPFG.init()).toBe(undefined);
    expect(SPFG.init).toThrowError(SPFG.Error, 'SPFG_ERROR_ALREADY_INITIALIZED');
  });

  it("when init is called a second time with finishing first should not fail", function() {
    expect(SPFG.init()).toBe(undefined);
    expect(SPFG.finish()).toBe(undefined);
    expect(SPFG.init()).toBe(undefined);
  });
});

describe("SPFG grid", function() {
  beforeEach(naiveFinish);
  beforeEach(SPFG.init);

  it("creation should not fail", function() {
    expect(SPFG.createGrid.bind(SPFG.createGrid, 'gr0')).not.toThrow();
  });

  it("creation should return valid grid id", function() {
    expect(SPFG.createGrid('gr0')).toBeGreaterThan(0);
  });

  it("creation with empty name should fail", function() {
    expect(SPFG.createGrid.bind(SPFG.createGrid, '')).toThrowError('needs a non-empty name');
  });

  it("removal should work with valid id", function() {
    var id = SPFG.createGrid.bind(SPFG.createGrid, 'gr0');
    expect(SPFG.removeGrid.bind(SPFG.removeGrid, id)).not.toThrow();
  });
});

describe("SPFG datapoint", function() {
  var gr0;

  beforeEach(naiveFinish);
  beforeEach(SPFG.init);

  beforeEach(function(){
    gr0 = SPFG.createGrid('gr0');
  });

  it("creation should not raise", function() {
    expect(SPFG.createDatapoint.bind(SPFG.createDatapoint, gr0, 'bool', 'dp0')).not.toThrow();
  });

  it("creation should return valid datapoint id", function() {
    expect(SPFG.createDatapoint(gr0, 'bool', 'dp0')).toBeGreaterThan(0);
  });

  it("creation with empty name should fail", function() {
    expect(SPFG.createDatapoint.bind(SPFG.createDatapoint, gr0, 'bool', '')).toThrowError('needs a non-empty name');
  });

  it("creation with invalid type should fail", function() {
    expect(SPFG.createDatapoint.bind(SPFG.createDatapoint, gr0, 'booleanus', 'dp0')).toThrowError('needs a valid datapoint type');
  });

  it("removal should work with valid id", function() {
    var id = SPFG.createDatapoint.bind(SPFG.createDatapoint, gr0, 'booleanus', 'dp0');
    expect(SPFG.removeDatapoint.bind(SPFG.removeFunction, gr0, id)).not.toThrow();
  });
});

describe("SPFG function", function() {
  var gr0, dp0, dp1, dp2;

  beforeEach(naiveFinish);
  beforeEach(SPFG.init);

  beforeEach(function(){
    gr0 = SPFG.createGrid('gr0');
    dp0 = SPFG.createDatapoint(gr0, 'bool', 'dp0');
    dp1 = SPFG.createDatapoint(gr0, 'bool', 'dp1');
    dp2 = SPFG.createDatapoint(gr0, 'bool', 'dp2');
  });

  it("creation should not raise", function() {
    expect(SPFG.createFunction.bind(SPFG.createFunction,
      gr0, 'and(bool,bool)->bool', 0, [dp0, dp1], [dp2], 'fn0')).not.toThrow();
  });

  it("creation should return valid function id", function() {
    expect(SPFG.createFunction(gr0, 'and(bool,bool)->bool', 0, [dp0, dp1], [dp2], 'fn0')).toBeGreaterThan(0);
  });

  it("creation with empty name should fail", function() {
    expect(SPFG.createFunction.bind(SPFG.createFunction,
      gr0, 'and(bool,bool)->bool', 0, [dp0, dp1], [dp2], '')).toThrow(Error('needs a non-empty name'));
  });

  it("creation with invalid type should fail", function() {
    expect(SPFG.createFunction.bind(SPFG.createFunction,
      gr0, 'blargh(bool,real)->real', 0, [dp0, dp1], [dp2], 'fn0')).toThrow(Error('needs a valid function type'));
  });

  it("removal should work with valid id", function() {
    var id = SPFG.createFunction.bind(SPFG.createFunction, gr0, 'blargh(bool,real)->real', 0, [dp0, dp1], [dp2], 'fn0');
    expect(SPFG.removeFunction.bind(SPFG.removeFunction, gr0, id)).not.toThrow();
  });
});


describe("SPFG cycle", function() {
  var gr0, dp0, dp1, dp2, dp3, dp4, fn0, fn1;

  beforeEach(naiveFinish);
  beforeEach(SPFG.init);

  beforeEach(function(){
    gr0 = SPFG.createGrid('gr0');
    dp0 = SPFG.createDatapoint(gr0, 'bool', 'dp0');
    dp1 = SPFG.createDatapoint(gr0, 'bool', 'dp1');
    dp2 = SPFG.createDatapoint(gr0, 'bool', 'dp2');
    dp3 = SPFG.createDatapoint(gr0, 'bool', 'dp3');
    dp4 = SPFG.createDatapoint(gr0, 'bool', 'dp4');
    fn0 = SPFG.createFunction(gr0, 'and(bool,bool)->bool', 0, [dp0, dp1], [dp2], 'fn0');
    fn1 = SPFG.createFunction(gr0, 'and(bool,bool)->bool', 1, [dp2, dp3], [dp4], 'fn1');
  });

  it("reset should not raise", function() {
    expect(SPFG.resetCycle.bind(SPFG.resetCycle, gr0)).not.toThrow();
  });

  it("run should not raise", function() {
    expect(SPFG.resetCycle.bind(SPFG.resetCycle, gr0)).not.toThrow();
    expect(SPFG.runCycle.bind(SPFG.runCycle, gr0, 0, 0)).not.toThrow();
  });

  it("run with a callback should call with arguments", function() {
    var expectedPhase = 0;
    var expectedFunctionId = fn0;

    function callback(gridId, functionId, phase) {
      expect(this.xyz).toEqual(123);
      expect(gridId).toEqual(gr0);
      expect(phase).toEqual(expectedPhase);
      expect(functionId).toEqual(expectedFunctionId);
      expectedPhase = 1;
      expectedFunctionId = fn1;
    }

    expect(SPFG.resetCycle.bind(SPFG.resetCycle, gr0)).not.toThrow();
    expect(SPFG.runCycle.bind(SPFG.runCycle, gr0, 0, callback, {xyz: 123})).not.toThrow();
  });

  it("run with a callback returning stop should stop cycle", function() {
    var expectedPhase = 0;

    function callback1(gridId, functionId, phase) {
      expect(phase).toEqual(expectedPhase);
      return SPFG.codes.ctl.SPFG_LOOP_CONTROL_STOP;
    }

    function callback2(gridId, functionId, phase) {
      expect(phase).toEqual(expectedPhase);
      expectedPhase += 1;
      return SPFG.codes.ctl.SPFG_ERROR_NO;
    }

    expect(SPFG.resetCycle.bind(SPFG.resetCycle, gr0)).not.toThrow();
    expect(SPFG.runCycle.bind(SPFG.runCycle, gr0, 0, callback1)).not.toThrow();
    expect(SPFG.runCycle.bind(SPFG.runCycle, gr0, 1, callback2)).not.toThrow();
  });
});

describe("SPFG grid import", function(){

  beforeEach(naiveFinish);
  beforeEach(SPFG.init);

  it("should handle the json for a simple grid", function(){
    var snapshot = {id: 1, name: 'gr0', fns: [], dps: [], ctl: {curr_phase: 0, curr_fn_idx: 0}};
    expect(SPFG.importGridSnapshot.bind(SPFG.importGridSnapshot, snapshot)).not.toThrow();
  });

  it("should return the id of the imported grid", function(){
    var snapshot = {id: 144, name: 'gr0', fns: [], dps: [], ctl: {curr_phase: 0, curr_fn_idx: 0}};
    var gridId = SPFG.importGridSnapshot(snapshot);
    expect(gridId).toEqual(144);
  });
});

describe("SPFG grid export", function(){

  beforeEach(naiveFinish);
  beforeEach(SPFG.init);

  it("should return the expected grid snapshot data", function(){
    var original = {id: 144, name: 'gr144', fns: [], dps: [], ctl: {curr_phase: 0, curr_fn_idx: 0}};
    var gridId = SPFG.importGridSnapshot(original);
    var exported = SPFG.exportGridSnapshot(gridId);
    expect(exported).toEqual(original);
  });
});

// -------------------------------------
// Test Helper functions
// -------------------------------------

function naiveFinish() {
  try {
    SPFG.finish();
  } catch (e) {
    // ignore
  }
}
