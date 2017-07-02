
SPFG = (function SPFG() {

    var errors = {
        SPFG_ERROR_NO:  0,
        SPFG_ERROR_BAD_PARAM_NULL_POINTER: -1,
        SPFG_ERROR_BAD_PARAM_INVALID_VALUE: -2,
        SPFG_ERROR_ALREADY_INITIALIZED: -3,
        SPFG_ERROR_NOT_INITIALIZED: -4,
        SPFG_ERROR_OUT_OF_SLOTS: -5,
        SPFG_ERROR_BAD_BLOCK_NAME: -6,
        SPFG_ERROR_NOT_FOUND: -7,
        SPFG_ERROR_INVALID_GR_ID: -8,
        SPFG_ERROR_INVALID_DP_ID: -9,
        SPFG_ERROR_INVALID_FN_ID: -10,
        SPFG_ERROR_CYCLE_FAILURE: -11,
        SPFG_ERROR_UNIMPLEMENTED: -12,
        SPFG_ERROR_REINDEX_FN: -13,
        SPFG_ERROR_BUFFER_OVERFLOW: -14,
    };

    var control = {
        SPFG_LOOP_CONTROL_STOP: 1,
    };

    var datapointTypes = {
        'int': 1,
        'real': 2,
        'bool': 3,
    };

    var functionTypes = {
        'inv(bool)->bool': 1,
        'and(bool,bool)->bool': 2,
    };

    CustomError = createCustomError();

    return {
        init: spfg_init,
        finish: spfg_finish,
        createGrid: spfg_gr_create,
        createDatapoint: spfg_dp_create,
        createFunction: spfg_fn_create,
        resetCycle: spfg_reset_cycle,
        runCycle: spfg_run_cycle,
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
     *
     */
    function spfg_init() {
        var module = getModule();
        checkErr(module._spfg_init());
    }

    /**
     *
     */
    function spfg_finish() {
        var module = getModule();
        checkErr(module._spfg_finish());
    }

    /**
     *
     * @param string name
     */
    function spfg_gr_create(name) {

        if (!name) {
            throw Error('needs a non-empty name');
        }

        var module = getModule();
        var idOutPtr = module._malloc(4);
        var charsLength = name.length * 4 + 1;
        var nameInPtr = module._malloc(charsLength);
        module.stringToUTF8(name, nameInPtr, charsLength - 1);
        checkErr(module._spfg_gr_create(idOutPtr, nameInPtr));
        return module.getValue(idOutPtr, 'i32');
    }

    /**
     *
     * @param number gridId
     * @param string type
     * @param string name
     */
    function spfg_dp_create(gridId, type, name) {

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
        module.stringToUTF8(name, nameInPtr, charsLength - 1);
        checkErr(module._spfg_dp_create(gridId, type, nameInPtr, idOutPtr));
        return module.getValue(idOutPtr, 'i32');
    }

    /**
     *
     * @param number gridId
     * @param string type
     * @param number phase
     * @param array[number] inDpIds
     * @param array[number] outDpIds
     * @param string name
     * @return number
     */
    function spfg_fn_create(gridId, type, phase, inDpIds, outDpIds, name) {

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
        module.stringToUTF8(name, nameInPtr, charsLength - 1);

        var inDpIdsArray = new Uint32Array(inDpIds);
        var inDpIdsInPtr = module._malloc(inDpIds.length * 4);
        module.HEAP32.set(inDpIdsArray, inDpIdsInPtr / 4);

        var outDpIdsArray = new Uint32Array(outDpIds);
        var outDpIdsInPtr = module._malloc(outDpIds.length * 4);
        module.HEAPU32.set(outDpIdsArray, outDpIdsInPtr / 4);

        checkErr(module._spfg_fn_create(
            gridId, type, phase,
            inDpIdsInPtr, inDpIds.length,
            outDpIdsInPtr, outDpIds.length,
            nameInPtr, idOutPtr));

        return module.getValue(idOutPtr, 'i32');
    }

    /**
     *
     * @param number gridId
     * @return undefined
     */
    function spfg_reset_cycle(gridId) {
        checkErr(getModule()._spfg_reset_cycle(gridId));
    }

    /**
     *
     * @param number gridId
     * @param number timestamp
     * @param function callback [optional]
     * @param object context [optional]
     * @return undefined
     */
    function spfg_run_cycle(gridId, timestamp, callback, thisCtx) {
        var module = getModule();
        callback = (callback && thisCtx) ? callback.bind(thisCtx) : callback;
        var cbInPtr = (callback || 0) && module.Runtime.addFunction(callback);
        checkErr(module._spfg_run_cycle(gridId, timestamp, cbInPtr, 0));
    }

    // --------------------------------------------------------------
    // Private functions
    // --------------------------------------------------------------

    /**
     *
     */
    function getModule() {
        return window.ModuleSPFG;
    }

    /**
     *
     * @param number err
     */
    function checkErr(err) {
        if (err != errors.SPFG_ERROR_NO) {
            throw new CustomError(err);
        }
    }

    /**
     *
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

})();

