#define Uses_STL_AUTOPTR
#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO
#define Uses_SCIM_UTILITY

#include <scim.h>
#include "scim_pinyin_private.h"
#include "scim_pinyin.h"
#include "scim_phrase.h"
#include "scim_pinyin_phrase.h"

typedef std::multimap <WideString, PinyinKeyVector> PPValidateMap;
typedef std::pair <WideString, PinyinKeyVector> PPValidatePair;

static PPValidateMap pp_validate_map;

static PinyinCustomSettings custom;
static PinyinValidator *validator;


static inline String
_trim_blank (const String &str)
{
	unsigned int begin, len;

	begin = str.find_first_not_of (" \t\n\v");

	if (begin == String::npos)
		return String ();

	len = str.find_last_not_of (" \t\n\v") - begin + 1;

	return str.substr (begin, len);
}

static inline String
_get_param_portion (const String &str, const String &delim = "=")
{
	String ret = str;
	unsigned int pos = ret.find_first_of (String (" \t\v") + delim);

	if (pos != String::npos)
		ret.erase (pos, ret.length() - 1);

	return ret;
}

static inline String
_get_value_portion (const String &str, const String &delim = "=")
{
	String ret = str;
	unsigned int pos;

	pos = ret.find_first_of (delim);

	if (pos != String::npos)
		ret.erase (0, pos + 1);

	pos = ret.find_first_not_of(" \t\v");

	if (pos != String::npos)
		ret.erase (0, pos);

	pos = ret.find_last_not_of(" \t\v");

	if (pos != String::npos)
		ret.erase (pos + 1);

	return ret;
}

static inline String  
_get_line (std::istream &is)
{
	char temp [1024];
	String res;

	while (1) {
		is.getline (temp, 1023);
		res = _trim_blank (String (temp));

		if (res.length () > 0 && res [0] != '#') return res;
		if (is.eof ()) return String ();
	}
}

void load_validate_file (const char *file)
{
	std::ifstream ifs (file);
	if (ifs) {
		String line;
		String phrase;
		String keys;
		std::vector<String> keysv;
		PinyinKeyVector kv;
		WideString wcp;
		while (1) {
			kv.clear ();
			line = _get_line (ifs);
			if (line.length ()) {
				phrase = _get_param_portion (line, "=");
				keys = _get_value_portion (line, "=");
				scim_split_string_list (keysv, keys, ' ');

				for (int i=0; i<keysv.size (); i++) {
					kv.push_back (PinyinKey (*validator, keysv[i].c_str ()));
				}

				wcp = utf8_mbstowcs (phrase);
				pp_validate_map.insert (PPValidatePair (wcp, kv));
				std::cout << ".";
			} else {
				break;
			}
		}
	}
	std::cout << "\n";
}

bool pinyin_phrase_validator (const PinyinPhrase &phrase)
{
	WideString wcp = phrase.get_phrase ().get_content ();

	std::pair <PPValidateMap::iterator, PPValidateMap::iterator> result
		= pp_validate_map.equal_range (wcp);

	if (result.first == result.second) return true;

	PinyinKeyEqualTo eq (custom);

	for (PPValidateMap::iterator it = result.first; it != result.second; it++) {
		uint32 i;
		for (i=0; i<phrase.length (); i++) {
			if (! eq (phrase.get_key (i), (it->second) [i]))
				break;
		}
		if (i == phrase.length ())
			return true;
	}
	std::cerr << "D " << utf8_wcstombs (wcp) << " =";
	for (int i=0; i<phrase.length (); i++)
		std::cerr << ' ' << phrase.get_key (i);
	std::cerr << "\n";
	return false;
}

void show_mem ()
{
	char buff [255];

	std::ifstream ifs("/proc/self/status");

	while (!ifs.eof()) {
		ifs.getline (buff,254);
		std::cout << buff << "\n";
	}
}

