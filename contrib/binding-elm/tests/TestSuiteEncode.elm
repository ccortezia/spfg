module TestSuiteEncode exposing (..)

import Expect exposing (Expectation)
import Json.Encode
import Spfg.Encode exposing (grEncoder)
import Test exposing (..)
import TestData
import TestUtils exposing (packedJson)


grEncoderSuite : Test
grEncoderSuite =
    describe "grEncoder"
        [ test "when fed with minimal native data should result in the proper json" <|
            \_ ->
                Expect.equal
                    (packedJson <| Json.Encode.encode 1 (Spfg.Encode.grEncoder TestData.minimalGridNative))
                    (packedJson TestData.minimalGridJson)
        , test "when fed with native data for a single datapoint should result in the proper json" <|
            \_ ->
                Expect.equal
                    (packedJson <| Json.Encode.encode 1 (Spfg.Encode.grEncoder TestData.partialGridNative))
                    (packedJson TestData.partialGridJson)
        , test "when fed with native data for a single datapoint and a single function should result in the proper json" <|
            \_ ->
                Expect.equal
                    (packedJson <| Json.Encode.encode 1 (Spfg.Encode.grEncoder TestData.completeGridNative))
                    (packedJson TestData.completeGridJson)
        ]
