module TestSuiteDecode exposing (..)

import Expect exposing (Expectation)
import Json.Decode exposing (decodeString)
import Spfg.Decode exposing (grDecoder)
import Test exposing (..)
import TestData


grDecoderSuite : Test
grDecoderSuite =
    describe "grDecoder"
        [ test "when fed with minimal data should result in the proper data structure" <|
            \_ ->
                Expect.equal
                    (decodeString grDecoder TestData.minimalGridJson)
                    (Ok TestData.minimalGridNative)
        , test "when fed with data for a single datapoint should result in the proper data structure" <|
            \_ ->
                Expect.equal
                    (decodeString grDecoder TestData.partialGridJson)
                    (Ok TestData.partialGridNative)
        , test "when fed with data for a single datapoint and a single function should result in the proper data structure" <|
            \_ ->
                Expect.equal (decodeString grDecoder TestData.completeGridJson)
                    (Ok TestData.completeGridNative)
        ]
