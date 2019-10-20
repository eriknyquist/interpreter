SRC_DIR := source
OUTPUT_DIR := build
SRC_SUBDIRS := $(SRC_DIR)/runtime \
			   $(SRC_DIR)/frontend \
			   $(SRC_DIR)/backend \
			   $(SRC_DIR)/common

VPATH := $(SRC_SUBDIRS)
SRC_FILES := $(foreach dir,$(SRC_SUBDIRS),$(wildcard $(dir)/*.c))
OBJ_FILES := $(patsubst %.c,%.o,$(addprefix $(OUTPUT_DIR)/,$(notdir $(SRC_FILES))))

INCLUDE_FLAGS := $(addprefix -I, $(SRC_SUBDIRS))

PROGNAME := testexe
BUILD_OUTPUT := $(OUTPUT_DIR)/$(PROGNAME)

CFLAGS += -Wall $(INCLUDE_FLAGS)

.PHONY: clean

all: $(BUILD_OUTPUT)

$(BUILD_OUTPUT): output_dir $(OBJ_FILES)
	$(CC) $(LFLAGS) $(OBJ_FILES) -o $@

$(OUTPUT_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

output_dir:
	echo "$(INCLUDE_FLAGS)"
	[ -d $(OUTPUT_DIR) ] || mkdir -p $(OUTPUT_DIR)

clean:
	[ -d $(OUTPUT_DIR) ] && rm -rf $(OUTPUT_DIR)
