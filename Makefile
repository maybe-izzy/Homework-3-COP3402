# $Id: Makefile,v 1.15 2023/03/17 03:00:59 leavens Exp leavens $
# Makefile for lexer in COP 3402

# Add .exe to the end of target to get that suffix in the rules
COMPILER = compiler
VM = vm
CC = gcc
CFLAGS = -g -std=c17 -Wall
RM = rm -f
SUBMISSIONZIPFILE = submission.zip
ZIP = zip -9
SOURCESLIST = sources.txt
TESTFILES = hw3-asttest*.pl0 hw3-parseerrtest*.pl0 hw3-declerrtest*.pl0
EXPECTEDOUTPUTS = `echo "$(TESTFILES)" | sed -e 's/\\.pl0/.out/g'`

$(COMPILER): *.c *.h
	$(CC) $(CFLAGS) -o $(COMPILER) `cat $(SOURCESLIST)`

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) *~ *.o *.myo '#'*
	$(RM) $(COMPILER).exe $(COMPILER)
	$(RM) *.stackdump core
	$(RM) $(SUBMISSIONZIPFILE)

.PRECIOUS: %.myo
%.myo: %.pl0 $(COMPILER)
	./$(COMPILER) $< > $@ 2>&1

check-outputs: $(COMPILER) hw3-*test*.pl0
	DIFFS=0; \
	for f in `echo $(TESTFILES) | sed -e 's/\\.pl0//g'`; \
	do \
		echo running "$$f.pl0"; \
		./$(COMPILER) "$$f.pl0" >"$$f.myo" 2>&1; \
		diff -w -B "$$f.out" "$$f.myo" && echo 'passed!' || DIFFS=1; \
	done; \
	if test 0 = $$DIFFS; \
	then \
		echo 'All tests passed!'; \
	else \
		echo 'Test(s) failed!'; \
	fi

$(SUBMISSIONZIPFILE): $(SOURCESLIST) *.c *.h *.myo
	$(ZIP) $(SUBMISSIONZIPFILE) $(SOURCESLIST) *.c *.h *.myo

.PRECIOUS: $(SOURCESLIST)
	echo *.c > $(SOURCESLIST)

# developer's section below...

.PRECIOUS: %.out
%.out: %.pl0 $(COMPILER)
	./$(COMPILER) $< > $@ 2>&1

.PHONY: create-outputs
create-outputs: $(COMPILER) hw3-*test*.pl0
	@echo 'Students should use the target check-outputs,'
	@echo 'as using this target (create-outputs) will invalidate the tests'
	@if test '$(IMTHEINSTRUCTOR)' != true ; \
	then \
		exit 1; \
	fi; \
	for f in `echo $(TESTFILES) | sed -e 's/\\.pl0//g'`; \
	do \
		echo running "$$f.pl0"; \
		$(RM) "$$f.out"; \
		./$(COMPILER) "$$f.pl0" >"$$f.out" 2>&1; \
	done; \
	echo done creating test outputs!

check-good-outputs: create-outputs
	DIFFS=0; \
	for f in `echo hw3-asttest*.pl0 | sed -e 's/\\.pl0//g'`; \
	do \
		./$(COMPILER) $$f.out >"$$f.myo" 2>&1; \
		diff -w -B "$$f.out" "$$f.myo" && echo 'passed!' || DIFFS=1; \
	done; \
	if test 0 = $$DIFFS; \
	then \
		echo 'All tests passed!'; \
	else \
		echo 'Test(s) failed!'; \
	fi

digest.txt: 
	for f in `ls $(TESTFILES) | sed -e 's/\\.pl0//g'`; \
        do cat $$f.pl0; echo " "; cat $$f.out; echo " "; echo " "; \
        done >digest.txt

# don't use develop-clean unless you want to regenerate the expected outputs
.PHONY: develop-clean
develop-clean: clean
	$(RM) *.out digest.txt

TESTSZIPFILE = ~/WWW/COP3402/homeworks/hw3-tests.zip
PROVIDEDFILES = token.[ch] lexer_output.[ch] utilities.[ch] lexer.h \
		ast.[ch] file_location.[ch] id_attrs.[ch] unparser.[ch] unparserInternal.h

hw3-tests.zip: create-outputs $(TESTSZIPFILE)

$(TESTSZIPFILE): $(TESTFILES) Makefile $(PROVIDEDFILES)
	$(RM) $(TESTSZIPFILE)
	chmod 444 $(TESTFILES) $(EXPECTEDOUTPUTS) Makefile $(PROVIDEDFILES)
	$(ZIP) $(TESTSZIPFILE) $(TESTFILES) $(EXPECTEDOUTPUTS) Makefile $(PROVIDEDFILES)
