V ?= 1

ifeq ($(V),0)
  Q := @
endif
ifeq ($(V),1)
  Q := @
  define cmd-print
    @echo '$(1)'
  endef
endif
ifeq ($(V),2)
  Q := @
  define cmd-print
    @echo '$(1)'
  endef
  define cmd-info
    @echo '$(1)'
  endef
endif


define cmd-ar ?=
  $(call cmd-print,  AR      $(strip $(1)))
  $(Q)ar cr $(1) $(2)
endef

define cmd-as-64 ?=
  $(call cmd-print,  AS      $(strip $(1)))
  $(Q)gcc -Wall -Wextra -O2 -nostdlib -nodefaultlibs -c $(2) -o $(1) \
          $(patsubst %, -I%, $(3))
endef

define cmd-as-32 ?=
  $(call cmd-print,  AS      $(strip $(1)))
  $(Q)gcc -Wall -Wextra -O2 -nostdlib -nodefaultlibs -c $(2) -o $(1) -m32 \
          $(patsubst %, -I%, $(3))
endef

define cmd-cc ?=
  $(call cmd-print,  CC      $(strip $(1)))
  $(Q)gcc -Wall -Wextra -O2 -c $(2) -o $(1) $(addprefix -I, $(3))
endef

define cmd-cc-64 ?=
  $(call cmd-print,  CC      $(strip $(1)))
  $(Q)gcc -Wall -Wextra -O2 -nostdlib -nodefaultlibs -fno-builtin -Wno-main \
          -c $(2) -o $(1) $(addprefix -I, $(3))
endef

define cmd-cc-32 ?=
  $(call cmd-print,  CC      $(strip $(1)))
  $(Q)gcc -Wall -Wextra -O2 -nostdlib -nodefaultlibs -fno-builtin \
          -ffreestanding -c $(2) -o $(1) $(addprefix -I, $(3)) -m32
endef

define cmd-ghs-32 ?=
  $(call cmd-print,  GEN     $(strip $(1)))
  $(Q)gcc -Wall -Wextra -O2 -nostdlib -nodefaultlibs -fno-builtin \
          -ffreestanding -S -c $(2) -o $(1) $(addprefix -I, $(3)) -m32
  $(Q)perl -wnli \
           -e '/^\s*ASSEMBLY_DECLARE_VALUE (\S+) \$$(\S+)\s*$$/ or next;' \
           -e 'printf("#define %s %s\n", $$1, $$2);' $(1)
endef

define cmd-ccld ?=
  $(call cmd-print,  CCLD    $(strip $(1)))
  $(Q)gcc -Wall -Wextra -O2 $(2) -o $(1) $(3)
endef

define cmd-ld ?=
  $(call cmd-print,  LD      $(strip $(1)))
  $(Q)ld -o $(1) -T $(2) -z max-page-size=0x1000 $(3)
endef

define cmd-map ?=
  $(call cmd-print,  MAP     $(strip $(1)))
  $(Q)nm $(2) | perl -wpl -e 's/^(.*) . (.*)$$/0x$$1 $$2/' > $(1)
endef


define cmd-mkdir ?=
  $(call cmd-info,  MKDIR   $(strip $(1)))
  $(Q)mkdir $(1)
endef

define cmd-make ?=
  $(call cmd-info,  MAKE    $(strip $(1)))
  $(Q)+$(MAKE) $(1)
endef

define cmd-cp ?=
  $(call cmd-info,  CP      $(strip $(1)))
  $(Q)rm -rf $(1) 2> /dev/null || true
  $(Q)cp -R $(2) $(1)
endef

define cmd-touch ?=
  $(call cmd-info,  TOUCH   $(strip $(1)))
  $(Q)touch $(1)
endef

define cmd-clean ?=
  $(call cmd-print,  CLEAN)
  $(Q)rm -rf $(1) 2> /dev/null || true
endef


define cmd-iso ?=
  $(call cmd-print,  ISO     $(strip $(1)))
  $(Q)grub-mkrescue -d /usr/lib/grub/i386-pc -o $(1) --modules=multiboot2 \
    $(2) 2> /dev/null
endef

define cmd-call ?=
  $(call cmd-print,  CALL    $(strip $(1)))
  $(Q)./$(strip $(1)) $(2)
endef

define cmd-boot ?=
  $(call cmd-print,  BOOT    $(strip $(1)))
  $(Q)qemu-system-x86_64 -enable-kvm -cpu host -smp 8 -m 48M \
    -drive file=$(strip $(1)),format=raw -monitor stdio
endef

define cmd-gdb ?=
  $(call cmd-print,  BOOT    $(strip $(1)))
  $(Q)rm .qemu.pid 2> /dev/null || true
  $(Q)qemu-system-x86_64 -enable-kvm -smp 8 -m 48M \
    -drive file=$(strip $(1)),format=raw -s -S -pidfile .qemu.pid &
  $(Q)while [ ! -e .qemu.pid ] ; do sleep 1 ; done
  $(Q)gdb $(addprefix -s , $(2)) -x .gdbinitz
  $(Q)rm .qemu.pid 2> /dev/null || true
endef


define cmd-dep ?=
  $(call cmd-info,  DEP     $(strip $(1)))
  $(Q)cat $(2) > $(1)
endef

define cmd-depc ?=
  $(call cmd-info,  DEP     $(strip $(1)))
  $(Q)gcc -MM $(2) -o $(1) -MT $(3) $(addprefix -I, $(4))
endef

define cmd-depas ?=
  $(call cmd-info,  DEP     $(strip $(1)))
  $(Q)gcc -MM $(2) -o $(1) -MT $(3) $(addprefix -I, $(4))
endef
