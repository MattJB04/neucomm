from ._core import simple_minerva, Extraction

__all__ = ["simple_minerva", "Extraction"]

def simple_minerva(extraction: Extraction) -> list:
    """A very basic model. Takes in a proton extraction, and applies the MINERvA experimental event rate (0.81 per 2.25e13 POT). Returns the times of detected neutrinos, relative to the start of an extraction
    Note: bunches larger than 10e20 may take a while"""
    ...

class Extraction:
    """The start point when modelling. This is a rather simple version that does not take into account bunch width - I might make a more complicated version if given enough time
    This is passed to models - usually returning a neutrino beam or, for a full model, the times of neutrino detection"""
    ...
    def __init__(
            self,
            ppb: float,
            bunches: int,
            bunch_spacing: float,
            rise_time: float,
            fall_time:float,
    ) -> None:
        """Initialise new extraction
        Parameters:
        ----------
        ppb : float
            protons per bunch. 
        bunches : int
            bunches per extraction
        bunchspacing : float
            Spacing of bunches (seconds)
        rise_time : float
            Rise time of kicker magnet (seconds)
        fall_time : float
            Fall time of kicker magnet (seconds)"""
        ...

