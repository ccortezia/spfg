
SPFG = (function SPFG() {

    // Maps native error return codes.
    var errors = {
        SPFG_ERROR_NO:                       0,
        SPFG_ERROR_FAIL:                    -1,
        SPFG_ERROR_BAD_PARAM_NULL_POINTER:  -2,
        SPFG_ERROR_BAD_PARAM_INVALID_VALUE: -3,
        SPFG_ERROR_ALREADY_INITIALIZED:     -4,
        SPFG_ERROR_NOT_INITIALIZED:         -5,
        SPFG_ERROR_OUT_OF_SLOTS:            -6,
        SPFG_ERROR_BAD_BLOCK_NAME:          -7,
        SPFG_ERROR_NOT_FOUND:               -8,
        SPFG_ERROR_INVALID_GR_ID:           -9,
        SPFG_ERROR_INVALID_DP_ID:           -10,
        SPFG_ERROR_INVALID_FN_ID:           -11,
        SPFG_ERROR_EVAL_FN_FAILURE:         -12,
        SPFG_ERROR_EVAL_CB_FAILURE:         -13,
        SPFG_ERROR_UNIMPLEMENTED:           -14,
        SPFG_ERROR_REINDEX:                 -15,
        SPFG_ERROR_VALIDATE_FN:             -16,
        SPFG_ERROR_BUFFER_OVERFLOW:         -17,
        SPFG_ERROR_FN_INTEGRITY:            -18,
        SPFG_ERROR_ALREADY_EXISTS:          -19
    };

    // Maps native control return codes.
    var control = {
        SPFG_LOOP_CONTROL_STOP: 1,
    };

    // Keys from this map are accepted as createDatapoint() parameter.
    var datapointTypes = {
        'bool': 3,
    };

    // Keys from this map are accepted as .createFunction() parameter.
    var functionTypes = {
        'and(bool,bool)->bool': 2,
    };

    // Prepare custom exception function.
    CustomError = createCustomError();

    // Private map to manage memory.
    var allocations = {};

    return {
        create: create,
        codes: {
            dp: datapointTypes,
            fn: functionTypes,
            err: errors,
            ctl: control,
        },
        Error: CustomError
    };

    /**
     * Allocates memory for a runtime and returns a reference.
     *
     * @return memory address for the created runtime object
     */
    function spfgCreateRuntime() {

        var module = getModule();
        var sizeOutPtr = module._malloc(4);

        var err = module._spfg_runtime_size(sizeOutPtr);

        cleanErr(err, function(){
            module._free(sizeOutPtr);
        });

        var size = module.getValue(sizeOutPtr, 'i32');
        var runtimePtr = module._malloc(size);

        module._free(sizeOutPtr);
        return runtimePtr;
    }

    /**
     * Deallocates memory for the given runtime reference.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @return undefined
     */
    function spfgDestroyRuntime(runtimePtr) {
        var module = getModule();
        module._free(runtimePtr);
    }

    /**
     * Creates a grid.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @param string name: symbolic name for the grid
     * @return undefined
     */
    function spfgInitRuntime(runtimePtr, name) {

        if (!name) {
            throw Error('needs a non-empty name');
        }

        var module = getModule();
        var charsLength = name.length * 4 + 1;
        var nameInPtr = module._malloc(charsLength);
        module.stringToUTF8(name, nameInPtr, charsLength);

        var err = module._spfg_rt_init(runtimePtr, nameInPtr);

        cleanErr(err, function(){
            module._free(nameInPtr);
        });

        markAllocation(runtimePtr, null, null, nameInPtr);
        return;
    }

    /**
     * Finishes the given runtime.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @return undefined
     */
    function spfgFinishRuntime(runtimePtr) {
        var module = getModule();
        cleanErr(module._spfg_rt_finish(runtimePtr));
        releaseAllocations(runtimePtr, null, null);
    }

    /**
     * Creates a grid datapoint.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @param string type: refer to datapointTypes for the available types
     * @param string name: symbolic name for the datapoint
     * @return number, the created datapoint id
     */
    function spfgCreateDP(runtimePtr, type, name) {

        if (!name) {
            throw Error('needs a non-empty name');
        }

        if (!datapointTypes[type]) {
            throw Error('needs a valid datapoint type');
        }

        var module = getModule();
        var idOutPtr = module._malloc(4);
        var charsLength = name.length * 4 + 1;
        var nameInPtr = module._malloc(charsLength);
        module.stringToUTF8(name, nameInPtr, charsLength);
        var err = module._spfg_rt_dp_create(runtimePtr, datapointTypes[type], nameInPtr, idOutPtr);

        cleanErr(err, function(){
            module._free(idOutPtr);
            module._free(nameInPtr);
        });

        var id = module.getValue(idOutPtr, 'i32');
        markAllocation(runtimePtr, null, id, idOutPtr);
        markAllocation(runtimePtr, null, id, nameInPtr);
        return id;
    }

    /**
     * Removes a grid datapoint.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @param number datapointId
     * @return undefined
     */
    function spfgRemoveDP(runtimePtr, datapointId) {
        var module = getModule();
        cleanErr(module._spfg_rt_dp_remove(runtimePtr, datapointId));
        releaseAllocations(runtimePtr, null, datapointId);
    }

    /**
     * Creates a grid function.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @param string type: refer to functionType for the available types
     * @param number phase: evaluation phase to attach the function to
     * @param array[number] inDpIds: list of datapoint ids to use as input for the function
     * @param array[number] outDpIds: list of datapoint ids to use as output for the function
     * @param string name: symbolic name for the function
     * @return number, the created function id
     */
    function spfgCreateFN(runtimePtr, type, phase, inDpIds, outDpIds, name) {

        if (!name) {
            throw Error('needs a non-empty name');
        }

        if (!functionTypes[type]) {
            throw Error('needs a valid function type');
        }

        inDpIds = inDpIds || [];
        outDpIds = outDpIds || [];

        var module = getModule();
        var idOutPtr = module._malloc(4);

        var charsLength = name.length * 4 + 1;
        var nameInPtr = module._malloc(charsLength);
        module.stringToUTF8(name, nameInPtr, charsLength);

        var inDpIdsArray = new Uint32Array(inDpIds);
        var inDpIdsInPtr = module._malloc(inDpIds.length * 4);
        module.HEAP32.set(inDpIdsArray, inDpIdsInPtr / 4);

        var outDpIdsArray = new Uint32Array(outDpIds);
        var outDpIdsInPtr = module._malloc(outDpIds.length * 4);
        module.HEAPU32.set(outDpIdsArray, outDpIdsInPtr / 4);

        var err = module._spfg_rt_fn_create(
            runtimePtr, functionTypes[type], phase,
            inDpIdsInPtr, inDpIds.length,
            outDpIdsInPtr, outDpIds.length,
            nameInPtr, idOutPtr);

        cleanErr(err, function(){
            module._free(idOutPtr);
            module._free(nameInPtr);
            module._free(inDpIdsInPtr);
            module._free(outDpIdsInPtr);
        });

        var id = module.getValue(idOutPtr, 'i32');
        markAllocation(runtimePtr, id, null, idOutPtr);
        markAllocation(runtimePtr, id, null, nameInPtr);
        markAllocation(runtimePtr, id, null, inDpIdsInPtr);
        markAllocation(runtimePtr, id, null, outDpIdsInPtr);
        return id;
    }

    /**
     * Removes a grid function.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @param number functionId
     * @return undefined
     */
    function spfgRemoveFN(runtimePtr, functionId) {
        var module = getModule();
        cleanErr(module._spfg_rt_fn_remove(runtimePtr, functionId));
        releaseAllocations(runtimePtr, functionId, null);
    }

    /**
     * Restart the evaluation cycle for the given grid.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @return undefined
     */
    function spfgResetCycle(runtimePtr) {
        cleanErr(getModule()._spfg_rt_reset_cycle(runtimePtr));
    }

    /**
     * Resumes the evaluation cycle for the given grid.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @param number timestamp: number representing the passage of time
     * @param function callback [optional]: called upon each cycle iteration
     * @param object context [optional]: context to bind the callback upon each call
     * @return undefined
     */
    function spfgRunCycle(runtimePtr, timestamp, callback, thisCtx) {
        var module = getModule();
        callback = (callback && thisCtx) ? callback.bind(thisCtx) : callback;
        var cbInPtr = (callback || 0) && module.addFunction(callback);
        cleanErr(module._spfg_rt_run_cycle(runtimePtr, timestamp, cbInPtr, 0));
    }

    /**
     * Imports a grid snapshot and either creates a new grid or updates an existing one.
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @param Object gridSnapshot: object containing a deserialized snapshot of a grid
     * @return undefined
     */
    function spfgImportGridSnapshot(runtimePtr, gridSnapshot) {

        if (!gridSnapshot) {
            throw Error('needs a non-empty object');
        }

        var module = getModule();
        var jsonData = JSON.stringify(gridSnapshot);
        var charsLength = jsonData.length * 4 + 1;
        var jsonInPtr = module._malloc(charsLength);
        module.stringToUTF8(jsonData, jsonInPtr, charsLength);

        var err = module._spfg_rt_import_json(runtimePtr, jsonInPtr, charsLength);

        cleanErr(err, function(){
            module._free(jsonInPtr);
        });

        module._free(jsonInPtr);
    }

    /**
     *  Exports an existing grid into a deserialized JSON object
     *
     * @param number runtimePtr: memory address for the target runtime object
     * @return Object, the deserialized JSON snapshot
     */
    function spfgExportGridSnapshot(runtimePtr) {
        var module = getModule();
        var maxlen = 1024 * 24;
        var jsonOutPtr = module._malloc(maxlen);
        var outlenOutPtr = module._malloc(4);
        module.setValue(outlenOutPtr, 0, 'i32');

        var err = module._spfg_rt_export_json(runtimePtr, jsonOutPtr, maxlen, outlenOutPtr);

        cleanErr(err, function(){
            module._free(jsonOutPtr);
            module._free(outlenOutPtr);
        });

        var jsonData = module.UTF8ToString(jsonOutPtr);
        var gridSnapshot = JSON.parse(jsonData);

        module._free(jsonOutPtr);
        module._free(outlenOutPtr);
        return gridSnapshot;
    }

    /**
     * Sets the value of a datapoint of type boolean
     *
     * @param {number} runtimePtr: memory address for the target runtime object
     * @param {number} datapointId: the target datapoint id
     * @param {undefined}
     */
    function spfgSetBoolean(runtimePtr, datapointId, value) {
        var module = getModule();
        cleanErr(module._spfg_rt_dp_set_bool(runtimePtr, datapointId, value));
    }

    /**
     * Retrieves the loaded wasm module
     */
    function getModule() {
        return window.ModuleSPFG;
    }

    /**
     * Process an error code returned from the underlying API, and potentially throws an error
     *
     * @param number err: return value from wrapped library
     * @param function callback: called before throwing an error
     * @throws SPFGError, in case of error
     */
    function cleanErr(err, callback) {
        if (err != errors.SPFG_ERROR_NO) {
            callback && callback();
            throw new CustomError(err);
        }
    }

    /**
     * Creates the custom library error.
     *
     * @return function
     */
    function createCustomError() {

        // Map for reverse lookup of errors
        var errorLabels = {};
        for (label in errors) {
            errorLabels[errors[label]] = label;
        }

        function CustomError(error_code) {
            this.name = 'SPFGError';
            this.message = errorLabels[error_code];
            this.stack = (new Error(this.message)).stack;
        }
        CustomError.prototype = Object.create(Error.prototype);
        CustomError.prototype.constructor = CustomError;
        return CustomError;
    }

    /**
     * Creates a key for the allocation map.
     *
     * @param number runtimePtr
     * @param number fnId
     * @param number dpId
     * @return string, the lookup key
     */
    function allocationKey(runtimePtr, fnId, dpId) {
        return runtimePtr + '/' + fnId + '/' + dpId;
    }

    /**
     * Takes note of the pointer for a dynamically allocated memory area, and add it to a
     * pointer bucket for later deallocation.
     *
     * @param number runtimePtr
     * @param number fnId
     * @param number dpId
     * @param number pointer
     * @return undefined
     */
    function markAllocation(runtimePtr, fnId, dpId, pointer) {
        var key = allocationKey(runtimePtr, fnId, dpId);
        allocations[key] = allocations[key] || [];
        allocations[key].push(pointer);
    }

    /**
     * Deallocates memory from the pointers in a bucket.
     *
     * @param number runtimePtr
     * @param number fnId
     * @param number dpId
     * @return undefined
     */
    function releaseAllocations(runtimePtr, fnId, dpId) {
        var module = getModule();
        var key = allocationKey(runtimePtr, fnId, dpId);
        var ptrs = (allocations[key] || []);
        while (ptrs.length) {
            module._free(ptrs.pop());
        }
    }

    function create(name) {

        var ptr = spfgCreateRuntime(name);

        return {
            init: spfgInitRuntime.bind(null, ptr),
            finish: spfgFinishRuntime.bind(null, ptr),
            destroy: spfgDestroyRuntime.bind(null, ptr),
            createDP: spfgCreateDP.bind(null, ptr),
            removeDP: spfgRemoveDP.bind(null, ptr),
            createFN: spfgCreateFN.bind(null, ptr),
            removeFN: spfgRemoveFN.bind(null, ptr),
            setb: spfgSetBoolean.bind(null, ptr),
            reset: spfgResetCycle.bind(null, ptr),
            run: spfgRunCycle.bind(null, ptr),
            import: spfgImportGridSnapshot.bind(null, ptr),
            export: spfgExportGridSnapshot.bind(null, ptr),
        };
    }

})();

