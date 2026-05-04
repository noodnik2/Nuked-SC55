# MAKEFILE_DIR points to the folder in which this Makefile is located
MAKEFILE_DIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# BINSRC points to the folder in which the ROMs (*.bin) are located
BINSRC=$(MAKEFILE_DIR)/share/nuked-sc55

# MIDISRC points to the folder in which the MIDIs (*.mid) are located
MIDISRC=$(MAKEFILE_DIR)/share/midis


help: ## Print this help
	@fgrep -h "##" $(MAKEFILE_LIST) | grep -v fgrep | sed 's/\(.*\):.*## \(.*\)/\1 - \2/' | sort
	@echo Also check out the scripts in the "scripts" folder.

.PHONY: clean
clean: ## clean the build environment
	rm -rf build

.PHONY: build
build: ## build the executables
	mkdir -p build
	cp $(BINSRC)/*.bin build
	(cd build; \
		cmake -DCMAKE_BUILD_TYPE=Release ..; \
		cmake --build .\
	)

.PHONY: dist
dist: ## copy the executable artifacts into the distribution folder
	mkdir -p dist
	cp build/nuked-sc55 dist
	cp build/nuked-sc55-render dist
	cp scripts/* dist

.PHONY: query-ports
query-ports: ## list the available input and output ports
	./build/nuked-sc55 --help

.PHONY: run-interactive
run-interactive: ## run the application interactively
	@echo "NOTES:"
	@echo "- After Nuked-SC55 App starts, use 'playmidi' to send a file to its input port"
	@echo "- If input or output sources aren't correct, check values for -p (input) and -a (output) ports using the --help option"
	@echo "- Use the 'Audio MIDI Setup / MIDI Server' to configure device mappings"
	./build/nuked-sc55 -d share/nuked-sc55 --mk2 -p 1 -a 0

.PHONY: run-render
run-render: ## example of running the application's renderer
	MIDIFILE=55luv_me
	MIDIPATH=$(MIDISRC)/$(MIDIFILE).mid
	WAVPATH=$(MIDISRC)/$(MIDIFILE).mid
	./build/nuked-sc55-render -d build -r gs -o $(WAVPATH) $(MIDIPATH)
