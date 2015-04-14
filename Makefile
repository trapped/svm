.PHONY: all svm clean

all: svm

svm:
	@make -C svm $@

clean:
	@make -C svm $@

