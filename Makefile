# configuration
PYTHON ?= python3
PIP ?= $(PYTHON) -m pip
BUILD ?= $(PYTHON) -m build

DIST_DIR = dist
BUILD_DIR = build 
EGG_INFO = *.egg-info

# targets
.PHONY: all build install editable clean

all: build install

#Build wheel
build: 
	$(BUILD)

#Install wheel
install: build 
	$(PIP) install --force-reinstall $(DIST_DIR)/*whl

#Install editable
editable:
	$(PIP) install -e .

clean:
	rm -r $(BUILD_DIR)
	rm -r $(EGG_INFO)
	rm -r neucomm/*.so
	rm -r neucomm/__pycache__
	rm -r dist