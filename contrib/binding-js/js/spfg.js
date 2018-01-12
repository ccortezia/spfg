
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
        // Lifecycle functions
        init: spfgInit,
        finish: spfgFinish,

        // Composition functions
        createGrid: spfgCreateGrid,
        createDatapoint: spfgCreateDatapoint,
        createFunction: spfgCreateFunction,
        removeGrid: spfgRemoveGrid,
        removeDatapoint: spfgRemoveDatapoint,
        removeFunction: spfgRemoveFunction,

        // Evaluation functions
        setBoolean: spfgSetBoolean,
        resetCycle: spfgResetCycle,
        runCycle: spfgRunCycle,

        // Import / Export functions
        importGridSnapshot: spfgImportGridSnapshot,
        exportGridSnapshot: spfgExportGridSnapshot,

        // Exported symbols
        codes: {
            dp: datapointTypes,
            fn: functionTypes,
            err: errors,
            ctl: control,
        },
        Error: CustomError
    };

    // --------------------------------------------------------------
    // Public functions
    // --------------------------------------------------------------

    /**
     * Initializes the underlying library.
     *
     * @return undefined
     */
    function spfgInit() {
        var module = getModule();
        cleanErr(module._spfg_init());
    }

    /**
     * Finalizes the underlying library.
     *
     * @return undefined
     */
    function spfgFinish() {
        var module = getModule();
        cleanErr(module._spfg_finish());
    }

    /**
     * Creates a grid.
     *
     * @param string name: symbolic name for the grid
     * @return number, the created grid id
     */
    function spfgCreateGrid(name) {

        if (!name) {
            throw Error('needs a non-empty name');
        }

        var module = getModule();
        var idOutPtr = module._malloc(4);
        var charsLength = name.length * 4 + 1;
        var nameInPtr = module._malloc(charsLength);
        module.stringToUTF8(name, nameInPtr, charsLength);
        var err = module._spfg_gr_create(nameInPtr, idOutPtr);

        cleanErr(err, function(){
            module._free(idOutPtr);
            module._free(nameInPtr);
        });

        var id = module.getValue(idOutPtr, 'i32');
        markAllocation(id, null, null, idOutPtr);
        markAllocation(id, null, null, nameInPtr);
        return id;
    }

    /**
     * Removes a grid.
     *
     * @param number gridId
     * @return undefined
     */
    function spfgRemoveGrid(gridId) {
        var module = getModule();
        cleanErr(module._spfg_gr_remove(gridId));
        releaseAllocations(gridId, null, null);
    }

    /**
     * Creates a grid datapoint.
     *
     * @param number gridId: id of the target grid
     * @param string type: refer to datapointTypes for the available types
     * @param string name: symbolic name for the datapoint
     * @return number, the created datapoint id
     */
    function spfgCreateDatapoint(gridId, type, name) {

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
        var err = module._spfg_dp_create(gridId, datapointTypes[type], nameInPtr, idOutPtr);

        cleanErr(err, function(){
            module._free(idOutPtr);
            module._free(nameInPtr);
        });

        var id = module.getValue(idOutPtr, 'i32');
        markAllocation(gridId, null, id, idOutPtr);
        markAllocation(gridId, null, id, nameInPtr);
        return id;
    }

    /**
     * Removes a grid datapoint.
     *
     * @param number gridId
     * @param number datapointId
     * @return undefined
     */
    function spfgRemoveDatapoint(gridId, datapointId) {
        var module = getModule();
        cleanErr(module._spfg_dp_remove(gridId, datapointId));
        releaseAllocations(gridId, null, datapointId);
    }

    /**
     * Creates a grid function.
     *
     * @param number gridId: id of the target grid
     * @param string type: refer to functionType for the available types
     * @param number phase: evaluation phase to attach the function to
     * @param array[number] inDpIds: list of datapoint ids to use as input for the function
     * @param array[number] outDpIds: list of datapoint ids to use as output for the function
     * @param string name: symbolic name for the function
     * @return number, the created function id
     */
    function spfgCreateFunction(gridId, type, phase, inDpIds, outDpIds, name) {

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

        var err = module._spfg_fn_create(
            gridId, functionTypes[type], phase,
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
        markAllocation(gridId, id, null, idOutPtr);
        markAllocation(gridId, id, null, nameInPtr);
        markAllocation(gridId, id, null, inDpIdsInPtr);
        markAllocation(gridId, id, null, outDpIdsInPtr);
        return id;
    }

    /**
     * Removes a grid function.
     *
     * @param number gridId
     * @param number functionId
     * @return undefined
     */
    function spfgRemoveFunction(gridId, functionId) {
        var module = getModule();
        cleanErr(module._spfg_dp_remove(gridId, functionId));
        releaseAllocations(gridId, null, functionId);
    }

    /**
     * Restart the evaluation cycle for the given grid.
     *
     * @param number gridId: id of the target grid
     * @return undefined
     */
    function spfgResetCycle(gridId) {
        cleanErr(getModule()._spfg_reset_cycle(gridId));
    }

    /**
     * Resumes the evaluation cycle for the given grid.
     *
     * @param number gridId: id of the target grid
     * @param number timestamp: number representing the passage of time
     * @param function callback [optional]: called upon each cycle iteration
     * @param object context [optional]: context to bind the callback upon each call
     * @return undefined
     */
    function spfgRunCycle(gridId, timestamp, callback, thisCtx) {
        var module = getModule();
        callback = (callback && thisCtx) ? callback.bind(thisCtx) : callback;
        var cbInPtr = (callback || 0) && module.addFunction(callback);
        cleanErr(module._spfg_run_cycle(gridId, timestamp, cbInPtr, 0));
    }

    /**
     * Imports a grid snapshot and either creates a new grid or updates an existing one.
     *
     * @param Object gridSnapshot: object containing a deserialized snapshot of a grid
     * @return number, the created or updated grid id
     */
    function spfgImportGridSnapshot(gridSnapshot) {

        if (!gridSnapshot) {
            throw Error('needs a non-empty object');
        }

        var module = getModule();
        var idOutPtr = module._malloc(4);
        var jsonData = JSON.stringify(gridSnapshot);
        var charsLength = jsonData.length * 4 + 1;
        var jsonInPtr = module._malloc(charsLength);
        module.stringToUTF8(jsonData, jsonInPtr, charsLength);
        module.setValue(idOutPtr, 0, 'i32');

        var err = module._spfg_gr_import_json(jsonInPtr, charsLength, idOutPtr);

        cleanErr(err, function(){
            module._free(idOutPtr);
            module._free(jsonInPtr);
        });

        module._free(jsonInPtr);
        var id = module.getValue(idOutPtr, 'i32');
        markAllocation(id, null, null, idOutPtr);
        return id;
    }

    /**
     *  Exports an existing grid into a deserialized JSON object
     *
     * @param number gridId: id of the target grid
     * @return Object, the deserialized JSON snapshot
     */
    function spfgExportGridSnapshot(gridId) {
        var module = getModule();
        var maxlen = 4096;
        var jsonOutPtr = module._malloc(maxlen);
        var outlenOutPtr = module._malloc(4);
        module.setValue(outlenOutPtr, 0, 'i32');

        var err = module._spfg_gr_export_json(gridId, jsonOutPtr, maxlen, outlenOutPtr);

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

    function spfgSetBoolean(gridId, datapointId, value) {
        var module = getModule();
        cleanErr(module._spfg_dp_set_bool(gridId, datapointId, value));
    }

    // --------------------------------------------------------------
    // Private functions
    // --------------------------------------------------------------

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
     * @param number grId
     * @param number fnId
     * @param number dpId
     * @return string, the lookup key
     */
    function allocationKey(grId, fnId, dpId) {
        return grId + '/' + fnId + '/' + dpId;
    }

    /**
     * Takes note of the pointer for a dynamically allocated memory area, and add it to a
     * pointer bucket for later deallocation.
     *
     * @param number grId
     * @param number fnId
     * @param number dpId
     * @param number pointer
     * @return undefined
     */
    function markAllocation(grId, fnId, dpId, pointer) {
        var key = allocationKey(grId, fnId, dpId);
        allocations[key] = allocations[key] || [];
        allocations[key].push(pointer);
    }

    /**
     * Deallocates memory from the pointers in a bucket.
     *
     * @param number grId
     * @param number fnId
     * @param number dpId
     * @return undefined
     */
    function releaseAllocations(grId, fnId, dpId) {
        var module = getModule();
        var key = allocationKey(grId, fnId, dpId);
        (allocations[key] || []).forEach(module._free.bind(module));
    }

})();

