
// -------------------------------------------------------------------------------------------------

describe("SPFG runtime", function() {

  it("create should return an object", function() {
    var runtime = SPFG.create();
    expect(runtime).toBeDefined();
    runtime.destroy();
  });

  it("init should not fail", function() {
    var runtime = SPFG.create();
    expect(runtime.init.bind(null, 'runtime')).not.toThrow();
    runtime.finish();
  });

  it("finish should not fail", function() {
    var runtime = SPFG.create();
    runtime.init('runtime');
    expect(runtime.finish.bind(null)).not.toThrow();
  });
});

// -------------------------------------------------------------------------------------------------

describe("SPFG DP", function() {
  var runtime;

  beforeEach(function(){
    runtime = SPFG.create();
    runtime.init('runtime');
  });

  afterEach(function(){
    runtime.finish();
    runtime.destroy();
  });

  it("create should not raise", function() {
    expect(runtime.createDP.bind(null, 'bool', 'dp0')).not.toThrow();
  });

  it("create should return valid datapoint id", function() {
    expect(runtime.createDP('bool', 'dp0')).toBeGreaterThan(0);
  });

  it("create with empty name should fail", function() {
    expect(runtime.createDP.bind(null, 'bool', '')).toThrowError('needs a non-empty name');
  });

  it('aaa', function(){})

  it("create with invalid type should fail", function() {
    expect(runtime.createDP.bind(null, 'booleanus', 'dp0')).toThrowError('needs a valid datapoint type');
  });

  it("remove should work with valid id", function() {
    var id = runtime.createDP('bool', 'dp0');
    expect(runtime.removeDP.bind(null, id)).not.toThrow();
  });
});

// -------------------------------------------------------------------------------------------------

describe("SPFG function", function() {
  var runtime, dp0, dp1, dp2;

  beforeEach(function(){
    runtime = SPFG.create();
    runtime.init('runtime');
  });

  afterEach(function(){
    runtime.finish();
    runtime.destroy();
  });

  beforeEach(function(){
    dp0 = runtime.createDP('bool', 'dp0');
    dp1 = runtime.createDP('bool', 'dp1');
    dp2 = runtime.createDP('bool', 'dp2');
  });

  it("create should not raise", function() {
    expect(runtime.createFN.bind(null, 'and(bool,bool)->bool', 0, [dp0, dp1], [dp2], 'fn0')).not.toThrow();
  });

  it("create should return valid function id", function() {
    expect(runtime.createFN('and(bool,bool)->bool', 0, [dp0, dp1], [dp2], 'fn0')).toBeGreaterThan(0);
  });

  it("create with empty name should fail", function() {
    expect(runtime.createFN.bind(null, 'and(bool,bool)->bool', 0, [dp0, dp1], [dp2], '')).toThrow(Error('needs a non-empty name'));
  });

  it("create with invalid type should fail", function() {
    expect(runtime.createFN.bind(null, 'blargh(bool,real)->real', 0, [dp0, dp1], [dp2], 'fn0')).toThrow(Error('needs a valid function type'));
  });

  it("removal should work with valid id", function() {
    var id = runtime.createFN('and(bool,bool)->bool', 0, [dp0, dp1], [dp2], 'fn0');
    expect(runtime.removeFN.bind(null, id)).not.toThrow();
  });
});

// -------------------------------------------------------------------------------------------------

