from setuptools import setup, Extension
import sys

setup(
    ext_modules=[
        Extension(
            name="neucomm._core",
            sources=[
                "neucomm/_core.c",
                "neucomm/basic_func.c",
                "neucomm/common.c",
                "neucomm/accelerators.c",
                "neucomm/neutrino_flux.c",
                "neucomm/detectors.c",
                "neucomm/codes/codes.c",
                "neucomm/codes/convoluted.c",
                "neucomm/Finalis/water_expectation.c"
            ],
            include_dirs=["neucomm", "neucomm/codes", "neucomm/Finalis"],
            extra_compile_args=['/std:c11'] if sys.platform == 'win32' else ['-std=c99'],
        )
    ]
)