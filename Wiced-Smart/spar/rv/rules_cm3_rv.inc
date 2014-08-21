################################################################################
# CM3 specific spar rules
################################################################################
# DEpendencies are made automatically when obj is created
$(BIN_OUT_DIR)/%.o: $(BIN_OUT_DIR)/%.d

.INTERMEDIATE: $(SPAR_SETUP_CALL_CGS) $(ELF_OUT_SYM) $(ELF_OUT_BIN) $(OVERLAY_SCAT) $(BIN_OUT_DIR)/*.ocgs

# The list file is really a side effect of building the elf
$(ELF_OUT_LIST): $(ELF_OUT)


$(SPAR_SETUP_CALL_CGS): $(ELF_OUT_LIST) $(ELF_OUT:.elf=.hex)
	$(QUIET)-$(XMD) $(@D)
	$(QUIET)spar_crt_setup_addr=`\
	       $(GREP) --perl-regexp --max-count=1 \
	         '^[[:space:]]+$(SPAR_CRT_SETUP)[[:space:]]+[[:alnum:]]+[[:space:]]+Thumb[[:space:]]Code[[:space:]]+[[:alnum:]]+[[:space:]]+.*\.o\(spar_setup\)' $< | \
	       $(AWK) '{print $$2}'` ; \
	  $(ECHO) 'Call to $(SPAR_CRT_SETUP) @' $$spar_crt_setup_addr ; \
	( $(ECHO) '                                               ' ; \
	  $(ECHO) '# SPAR: call to  $(SPAR_CRT_SETUP)         	  ' ; \
	  $(ECHO) '# SPAR: (address from $<)                      ' ; \
	  $(ECHO) 'ENTRY "Function Call"                          ' ; \
	  $(ECHO) '{                                              ' ; \
	  $(ECHO) '   "Address" =' $$spar_crt_setup_addr            ; \
	  $(ECHO) '}                                              ' ; \
	  $(ECHO) '                                               ' ; \
	    ) > $@

# elf -> hex, in case missing; covering tier1, tier2, and any SPARs
%.hex: %.elf
	$(QUIET)-$(XMD) $(@D)
	$(QUIET)if [ ! -f $@ ]; then \
		$(HEXCOPY) $< --output $@; \
	fi

ELF_OUT_SYM = $(ELF_OUT).sym 
ELF_OUT_BIN = $(ELF_OUT).bin

ifeq ($(DIRECT_LOAD),y)
  ENTRY = 'DIRECT_LOAD'
else
  ENTRY = 'ENTRY \"Data\"'
endif

$(ELF_OUT_SYM): $(ELF_OUT)
	$(QUIET)$(FROMELF) $(FROMELF_SYMTABOPTS) $< -o $@

$(ELF_OUT_BIN): $(ELF_OUT)
	$(QUIET)$(FROMELF) --bin -c -d $< -o $@

# From spar.elf.xyz.hex, make spar.elf.xyz.cgs, or a comment only
# if empty (man test, look for -s). $(subst .elf,,$(suffix $*))
$(ELF_OUT)%.cgs: $(ELF_OUT)
	$(QUIET)-$(XMD) $(@D)
	$(PERL) $(SECTION_TO_CGS) -e $< -o $(FROMELF) -s $* -t rv > $@

# Extract the compressed section binary as a stream of hex bytes. fromelf is pretty stupid and does not allow
# you to extract the binary image in a section. So, parse the section header and get its offset/length in
# the elf and  dump length bytes at offset as a hex byte stream
$(ELF_OUT)%.ccgs: $(ELF_OUT)
	$(QUIET)-$(XMD) $(@D)
	$(QUIET)$(PERL) $(SECTION_TO_CGS) -e $< -o $(FROMELF) -s $* -c $(COMPRESSOR) -t rv > $@

# Extract the overlay section binary as a stream of hex bytes. fromelf is pretty stupid and does not allow
# you to extract the binary image in a section. So, parse the section header and get its offset/length in
# the elf and use od to dump length bytes at offset as a hex byte stream read 1 byte at a time - just the
# format Bluetool expects commented hex bytes to be in.
$(ELF_OUT)%.ocgs: $(ELF_OUT_LIST)
	$(QUIET)-$(XMD) $(@D)
	  $(QUIET)section_start=`\
	      $(GREP) --perl-regexp --max-count=1 \
	              "^[[:space:]]+Execution[[:space:]]Region[[:space:]]$*[[:space:]]\(Exec[[:space:]]base\:[[:space:]]0x" $< | \
			  $(AWK) '{print $$6}' | $(AWK) '{str1=str1 $$0 ""}END{print str1}' | $(SED) -e 's/,//'` ; \
	  section_size=`\
		   $(GREP) --perl-regexp --max-count=1 \
	              "^[[:space:]]+Execution[[:space:]]Region[[:space:]]$*[[:space:]]\(Exec[[:space:]]base\:[[:space:]]0x" $< | \
			  $(AWK) '{print $$9}' | $(AWK) '{str1=str1 $$0 ""}END{print str1}' | $(SED) -e 's/,//'` ; \
	  if [[ "$$section_size" = "0x00000000" ]]; then \
			$(ECHO) 'Section $* is empty. Skipping.' ; \
			( $(ECHO) '                                   ' ; \
			$(ECHO) '# Section $* is empty. Skipping. ' ) > $@; \
	  else \
	    overlay_offset=`\
	      $(FROMELF) -v --only $* $(ELF_OUT) | $(SED) -n -e '/$*/,$$p' | \
		     $(GREP) --max-count=1 --perl-regexp  "^[[:space:]]+File[[:space:]]Offset(.*)+(\d)+[[:space:]]+\(0x([0-9a-fA-F]+\))" | \
			 $(AWK) '{print $$4;}'` ; \
	    overlay_length=`\
	       $(FROMELF) -v --only $* $(ELF_OUT) | $(SED) -n -e '/$*/,$$p' | \
		      $(GREP) --max-count=1 --perl-regexp  "^[[:space:]]+Size[[:space:]]+(.*)+(\d)+[[:space:]]+bytes[[:space:]]+\(0x([0-9a-fA-F]+\))" | \
			  $(AWK) '{print $$3;}'` ; \
	    overlay_index=`$(ECHO) $* | $(CUT) -d'_' -f4`; \
	    overlay_bytes=`\
	       $(OD) -j $$overlay_offset -N $$overlay_length -t x1 $(ELF_OUT) | $(CUT) -d' ' -f2- | $(HEAD) -n -1 | $(PERL) -e 'while(<>){chomp;print "$$_\n\t\t\t";}'`; \
	    $(ECHO) "Making $@, start at $$section_start ..."    ; \
	    ( $(ECHO) '                                    ' ; \
	      $(ECHO) '# SPAR: $* from $(ELF_OUT)         '  ; \
	      $(ECHO) 'ENTRY "Overlay data" = "$* from $(ELF_OUT)" ' ; \
	      $(ECHO) '{                                   ' ; \
		  $(ECHO) '  "Overlay ID" = ' $$overlay_index    ; \
	      $(ECHO) '  "Address" =' $$section_start        ; \
	      $(ECHO) '  "Data" =                          ' ; \
	      $(ECHO) '   COMMENTED_BYTES                  ' ; \
	      $(ECHO) '     {                              ' ; \
	      $(ECHO) '        <hex>                       ' ; \
	      $(ECHO) "            $$overlay_bytes"        ; \
	      $(ECHO) '     } END_COMMENTED_BYTES          ' ; \
	      $(ECHO) '}                                   ' ; \
	      $(ECHO) '# SPAR: end of $* ' \
	    ) > $@ ; \
	  fi

