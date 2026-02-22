from ._core import simple_minerva, Extraction, NeutrinoFlux, numi_no_transverse, encode_convolution, decode_convolution, cylindrical_water
from ._core import bits_to_string, string_to_bits, water_expectation, cross_section_huber, Huber_flux, distribution, MINERvA_flux, poisson

__all__ = [
    "simple_minerva",
    "Extraction",
    "NeutrinoFlux",
    "numi_no_transverse",
    "encode_convolution",
    "decode_convolution",
    "cylindrical_water",
    "bits_to_string",
    "string_to_bits",
    "cross_section_huber",
    "water_expectation",
    "Huber_flux",
    "distribution",
    "MINERvA_flux",
    "poisson"]

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

class distribution:
    """A distribution, used for the flux and cross-section distributions. This is a simple histogram distribution, with values and bounds. The value of the distribution between bounds[i] and bounds[i+1] is values[i]"""
    ...

class NeutrinoFlux:
    """A neutrino flux"""
    ...

def numi_no_transverse(extraction: Extraction) -> NeutrinoFlux:
    """numi with no transeverse momentum - still a work in progress!"""
    ...

def encode_convolution(string: list) -> list:
    """A convolution encoder, with a rate of 1/2 and constraint length of 7 (The same that NuMI used)
    
    Takes in a string, and returns the encoded binary as a list"""
    ...

def decode_convolution(binary: list) -> list:
    """A convolution decoder, using the viterbi algorith. Generates the trellis using the encode_convolution method.
    
    Takes in a list of binary, and returns a string"""
    ...

def cylindrical_water(flux: NeutrinoFlux, distance: float, radius: float, length: float, samples: int) -> list:
    """Cylindrical water Cherenkov detector. Input a neutrino flux and relavent parameters to generate a list of times that a neutrino was detected.

    Due to limits on computational power, a statistical approach is used. The detector samples from a distribution. 
    Having a larger number of samples will make the simulation more accurate, but will take longer to run.

    Setting the samples to -1 will attempt to sample for each individual proton. This is NOT RECOMMENDED (and frankly, is not really possible unless
    you have a super computer laying around)
    
    Something is seriously wrong this set up. I am not sure if the problem is with the detector or with the distribution function, but the detection 
    rates are abysmall
    Small update, doing the calculation by hand also leads to an abysmally low detection rate. So clearly I am doing something wrong. If I has to
    guess, it is the data on neutrino flux I am using is not accurate (I am sure the paper is accurate for its purposes, but I am probably not
    applying it correctly)"""
    ...

def string_to_bits(string: str) -> list:
    """Takes in a string, and returns a list of bits for the characters. Probably best to stick with ASCII for this on"""
    ...
def bits_to_string(bits: list) -> str:
    """Takes in a list of bits, and returns the ASCII values of those bits as a string"""
    ...

def water_expectation(flux: distribution, cross_section: distribution, path_length_option: int, distance: float) -> float:
    """Calculates the expected number of detections in a water cherenkov detector given a neutrino flux and a cross-section distribution. This is used for testing the cylindrical_water function, and is not intended to be used by itself"""
    ...

def cross_section_huber(energy: list) -> distribution:
    """Calculates the Huber cross-section distribution for a given set of parameters. This is used for testing the water_expectation function, and is not intended to be used by itself"""
    ...

def Huber_flux(energy: list, E_0: float, L: float, N_mu: float, area: float) -> distribution:
    """Calculates the Huber flux distribution for a given set of parameters. This is used for testing the water_expectation function, and is not intended to be used by itself"""
    ...

def MINERvA_flux(POT=2.23e13) -> distribution:
    """MINERvA flux from the MINERvA flux predictions paper. Leave arguments empty for the same number of POT then NUMI. Changing the number scales linearly - potententially not accurate"""
    ...

def poisson(expectation: float) -> int:
    """Returns a random sample from a poisson distribution"""
    ...