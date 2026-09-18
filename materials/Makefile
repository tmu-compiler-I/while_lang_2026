SHELL := /bin/sh

.NOTPARALLEL:

TMP_BASE := $(if $(TMPDIR),$(TMPDIR),tmp)
TEXMFCACHE := $(TMP_BASE)/while-lang-tex-cache
CACHE_READY := $(TEXMFCACHE)/.initialized
BUILDDIR := $(TMP_BASE)
OUTDIR := $(abspath ../output/pdf)
LATEX := env TEXMFCACHE="$(TEXMFCACHE)" lualatex -interaction=nonstopmode -halt-on-error -output-directory="$(BUILDDIR)"

.PHONY: all slides handout prepare clean

all: slides handout

prepare: $(CACHE_READY)
	@mkdir -p "$(BUILDDIR)" "$(OUTDIR)"

$(CACHE_READY):
	@mkdir -p "$(TEXMFCACHE)"
	@echo "Initializing the writable LuaTeX font cache"
	env TEXMFCACHE="$(TEXMFCACHE)" luaotfload-tool --update
	@touch "$@"

slides: prepare
	@echo "Building lecture_slides.pdf"
	$(LATEX) lecture_slides.tex
	$(LATEX) lecture_slides.tex
	cp "$(BUILDDIR)/lecture_slides.pdf" "$(OUTDIR)/lecture_slides.pdf"

handout: prepare
	@echo "Building lecture_handout.pdf"
	$(LATEX) lecture_handout.tex
	$(LATEX) lecture_handout.tex
	cp "$(BUILDDIR)/lecture_handout.pdf" "$(OUTDIR)/lecture_handout.pdf"

clean:
	env TEXMFCACHE="$(TEXMFCACHE)" latexmk -c -outdir="$(BUILDDIR)" lecture_slides.tex
	env TEXMFCACHE="$(TEXMFCACHE)" latexmk -c -outdir="$(BUILDDIR)" lecture_handout.tex