# If we have overlays enabled, we need to place them somewhere.	
OVERLAY_SCAT = $(BIN_OUT_DIR)/spar_ram_overlays.scat

# Make sure that the overlaid objects are always in a known order. All the autogenerated code
# depends on the order being known at compile time. So, sort the overlayd sources into objs.
# We don't quite care what order, as long as it is deterministic.
OVERLAY_OBJS = $(addsuffix .o,$(sort $(basename $(OVERLAY_SRC))))

ifneq ($(OVERLAY_SRC),)
$(BIN_OUT_DIR)/trampoline_%.s: $(BIN_OUT_DIR)/%.o
	$(QUIET)$(ECHO) "Generating trampoline functions for overlaid areas for $@..."
	$(QUIET)functions=`\
	    $(FROMELF) --text --info=function_sizes $< | $(GREP) --perl-regexp  "[[:space:]](.*)[[:digit:]]+[[:space:]]Thumb" | \
		$(AWK) '(NF>0){print $$1;}' | $(AWK) '{printf "%s ", $$0;}'`;\
	$(PERL) $(GEN_OVERLAY_FILES) -gen=tramp -tc=rv -file=$< $$functions > $@

OBJS += $(addsuffix .o, $(addprefix $(BIN_OUT_DIR)/trampoline_, $(sort $(basename $(OVERLAY_SRC)))))
endif

# Autogenerate the scatter file for the overlays
ifneq ($(OVERLAY_SRC),)
$(OVERLAY_SCAT):
	$(QUIET)$(PERL) $(GEN_OVERLAY_FILES) -gen=scat -tc=rv $(OVERLAY_OBJS) > $@
else
$(OVERLAY_SCAT):
	$(QUIET)$(ECHO) ' ' > $@
endif

# Always autogenerate the library installers
OBJS += $(BIN_OUT_DIR)/lib_installer.o

$(BIN_OUT_DIR)/lib_installer.c: $(LIBS)  $(filter-out $(BIN_OUT_DIR)/lib_installer.o,$(OBJS))
	$(QUIET)$(PERL) $(GEN_LIB_INSTALLER) -tc=rv -tool=$(FROMELF) $^ > $@

