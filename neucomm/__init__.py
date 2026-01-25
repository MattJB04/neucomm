"""Python libraries for a neutrino communication project I'm doing

Usage includes (/will include, this is a work in progress) two main parts: 
An accelerator model that takes in a proton extraction and returns a 
neutrino beam, and a detector model that takes in a neutrino beam and
returns a list of times that a neutrino was detected (relative to the 
start of the extraction). 

This is(/will hopefully be) modular, so you can mix and match accelerator
models and detector models. There are some exceptions, typically simple 
models that simulate the entire process. Even with these simple models,
the start point will be an extraction"""

from ._core import simple_minerva, Extraction, NeutrinoFlux, numi_no_transverse, encode_convolution, decode_convolution

__all__ = [
    "simple_minerva",
    "Extraction",
    "NeutrinoFlux",
    "numi_no_transverse",
    "encode_convolution",
    "decode_convolution"]