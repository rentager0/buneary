V ?= 1

ifeq ($(V),4)
  define CMD-INFO
    $(info $(1))
  endef
endif


define NOSLASH
  $(if $(filter %/, $(1)),                    \
    $(call NOSLASH, $(patsubst %/, %, $(1))), \
    $(1))
endef


define __FIND
  $(1) $(foreach e, $(1), \
    $(call FIND, $(wildcard $(strip $(e))/*)))
endef

define FIND
  $(if $(1), $(strip $(call __FIND, $(call NOSLASH, $(1)))))
endef


define __SPLIT
  $(if $(filter-out ., $(call NOSLASH, $(1))), \
    $(call __SPLIT, $(dir $(call NOSLASH, $(1))), \
                    $(notdir $(call NOSLASH, $(1))) $(2)), \
    $(2))
endef

define __CUMULATE
  $(if $(filter 1, $(words $(1))), $(2) $(1), \
    $(call __CUMULATE, \
      $(word 1, $(1))/$(word 2, $(1)) $(wordlist 3, $(words $(1)), $(1)), \
      $(2) $(word 1, $(1))))
endef

define PREFIXES
$(strip \
  $(call __CUMULATE, $(call __SPLIT, $(1))))
endef


define REVERSE
  $(if $(strip $(1)), \
    $(call REVERSE, $(wordlist 2, $(words $(1)),$(1)), $(word 1, $(1)) $(2)), \
    $(2))
endef


REQUIRE-DIR-DONE := .

define __GENERATE-DIR-TEMPLATE
  $(call CMD-INFO, $(strip $(1)): | $(strip $(call NOSLASH, $(dir $(1)))))
  $(call CMD-INFO,         $$(call cmd-mkdir, $$@))

  $(1): | $(call NOSLASH, $(dir $(1)))
	$$(call cmd-mkdir, $$@)

  REQUIRE-DIR-DONE += $(1)

  $(call GENERATE-DIR, $(call NOSLASH, $(dir $(1))))
endef

define __GENERATE-DIR
  $(if $(filter $(1), $(REQUIRE-DIR-DONE)), , \
    $(eval $(call __GENERATE-DIR-TEMPLATE, $(1))))
endef

define GENERATE-DIR
  $(foreach elm, $(call NOSLASH, $(1)), \
    $(call __GENERATE-DIR, $(elm)))
endef

define __REQUIRE-DIR-TEMPLATE
  $(call CMD-INFO, $(strip $(1)): | $(strip $(call NOSLASH, $(dir $(1)))))
  $(1): | $(call NOSLASH, $(dir $(1)))

  $(call GENERATE-DIR, $(call NOSLASH, $(dir $(1))))
endef

define REQUIRE-DIR
  $(foreach elm, $(call NOSLASH, $(1)), \
    $(eval $(call __REQUIRE-DIR-TEMPLATE, $(elm))))
endef


define __AUTODEP-RULE
  $(call CMD-INFO, .depends: $(strip $(1)))
  $(call CMD-INFO, $(strip $(1)): $(strip $(2)) .generated)
  $(call CMD-INFO)

  .depends: $(1)

  $(1): $(2) .generated

  $(call REQUIRE-DIR, $(1))
endef

define REQUIRE-DEP
  $(foreach o, $(1), \
    $(eval $(call __AUTODEP-RULE, $(patsubst %, .depends.d/%.d, $(o)), $(o))))
endef