describe("SPFG cycle", function() {
  var runtime, dp0, dp1, dp2, dp3, dp4, fn0, fn1;

  beforeEach(function(){
    runtime = SPFG.create();
    runtime.init('runtime');
  });

  afterEach(function(){
    runtime.finish();
    runtime.destroy();
  });

  beforeEach(function(){
    dp0 = runtime.createDP('bool', 'dp0');
    dp1 = runtime.createDP('bool', 'dp1');
    dp2 = runtime.createDP('bool', 'dp2');
    dp3 = runtime.createDP('bool', 'dp3');
    dp4 = runtime.createDP('bool', 'dp4');
    fn0 = runtime.createFN('and(bool,bool)->bool', 0, [dp0, dp1], [dp2], 'fn0');
    fn1 = runtime.createFN('and(bool,bool)->bool', 1, [dp2, dp3], [dp4], 'fn1');
  });

  it("reset should not raise", function() {
    expect(runtime.reset).not.toThrow();
  });

  it("run should not raise", function() {
    expect(runtime.reset).not.toThrow();
    expect(runtime.run.bind(null, 0, 0)).not.toThrow();
  });

  it("run with a callback should call with arguments", function() {
    var expectedPhase = 0;
    var expectedFnId = fn0;

    function callback(fnId, phase) {
      expect(this.xyz).toEqual(123);
      expect(fnId).toEqual(expectedFnId);
      expect(phase).toEqual(expectedPhase);
      expectedPhase = 1;
      expectedFnId = fn1;
    }

    expect(runtime.reset).not.toThrow();
    expect(runtime.run.bind(null, 0, callback, {xyz: 123})).not.toThrow();
  });

  it("run with a callback returning stop should stop cycle", function() {
    var expectedPhase = 0;

    function callback1(functionId, phase) {
      expect(phase).toEqual(expectedPhase);
      return SPFG.codes.ctl.SPFG_LOOP_CONTROL_STOP;
    }

    function callback2(functionId, phase) {
      expect(phase).toEqual(expectedPhase);
      expectedPhase += 1;
      return SPFG.codes.ctl.SPFG_ERROR_NO;
    }

    expect(runtime.reset).not.toThrow();
    expect(runtime.run.bind(null, 0, callback1)).not.toThrow();
    expect(runtime.run.bind(null, 1, callback2)).not.toThrow();
  });
});

// -------------------------------------------------------------------------------------------------

describe("SPFG evaluation", function() {
  var runtime, dp0, dp1, dp2, dp3, dp4, fn0, fn1;

  beforeEach(function(){
    runtime = SPFG.create();
    runtime.init('runtime');
  });

  afterEach(function(){
    runtime.finish();
    runtime.destroy();
  });

  beforeEach(function(){
    dp0 = runtime.createDP('bool', 'dp0');
    dp1 = runtime.createDP('bool', 'dp1');
    dp2 = runtime.createDP('bool', 'dp2');
    dp3 = runtime.createDP('bool', 'dp3');
    dp4 = runtime.createDP('bool', 'dp4');
    fn0 = runtime.createFN('and(bool,bool)->bool', 0, [dp0, dp1], [dp2], 'fn0');
    fn1 = runtime.createFN('and(bool,bool)->bool', 1, [dp2, dp3], [dp4], 'fn1');
  });

  it("dp0=false dp1=false dp3=false -> dp2=false dp4=false", function() {
    runtime.setb(dp0, false);
    runtime.setb(dp1, false);
    runtime.setb(dp3, false);
    runtime.reset();
    runtime.run(0, 0);
    expect(runtime.export().dps).toEqual([
      {id: 1, name: 'dp0', type: 3, value: false, emitted: false},
      {id: 2, name: 'dp1', type: 3, value: false, emitted: false},
      {id: 3, name: 'dp2', type: 3, value: false, emitted: false},
      {id: 4, name: 'dp3', type: 3, value: false, emitted: false},
      {id: 5, name: 'dp4', type: 3, value: false, emitted: false}]);
  });

  it("dp0=true dp1=true dp3=true -> dp2=true dp4=true", function() {
    runtime.setb(dp0, true);
    runtime.setb(dp1, true);
    runtime.setb(dp3, true);
    runtime.reset();
    runtime.run(0, 0);
    expect(runtime.export().dps).toEqual([
      {id: 1, name: 'dp0', type: 3, value: true, emitted: false},
      {id: 2, name: 'dp1', type: 3, value: true, emitted: false},
      {id: 3, name: 'dp2', type: 3, value: true, emitted: false},
      {id: 4, name: 'dp3', type: 3, value: true, emitted: false},
      {id: 5, name: 'dp4', type: 3, value: true, emitted: true}]);
  });
});

// -------------------------------------------------------------------------------------------------

describe("SPFG grid import/export", function(){
  var runtime;

  beforeEach(function(){
    runtime = SPFG.create();
    runtime.init('runtime');
  });

  afterEach(function(){
    runtime.finish();
    runtime.destroy();
  });

  it("should not fail", function(){
    var original = {id: 1, name: 'runtime', fns: [], dps: [], ctl: {curr_fn_idx: 0}};
    expect(runtime.import.bind(null, original)).not.toThrow;
    var exported = runtime.export();
    expect(exported).toEqual(original);
  });
});

// -------------------------------------------------------------------------------------------------
