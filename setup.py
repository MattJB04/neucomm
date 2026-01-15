from distutils.core import setup, Extension

def main():
    setup(name="neucomm",
          ext_modules=[
              Extension("neucomm", sources=["neucomm.c", "basic_func.c, common.c"]),
          ]
    )

if __name__ == "__main__":
    main()