# Force pre-processing the scatter-loader file - Getting linker to do this
# instead does not always work in Cygwin. Needs to be generated everytime before link
$(SPAR_LINK_LOAD_FILE): $(TC)/spar_$(SPAR_IN).scat $(ELF_LIST) $(OBJS) $(OVERLAY_SCAT) FORCE
	$(QUIET)$(XCC) -E -I. -I$(BIN_OUT_DIR)/ $(addprefix -D,$(LINK_LOC_FLAGS)) $(addsuffix ",$(addprefix -D",$(OVERLAY_FLAGS))) -o $@ $<

.FORCE:


.PHONY: print_stats
print_stats: 
	$(QUIET)$(ECHO) '--------------------------------------------------------------------------------'
	$(QUIET)tier2_start_addr=`\
		    $(GREP) --perl-regexp --max-count=1 "^[[:space:]]+Load[[:space:]]Region[[:space:]]CM3_Ver1[[:space:]].*Base.*[[:space:]]0x[[:alnum:]].*[[:space:]]Size.*[[:space:]]0x[[:alnum:]]" $(LINK_ELFS:.symdefs=.lst) | \
					$(AWK) '{str1=str1 $$0 ""}END{print str1};' | $(AWK) --non-decimal-data '{printf "0x%08X", ($$5)};'`;\
	  $(ECHO) 'Patches start at (addr): ' $$tier2_start_addr ; \
	tier2_end_adr=`\
		    $(GREP) --perl-regexp --max-count=1 "^[[:space:]]+Load[[:space:]]Region[[:space:]]CM3_Ver1[[:space:]].*Base.*[[:space:]]0x[[:alnum:]].*[[:space:]]Size.*[[:space:]]0x[[:alnum:]]" $(LINK_ELFS:.symdefs=.lst) | \
					$(AWK) '{str1=str1 $$0 ""}END{print str1};' | $(AWK) --non-decimal-data '{printf "0x%08X", ($$5 + $$7)};'`;\
	  $(ECHO) 'Patches end at (adr): ' $$tier2_end_adr ;\
	spar_start_adr=`\
	        $(GREP) --perl-regexp --max-count=1 "^[[:space:]]+Load[[:space:]]Region[[:space:]]CM3_Ver1[[:space:]].*Base.*[[:space:]]0x[[:alnum:]].*[[:space:]]Size.*[[:space:]]0x[[:alnum:]]" $(ELF_OUT_LIST) | \
					$(AWK) '{str1=str1 $$0 ""}END{print str1};' | $(AWK) --non-decimal-data '{printf "0x%08X", ($$5)};'`;\
	  $(ECHO) 'Application starts at (addr): ' $$spar_start_adr ; \
	spar_end_adr=`\
	        $(GREP) --perl-regexp --max-count=1 "^[[:space:]]+Load[[:space:]]Region[[:space:]]CM3_Ver1[[:space:]].*Base.*[[:space:]]0x[[:alnum:]].*[[:space:]]Size.*[[:space:]]0x[[:alnum:]]" $(ELF_OUT_LIST) | \
					$(AWK) '{str1=str1 $$0 ""}END{print str1};' | $(AWK) --non-decimal-data '{printf "0x%08X", ($$5 + $$7)};'`;\
	  $(ECHO) 'Application ends at (addr): ' $$spar_end_adr ; \
	  $(ECHO) $$tier2_end_adr $$tier2_start_addr | $(AWK) --non-decimal-data 'END{printf "Patch code + ro + rw (bytes): 0x%08X = %2.2fK", ($$1 - $$2), ($$1 - $$2) / 1024};' ;\
	  $(ECHO) '                                          ' ;\
	  $(ECHO) $$spar_start_adr $$tier2_start_addr |  $(AWK) --non-decimal-data 'END{printf "Patch size (code + ro + rw + zi - reused RAM) (bytes): 0x%08X = %2.2fK", ($$1 - $$2), ($$1 - $$2) / 1024};' ;\
	  $(ECHO) '                                          ' ;\
	  $(ECHO) $$spar_end_adr $$spar_start_adr |  $(AWK) --non-decimal-data 'END{printf "Application size (bytes): 0x%08X = %2.2fK", ($$1 - $$2), ($$1 - $$2) / 1024};' ;\
	  $(ECHO) '                                          ' ;\
	  $(ECHO) '                                          ' ;\
	  $(ECHO) $$spar_end_adr $$tier2_start_addr |  $(AWK) --non-decimal-data 'END{printf "Total RAM footprint: 0x%08X bytes = %02.2fK", ($$1 - $$2), ($$1 - $$2) / 1024};' ;\
	  $(ECHO) '                                          ' ;\
	  if [ -e $(LINK_ELFS:.symdefs=.stat) ];  then \
	    $(ECHO) 'Patchset used: ' ;\
	    $(CAT) $(LINK_ELFS:.symdefs=.stat) ;\
	  fi ;\
	  $(ECHO) '                              ' ; \
	  $(ECHO) '--------------------------------------------------------------------------------'
