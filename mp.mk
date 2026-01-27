PYTHON ?= python3

MPVERSION_HEADER = $(GENHDR_DIR)/mpversion.h
ROOT_POINTERS_HEADER = $(GENHDR_DIR)/root_pointers.h
MODULEDEFS_HEADER = $(GENHDR_DIR)/moduledefs.h
QSTR_GENERATED_HEADER = $(GENHDR_DIR)/qstrdefs.generated.h

$(GENHDR_DIR):
	mkdir -p $@

$(MPVERSION_HEADER): | $(GENHDR_DIR)
	@echo "  GEN: mpversion.h"
	$(PYTHON) $(MICROPYTHON_PATH)/py/makeversionhdr.py $@

$(ROOT_POINTERS_HEADER): $(MICROPYTHON_SRC) | $(GENHDR_DIR)
	@echo "  GEN: root_pointers.h"
	$(PREFIX)-gcc -E \
		-DNO_QSTR \
		$(CFLAGS) \
		-D'MP_REGISTER_ROOT_POINTER(x)=MP_REGISTER_ROOT_POINTER(x)' \
		$(filter %.c, $(MICROPYTHON_SRC)) > $(GENHDR_DIR)/root_pointers.collected

	$(PYTHON) $(MICROPYTHON_PATH)/py/make_root_pointers.py \
		$(GENHDR_DIR)/root_pointers.collected > $@

$(QSTR_GENERATED_HEADER): $(MICROPYTHON_SRC) $(MPVERSION_HEADER) $(ROOT_POINTERS_HEADER) $(MODULEDEFS_HEADER) | $(GENHDR_DIR)
	@echo "  QSTR: Scanning source files..."
	$(PYTHON) $(MICROPYTHON_PATH)/py/makeqstrdefs.py pp \
		$(PREFIX)-gcc -E \
		output $(GENHDR_DIR)/qstr.collected \
		cflags -DNO_QSTR $(CFLAGS) \
		sources $(filter %.c, $(MICROPYTHON_SRC)) \
		dependencies $(GENHDR_DIR)/qstr.dep

	cat $(MICROPYTHON_PATH)/py/qstrdefs.h $(GENHDR_DIR)/qstr.collected > $(GENHDR_DIR)/qstrdefs.raw.h

	@echo "  QSTR: Preprocessing..."
	$(PREFIX)-gcc -E -P \
		-DNO_QSTR \
		$(CFLAGS) \
		$(GENHDR_DIR)/qstrdefs.raw.h > $(GENHDR_DIR)/qstrdefs.pre.h
	sed 's/MP_QSTR_\([a-zA-Z0-9_]*\)/\nQ(\1)\n/g' $(GENHDR_DIR)/qstrdefs.pre.h > $(GENHDR_DIR)/qstrdefs.post.h

	@echo "  QSTR: Generating header..."
	$(PYTHON) $(MICROPYTHON_PATH)/py/makeqstrdata.py $(GENHDR_DIR)/qstrdefs.post.h > $@

$(MODULEDEFS_HEADER): $(QSTR_GENERATED_HEADER) | $(GENHDR_DIR)
	@echo "  GEN: moduledefs.h"
	$(PREFIX)-gcc -E \
		-DNO_QSTR \
		$(CFLAGS) \
		-D'MP_REGISTER_MODULE(name, obj)=MP_REGISTER_MODULE(name, obj)' \
		$(filter %.c, $(MICROPYTHON_SRC)) > $(GENHDR_DIR)/moduledefs.collected

	$(PYTHON) $(MICROPYTHON_PATH)/py/makemoduledefs.py \
		$(GENHDR_DIR)/moduledefs.collected > $@

$(TARGET).c : $(MODULEDEFS_HEADER)
