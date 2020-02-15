SRC_DIR := source
OUTPUT_DIR := build

COMMON_DIRS := $(SRC_DIR)/common
RUNTIME_DIRS := $(SRC_DIR)/runtime
BACKEND_DIRS := $(SRC_DIR)/backend

INCLUDE_DIRS := $(SRC_DIR) $(COMMON_DIRS) $(RUNTIME_DIRS) $(BACKEND_DIRS)

COMMON_FILES := $(foreach dir,$(COMMON_DIRS),$(wildcard $(dir)/*.c))
RUNTIME_FILES := $(foreach dir,$(RUNTIME_DIRS),$(wildcard $(dir)/*.c))
BACKEND_FILES := $(foreach dir,$(BACKEND_DIRS),$(wildcard $(dir)/*.c))

COMMON_OBJ_FILES := $(patsubst %.c,%.o,$(addprefix $(OUTPUT_DIR)/,$(notdir $(COMMON_FILES))))
RUNTIME_OBJ_FILES := $(patsubst %.c,%.o,$(addprefix $(OUTPUT_DIR)/,$(notdir $(RUNTIME_FILES))))
BACKEND_OBJ_FILES := $(patsubst %.c,%.o,$(addprefix $(OUTPUT_DIR)/,$(notdir $(BACKEND_FILES))))

VPATH := $(INCLUDE_DIRS)

SRC_FILES := $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.c)) \
			 $(COMMON_FILES) \
			 $(RUNTIME_FILES) \
			 $(BACKEND_FILES)

OBJ_FILES := $(patsubst %.c,%.o,$(addprefix $(OUTPUT_DIR)/,$(notdir $(SRC_FILES))))

HASHTABLE_TEST_OBJ_FILES := test/hashtable_test.o $(COMMON_OBJ_FILES) $(RUNTIME_OBJ_FILES) $(BACKEND_OBJ_FILES)

INCLUDE_FLAGS := $(addprefix -I, $(INCLUDE_DIRS))

PROGNAME := testexe
BUILD_OUTPUT := $(OUTPUT_DIR)/$(PROGNAME)
HASHTABLE_TEST := $(OUTPUT_DIR)/hashtable_test

CFLAGS += -Wall $(INCLUDE_FLAGS)

.PHONY: all debug output_dir clean

VM_CONFIG_OPTS :=

VM_CONFIG_FLAGS := $(addprefix -D,$(VM_CONFIG_OPTS))

all: CFLAGS += -O3 $(VM_CONFIG_FLAGS)
all: $(BUILD_OUTPUT)

debug: CFLAGS += -g -O0 $(VM_CONFIG_FLAGS)
debug: $(BUILD_OUTPUT)

$(BUILD_OUTPUT): output_dir $(OBJ_FILES)
	$(CC) $(LFLAGS) $(OBJ_FILES) -o $@

hashtable_test: CFLAGS += -g -O0 $(VM_CONFIG_FLAGS)
hashtable_test: $(HASHTABLE_TEST)

$(HASHTABLE_TEST): output_dir $(HASHTABLE_TEST_OBJ_FILES)
	$(CC) $(LFLAGS) $(HASHTABLE_TEST_OBJ_FILES) -o $@

$(OUTPUT_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

output_dir:
	echo "$(INCLUDE_FLAGS)"
	[ -d $(OUTPUT_DIR) ] || mkdir -p $(OUTPUT_DIR)

clean:
	rm -rf $(OUTPUT_DIR)
