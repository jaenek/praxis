FILES = $(patsubst entries/%.md, public/%.shtml, $(shell find entries -type f -name *.md))
INDEXES = $(patsubst entries/%, public/%/index.shtml, $(wildcard entries/*))

.PHONY: all clean

all: $(FILES) $(INDEXES) public/index.shtml public/navigation.html

clean:
	find public/* -maxdepth 0 -type d -exec rm -rfv {} +
	rm -fv public/index.shtml public/navigation.html

$(FILES) : public/%.shtml : entries/%.md
	mkdir -pv $(shell dirname $@)
	{ echo '<main>'; md2html --github $^; echo '</main>'; } | ./template.sh > $@


$(INDEXES): %index.shtml : %
	 ./generate_index.sh $^ | ./template.sh > $@

public/navigation.html: entries
	./generate_nav.sh $^ > $@

public/index.shtml: entries
	./generate_latest.sh $^ | ./template.sh > $@

