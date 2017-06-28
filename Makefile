NAME      :=  libpropsmgr_$(shell uname -m)-$(shell uname -s)
BINDIR	  :=  bin
OBJDIR	  :=  obj
SRCDIR	  :=  src
TSTDIR    :=  tests
MYINC     :=  ../common/include
INCDIR	  :=  $(SRCDIR)/include
INCFLAGS  :=	-I$(INCDIR) -I$(MYINC)
SNAME     :=  $(BINDIR)/$(NAME).a
DNAME     :=  $(BINDIR)/$(NAME).so
SRC       :=  $(wildcard src/*.c)
SOBJ      :=  $(SRC:src/%.c=$(OBJDIR)/stat_%.o)
DOBJ      :=  $(SRC:src/%.c=$(OBJDIR)/dyn_%.o)
ARFLAGS	  :=  rcs
CUSFLAGS  :=
CFLAGS    :=  -ansi -pedantic -Wall -Wextra -g3 -std=c99 $(CUSFLAGS)
LDFLAGS   :=  -L.
LDLIBS    :=  

.PHONY: all clean mrproper

all: tests static shared

test: $(BINDIR)/test

$(BINDIR)/test:$(SOBJ)
	$(CC) $^ -o $@ 

static: $(SNAME)

$(SNAME): $(SOBJ)
	$(AR) $(ARFLAGS) $@ $^

shared: $(DNAME)

$(DNAME): LDFLAGS += -shared
$(DNAME): $(DOBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJDIR)/dyn_%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCFLAGS)
	
$(OBJDIR)/stat_%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCFLAGS)

clean:
	$(RM) $(DOBJ) $(SOBJ)

mrproper: clean
	$(RM) $(SNAME) $(DNAME) $(BINDIR)/test
