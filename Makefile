SUBDIR = ./source
install:
	cd $(SUBDIR) && $(MAKE)
	make install -C $(SUBDIR)
clean:
	make clean -C $(SUBDIR)