int main (int argc, char * argv [])
{
	bool binary = false;
	bool refine = false;
	bool use_tone = true;

	const char *pplib_file = NULL;
	const char *ppindex_file = NULL;
	const char *validate_file = NULL;
	const char *dump_file = NULL;

	PinyinPhraseLib  *pinyin_phrase_lib;
	PinyinTable *pinyin_table;


	if (argc < 5) {
		std::cerr    << "Too few argument!\n"
			<< "Usage:\n"
			<< "  make_pinyin_phrase <pinyin_table> <phrase_lib> <pinyin_phrase_lib> <pinyin_phrase_index> [options]\n\n"
			<< "  pinyin_table\t\tthe pinyin table file (to be used to generate pinyin_phrase_lib)\n"
			<< "  phrase_lib\t\tthe phrase library file\n"
			<< "  pinyin_phrase_lib\tthe pinyin key library for phrase_lib\n"
			<< "  pinyin_phrase_index\tthe index file\n"
			<< "  -ol\tfile\tsave new pinyin_phrase_lib to file.\n"
			<< "  -oi\tfile\tsave new pinyin_phrase_index to file.\n"
			<< "  -b\t\t\tuse binary format\n"
			<< "  -r\tfile\trefine the pinyin phrase library, using validate file.\n"
			<< "  -nt\t\t\tdo not use tone\n"
			<< "  -d\tfile\tdump all phrases with pinyin to file.\n";
		return -1;
	}

	int i = 4;
	while (i<argc) {
		if (++i >= argc) break;

		if (String ("-b") == argv [i]) {
			binary = true;
			continue;
		}

		if (String ("-nt") == argv [i]) {
			use_tone = false;
			continue;
		}
		
		if (String ("-r") == argv [i]) {
			refine = true;
			if ( (i+1) >= argc || argv [i+1][0] == '-')
				continue;
			validate_file = argv [++i];
			continue;
		}

		if (String ("-ol") == argv [i]) {
			if (++i >= argc) {
				std::cerr << "No argument for option " << argv [i-1] << "\n";
				return -1;
			}
			pplib_file= argv [i];
			continue;
		}

		if (String ("-oi") == argv [i]) {
			if (++i >= argc) {
				std::cerr << "No argument for option " << argv [i-1] << "\n";
				return -1;
			}
			ppindex_file= argv [i];
			continue;
		}

		if (String ("-d") == argv [i]) {
			if (++i >= argc) {
				std::cerr << "No argument for option " << argv [i-1] << "\n";
				return -1;
			}
			dump_file = argv [i];
			continue;
		}

		std::cerr << "Invalid option: " << argv [i] << "\n";
		return -1;
	};

	custom.use_tone = use_tone;
	custom.use_incomplete = false;
	custom.use_dynamic_adjust = false;

	for (int i = 0; i<=SCIM_PINYIN_AmbLast; i++)
		custom.use_ambiguities [i] = false;

	validator = new PinyinValidator ();
	pinyin_table = new PinyinTable (custom, validator);

	std::cout << "Memory info before loading pinyin table:\n";
	show_mem ();

	if (!pinyin_table->load_table (argv [1])) {
		std::cerr << "Pinyin table load failed!" << "\n";
		return -1;
	}

	validator->initialize (pinyin_table);

	pinyin_phrase_lib = new PinyinPhraseLib (custom, validator, pinyin_table);

	std::cout << "Memory info before loading pinyin phrase lib:\n";
	show_mem ();

	if (!pinyin_phrase_lib->load_lib (argv [2], argv [3], argv [4])) {
		std::cerr << "Pinyin phrase library load failed!" << "\n";
		return -1;
	}

	std::cout << "Memory info after loading pinyin phrase lib:\n";
	show_mem ();

	if (validate_file) {
		std::cout << "Loading validate file ...\n";
		load_validate_file (validate_file);
	}

	if (refine) {
		std::cout << "Refining pinyin phrase library...\n";
		if (validate_file)
			pinyin_phrase_lib->refine_library (pinyin_phrase_validator);
		else
			pinyin_phrase_lib->refine_library (NULL);
	}

	if (dump_file) {
		std::ofstream ofs (dump_file);
		if (ofs) pinyin_phrase_lib->dump_content (ofs);
	}

	if (!pinyin_phrase_lib->save_lib (NULL, pplib_file, ppindex_file, binary)) {
		std::cerr << "Pinyin phrase library save failed!\n";
	}
	

	delete pinyin_table;
	delete pinyin_phrase_lib;
	delete validator;
	return 0;
}
