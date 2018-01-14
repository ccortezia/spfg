port module Main exposing (..)

-- import Svg exposing (rect, svg)
-- import Svg.Attributes exposing (..)
-- import Html.Events exposing (..)
-- import Spfg.Encode

import Html exposing (..)
import Json.Decode as Json exposing (..)
import Json.Encode
import Spfg.Decode exposing (grDecoder)
import Spfg.Types as Spfg
import Time exposing (Time, second)


main =
    Html.program
        { init = init
        , view = view
        , update = update
        , subscriptions = subscriptions
        }


type alias Model =
    { data : Spfg.GrInfo, status : String, iteration : Int }


init : ( Model, Cmd Msg )
init =
    let
        jsonData =
            """{"id": 0, "name": "gr0", "dps": [], "fns": []}"""
    in
    case Json.decodeString grDecoder jsonData of
        Ok decoded ->
            ( { data = decoded, status = "OK", iteration = 0 }, Cmd.none )

        Err message ->
            ( { data = Spfg.GrInfo 0 "gr0" [] [], status = message, iteration = 0 }, Cmd.none )


type Msg
    = Tick Int
    | Inbound Json.Encode.Value


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        Tick value ->
            ( { model | iteration = model.iteration + 1 }, ping value )

        Inbound data ->
            case decodeValue grDecoder data of
                Ok decoded ->
                    ( { model | data = decoded, status = "OK" }, Cmd.none )

                Err message ->
                    ( { model | status = message ++ " ==> " ++ toString data }, Cmd.none )


port ping : Int -> Cmd msg


port pong : (Json.Encode.Value -> msg) -> Sub msg


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ pong Inbound
        , Time.every second (\t -> Tick model.iteration)
        ]



-- { model
--     | id = 0
--     , name = "gr0"
--     , dps =
--         Array.fromList
--             [ DpInfo 0 1 "dp0" (DpValBool True) True
--             , DpInfo 1 1 "dp1" (DpValBool True) True
--             , DpInfo 2 1 "dp2" (DpValBool True) True
--             ]
--     , fns =
--         Array.fromList
--             [ FnInfo 0 1 "fn0" 1 (Array.fromList [ 0, 1 ]) (Array.fromList [ 2 ])
--             ]
-- }
-- view : Model -> Html Msg
-- view model =
--     div []
--         [ h1 [] [ Html.text "First Attempt" ]
--         , svg
--             [ width "120", height "120", viewBox "0 0 120 120" ]
--             [ rect [ x "10", y "10", width "100", height "100", rx "15", ry "15" ] [] ]
--         ]


view : Model -> Html Msg
view model =
    div []
        [ strong [] [ text model.data.name ]
        , button [] [ text "PING" ]
        , div [] [ text model.status ]
        , table []
            (List.concat
                [ [ tr [] [ td [] [ text "DPS" ] ] ]
                , List.map dpRow <| model.data.dps
                , [ tr [] [ td [] [ text "FNS" ] ] ]
                , List.map fnRow <| model.data.fns
                ]
            )
        ]


dpRow : Spfg.DpInfo -> Html Msg
dpRow dpInfo =
    tr []
        [ td [] [ text <| toString dpInfo.id ]
        , td [] [ text dpInfo.name ]
        , td [] [ text <| "value: " ++ Spfg.Decode.dpValFormat dpInfo.value ]
        , td [] [ text <| "emitted: " ++ toString dpInfo.emitted ]
        ]


fnRow : Spfg.FnInfo -> Html Msg
fnRow fnInfo =
    tr []
        [ td [] [ text <| toString fnInfo.id ]
        , td [] [ text fnInfo.name ]
        , td [] [ text <| toString fnInfo.inDpIds ]
        , td [] [ text <| toString fnInfo.outDpIds ]
        ]
