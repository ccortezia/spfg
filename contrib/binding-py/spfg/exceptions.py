
class SPFGError(Exception):
    """Base exception for the library"""


class SPFGInvalidType(SPFGError):
    """Raised when an invalid mapped type is used in the API"""
