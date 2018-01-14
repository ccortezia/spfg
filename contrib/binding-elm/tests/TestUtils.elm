module TestUtils exposing (..)


packedJson : String -> String
packedJson =
    String.filter (\c -> c /= '\n' && c /= ' ')
