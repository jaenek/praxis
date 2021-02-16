FILES = $(patsubst entries/%.md, public/%.html, $(shell find entries -type f -name *.md))
INDEXES = $(patsubst entries/%, public/%/index.html, $(wildcard entries/*))

.PHONY: all clean

all: $(FILES) $(INDEXES) public/index.html

clean:
	find public/* -maxdepth 0 -type d -exec rm -rfv {} +
	rm -fv public/index.html

$(FILES) : public/%.html : entries/%.md
	mkdir -pv $(shell dirname $@)
	{ ./generate_nav.sh entries; echo '<main>'; md2html --github $^; echo '</main>'; } | ./template.sh > $@

$(INDEXES): %index.html : %
	 { ./generate_nav.sh entries; ./generate_index.sh $^; } | ./template.sh > $@

public/index.html: entries
	{ ./generate_nav.sh $^; ./generate_latest.sh $^; } | ./template.sh > $@

