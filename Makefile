OBJ := obj/
LIB := lib/
BIN := bin/

MAKEFLAGS += --no-print-directory --no-builtin-rules --no-builtin-variables


include Command.mk
include Function.mk


generated-files := $(patsubst %, $(OBJ)%, \
  include/loader/offsets.h)

loader-sources := $(patsubst %, loader/%, \
    allocator.c entry.S jumpasm.S jumper.c main.c multiboot2.c pagetable.c \
    printk.c vga.c)
loader-objects := $(patsubst %.S, $(OBJ)%.o, \
                    $(patsubst %.c, $(OBJ)%.o, $(loader-sources)))

mistral-sources := $(patsubst %, mistral/%, \
  basic.c loader.c main.c mistral.c multiboot2.c payload.c print.c)
mistral-objects := $(patsubst %.c, $(OBJ)%.o, $(mistral-sources))


config-commands := clean .generated .depends $(generated-files)

ifeq ($(filter $(config-commands), $(MAKECMDGOALS)),)
  mode := build
else
  ifeq ($(filter-out $(config-commands), $(MAKECMDGOALS)),)
    mode := config
  else
    mode := mixed
  endif
endif


ifeq ($(mode),mixed)

  .NOTPARALLEL:

  %:
	$(call cmd-make, $@)

else


all: $(BIN)loader $(BIN)loader.map $(BIN)mistral

boot: $(BIN)mistral.iso
	$(call cmd-boot, $(BIN)mistral.iso)

check: $(BIN)mistral $(BIN)payload $(BIN)loader
	$(call cmd-call, $<, $(BIN)payload -s -o /dev/null -C $(BIN))

clean:
	$(call cmd-clean, $(OBJ) $(LIB) $(BIN) .generated .depends .depends.d)


$(call REQUIRE-DIR, $(BIN)loader $(BIN)loader.map $(BIN)mistral)

$(BIN)loader: loader/loader.ld $(loader-objects)
	$(call cmd-ld, $@, $<, $(filter-out $<, $^))

$(BIN)loader.map: $(BIN)loader
	$(call cmd-map, $@, $<)

$(BIN)mistral: $(mistral-objects)
	$(call cmd-ccld, $@, $^)


$(call REQUIRE-DIR, $(BIN)payload $(BIN)kernel $(BIN)mistral.iso)

$(BIN)payload: utils/payload/payload.ld $(OBJ)utils/payload/entry.o \
               $(OBJ)utils/payload/main.o
	$(call cmd-ld, $@, $<, $(filter-out $<, $^))

$(BIN)kernel: $(BIN)mistral $(BIN)payload $(BIN)loader | $(BIN)
	$(call cmd-call, $<, $(BIN)payload -C $(BIN) -o $@ \
                               -a 0xffffffff80000000)

$(BIN)mistral.iso: $(OBJ)iso
	$(call cmd-iso, $@, $<)


$(call REQUIRE-DIR, $(OBJ)iso)

$(OBJ)iso: $(BIN)kernel $(call FIND, utils/iso) | $(OBJ)
	$(call cmd-cp, $(OBJ)iso, utils/iso)
	$(call cmd-cp, $(OBJ)iso/boot/kernel, $(BIN)kernel)

.generated: $(generated-files)
	$(call cmd-touch, $@)


$(call REQUIRE-DIR, $(generated-files))
$(call REQUIRE-DIR, $(loader-objects))
$(call REQUIRE-DIR, $(mistral-objects))

$(call REQUIRE-DEP, $(loader-sources))
$(call REQUIRE-DEP, $(mistral-sources))

$(OBJ)include/loader/%.h: include/loader/%.h.c
	$(call cmd-ghs-32, $@, $<, include/)

$(OBJ)loader/%.o: loader/%.c
	$(call cmd-cc-32, $@, $<, include/)

$(OBJ)loader/%.o: loader/%.S
	$(call cmd-as-32, $@, $<, include/ $(OBJ)include/)

$(OBJ)mistral/%.o: mistral/%.c
	$(call cmd-cc, $@, $< -D__USERLAND__, include/)


$(call REQUIRE-DIR, $(OBJ)utils/payload/entry.o $(OBJ)utils/payload/main.o)

$(OBJ)utils/payload/%.o: utils/payload/%.c
	$(call cmd-cc-64, $@, $<, include/mistral)

$(OBJ)utils/payload/%.o: utils/payload/%.S
	$(call cmd-as-64, $@, $<, include/mistral)


.depends: .generated
	$(call cmd-dep, $@, $(filter %.d, $^))

.depends.d/loader/%.c.d: loader/%.c
	$(call cmd-depc, $@, $<, $(patsubst %.c, $(OBJ)%.o, $<), include/)

.depends.d/mistral/%.c.d: mistral/%.c
	$(call cmd-depc, $@, $< -D__USERLAND__, \
               $(patsubst %.c, $(OBJ)%.o, $<), include/)

.depends.d/%.S.d: %.S
	$(call cmd-depas, $@, $<, $(patsubst %.S, $(OBJ)%.o, $<), include/ \
               $(OBJ)include/)


ifeq ($(mode),build)
  -include .depends
  -include .generated
endif

endif
