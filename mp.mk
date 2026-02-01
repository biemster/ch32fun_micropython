PYTHON ?= python3

MPVERSION_HEADER = $(GENHDR_DIR)/mpversion.h
ROOT_POINTERS_HEADER = $(GENHDR_DIR)/root_pointers.h
MODULEDEFS_HEADER = $(GENHDR_DIR)/moduledefs.h
QSTR_GENERATED_HEADER = $(GENHDR_DIR)/qstrdefs.generated.h
HWDEF_HEADERS = $(GENHDR_DIR)/ch32fun_hwdefs.collected
PINDEF_HEADER = $(GENHDR_DIR)/ch32fun_pindefs.h
REGDEF_HEADER = $(GENHDR_DIR)/ch32fun_regdefs.h
ISLERDEF_HEADER = $(GENHDR_DIR)/ch32fun_islerdefs.h
ISLERREG_HEADER = $(GENHDR_DIR)/ch32fun_islerregs.h

$(GENHDR_DIR):
	mkdir -p $@
	touch $(PINDEF_HEADER)
	touch $(REGDEF_HEADER)
	touch $(ISLERDEF_HEADER)
	touch $(ISLERREG_HEADER)

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

$(HWDEF_HEADERS): | $(GENHDR_DIR)
	@echo "  GEN: ch32fun_{pin,reg}defs.h"
	$(PREFIX)-gcc -E -dM -DNO_QSTR $(CFLAGS) ./*.c > $@

	sed 's/.*define \(P[ABCD][0-9]\{1,2\}\).*/{ MP_ROM_QSTR(MP_QSTR_\1),   MP_ROM_INT(\1) },/;t;d' $@ |sort|uniq > $(PINDEF_HEADER)
	sed 's/.*define \(R[0-9]\{1,2\}_.* \).*vu\([0-9]\{1,2\}\).*/{ MP_QSTR_\1, (uintptr_t)\&\1, W\2 },/;t;d' $@ |sort|uniq > $(REGDEF_HEADER)
	sed 's/.*define \(LL_TX_POWER.* \).*/{ MP_ROM_QSTR(MP_QSTR_\1), MP_ROM_INT(\1) },/;t;d' $@ |sort|uniq > $(ISLERDEF_HEADER)
	sed 's/\tvolatile uint32_t \(..\)\([0-9]\{1,2\}\).*/{ MP_QSTR_\1_\1\2, (uintptr_t)\&\1->\1\2, W32 },/;t;d' $(CH32FUN_PATH)/extralibs/iSLER.h $@ > $(ISLERREG_HEADER)
	sed 's/.*define \([A-Z0-9_]*\) \([BLR][BLF]\)\([0-9]\{1,2\}\).*/{ MP_QSTR_\2_\1, (uintptr_t)\&\2->\2\3, W32 },/;t;d' $@ >> $(ISLERREG_HEADER)

$(QSTR_GENERATED_HEADER): $(MICROPYTHON_SRC) $(MPVERSION_HEADER) $(ROOT_POINTERS_HEADER) $(HWDEF_HEADERS) | $(GENHDR_DIR)
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
