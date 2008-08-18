MAINTAINERCLEANFILES	= Makefile.in

scim_pinyin_datadir   = @SCIM_PINYIN_DATADIR@
scim_pinyin_data_DATA =  pinyin_table \
			 phrase_lib \
			 pinyin_phrase_lib \
			 pinyin_phrase_index \
			 special_table

icondir			= $(SCIM_ICONDIR)
icon_DATA		= smart-pinyin.png

EXTRA_DIST=$(scim_pinyin_data_DATA) \
	   pinyin_table.txt \
	   phrase_lib.txt \
	   pinyin_phrase_lib.txt \
	   pinyin_phrase_index.txt \
	   smart-pinyin.png

if ENABLE_TOOLS
pinyin_table: pinyin_table.txt
	$(top_srcdir)/src/make-pinyin $< -o $@ -b

phrase_lib: phrase_lib.txt
	$(top_srcdir)/src/make-phrase $< -o $@ -b

pinyin_phrase_index: pinyin_phrase_index.txt pinyin_phrase_lib.txt phrase_lib pinyin_table
	$(top_srcdir)/src/make-pinyin-phrase \
		pinyin_table \
		phrase_lib \
		pinyin_phrase_lib.txt \
		pinyin_phrase_index.txt \
		-b -oi $@

pinyin_phrase_lib: pinyin_phrase_lib.txt pinyin_phrase_index.txt phrase_lib pinyin_table
	$(top_srcdir)/src/make-pinyin-phrase \
		pinyin_table \
		phrase_lib \
		pinyin_phrase_lib.txt \
		pinyin_phrase_index.txt \
		-b -ol $@
endif
