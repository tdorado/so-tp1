#MakeFile para compilar o debugear los archivos

#todos los archivos binarios van a tener extensión .so

#arhivos que tienen un main
TARGETS = view application slave
#arhivos que no tienen un main
SOURCES = utilities/sources/queue utilities/sources/utils slave_lib application_lib view_lib
#testing
TASTEFUL = tasteful
TESTS = Tests/queueTest Tests/test1 Tests/slave_test Tests/application_test Tests/view_test Tests/app_v_integration_test
TEST_SOURCES = utilities/sources/queue Testing/testing_suite utilities/sources/utils slave_lib application_lib view_lib

CROSS_TOOL =
CC_C = $(CROSS_TOOL) gcc

CFLAGS = -Wall -g -pthread -std=gnu99
CPPFLAGS = --quiet --enable=all --force --inconclusive --suppress=missingIncludeSystem --suppress=unusedFunction --inline-suppr

all: clean $(TARGETS)

$(TARGETS):
	$(CC_C) $(CFLAGS) $(SOURCES:=.c) $@.c -o $@.so -lrt

clean:
	@rm -f $(TARGETS) $(TARGETS:=.o) $(TARGETS:=.so)
	@rm -f $(SOURCES) $(SOURCES:=.o)
	@rm -f Tests/$(TASTEFUL) Tests/$(TASTEFUL:=.o) Tests/$(TASTEFUL:=.so)
	@rm -f $(TESTS) $(TESTS:=.o) $(TESTS:=.so)
	@rm -f $(TEST_SOURCES) $(TEST_SOURCES:=.o)
	@rm -f result.txt

debug: all #primero limpiar y compilar todo
	#----------------------------------checkeos cppcheck
	@cppcheck $(CPPFLAGS) $(TARGETS:=.c)
	@cppcheck $(CPPFLAGS) $(SOURCES:=.c)
	@cppcheck $(CPPFLAGS) $(TESTS:=.c)
	@cppcheck $(CPPFLAGS) $(TEST_SOURCES:=.c)
	#----------------------------------checkeos de los binarios con valgrind
	$(foreach f, $(TARGETS:=.so), valgrind ./$f)
	$(foreach f, $(TESTS:=.so), valgrind ./$f)

test: all $(TESTS) #correr todas las suites de test
	@$(CC_C) $(CFLAGS) Testing/$(TASTEFUL).c -o Tests/$(TASTEFUL).so
	@cd Tests && ./$(TASTEFUL).so
	
$(TESTS):
	@$(CC_C) $(CFLAGS) $(TEST_SOURCES:=.c) $@.c -o $@.so -lrt
	
.PHONY: all, debug, test

	