

#
#	Variables; can be overridden from the command line
#

# MAKEFILE_DIR points to the folder in which this Makefile is located; everything else will be relative to here
MAKEFILE_DIR=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))

# BUILD_DIR points to the folder where the stuff will be built into
BUILD_DIR=$(MAKEFILE_DIR)/build

# DIST_DIR points to the folder where the stuff that's built will be run from
DIST_DIR=$(MAKEFILE_DIR)/dist

# MIDIS_DIR points to the folder in which the MIDIs (*.mid) are located
MIDIS_DIR=$(MAKEFILE_DIR)/share/midis

# OUTPUT_DIR points to the folder to which output is written
OUTPUT_DIR=$(MAKEFILE_DIR)/share/output

# TEST_MIDI_PATH points to the file that will be used for testing
TEST_MIDI_PATH=test/integration/avmidi/01.mid


#
#	Macros
#

define assert_paths_match
	@EXPECTED_PATH=$(abspath $(1)); \
	ACTUAL_PATH=$(abspath $(2)); \
	if [ "$$EXPECTED_PATH" != "$$ACTUAL_PATH" ]; then \
		echo "Error: Path mismatch!"; \
		echo "Expected: $$EXPECTED_PATH"; \
		echo "Actual:   $$ACTUAL_PATH"; \
		exit 1; \
	fi
endef


#
#	Targets
#

.PHONY: help
help: ## Print this help
	@fgrep -h "##" $(MAKEFILE_LIST) | grep -v fgrep | sed 's/\(.*\):.*## \(.*\)/\1 - \2/' | sort
	@echo ""
	@echo "E.g., - 'make clean build dist'"
	@echo ""
	@echo "Also check out the scripts in the 'scripts' folder."

.PHONY: clean
# NOTE: we use constants instead of variables for safety; update the script if these values change.
clean: ## clean the build environment
	$(call assert_paths_match, $(BUILD_DIR), build)
	rm -rf build dist

.PHONY: build
build: ## build the executables
	mkdir -p $(BUILD_DIR)
	(cd $(BUILD_DIR); \
		cmake -DCMAKE_BUILD_TYPE=Release ..; \
		cmake --build .\
	)

.PHONY: dist
dist: build ## copy the executable artifacts into the distribution folder
	mkdir -p $(DIST_DIR)
	cp $(BUILD_DIR)/nuked-sc55 $(DIST_DIR)
	cp $(BUILD_DIR)/nuked-sc55-render $(DIST_DIR)
	cp scripts/* $(DIST_DIR)

.PHONY: query-ports
query-ports: ## list the available input and output ports
	$(DIST_DIR)/nuked-sc55 --help

.PHONY: run-render
run-render: dist ## example of running the application's renderer
	@echo "NOTES:"
	@echo "- Select the test file to be rendered with e.g., 'make TEST_MIDI_PATH=test/integration/avmidi/05.mid run-render'"
	$(DIST_DIR)/nuked-sc55-render -r gs -o $(OUTPUT_DIR)/test.wav $(TEST_MIDI_PATH)

.PHONY: run-interactive
run-interactive: dist ## run the application interactively
	@echo "NOTES:"
	@echo "- After Nuked-SC55 App starts, send MIDI events to its input port (e.g., using the 'play-test' target)."
	@echo "- If input or output sources aren't correct, check values for -p (input) and -a (output) ports using the --help option."
	@echo "- Use the 'Audio MIDI Setup / MIDI Server' to configure device mappings."
	$(DIST_DIR)/nuked-sc55 --romset mk2 -p 1 -a 0

.PHONY: play-test
play-test: ## play a test MIDI file to the input Bus of a running interactive server
	@echo "NOTES:"
	@echo "- Sending MIDI events to the input port of a running Nuked-SC55 Interactive server (e.g., already started using the 'run-interactive' target)."
	@echo "- Select the test file to be played with e.g., 'make TEST_MIDI_PATH=test/integration/avmidi/05.mid play-test'"
	playmidi $(TEST_MIDI_PATH) 1
