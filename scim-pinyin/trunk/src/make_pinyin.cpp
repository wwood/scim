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
#define Uses_C_WCTYPE
#define Uses_SCIM_UTILITY

#include <scim.h>
#include "scim_pinyin_private.h"
#include "scim_pinyin.h"

static const char *help_msg =
	"Too few argument!\n"
	"Usage:\n"
	"  make_pinyin <pinyin_table> [options]\n\n"
	"  pinyin_table\tthe pinyin table file\n"
	"  -b\t\tuse binary format\n"
	"  -nt\t\tdo not use tone\n"
	"  -s file\tspecifiy the source file to count char ages.\n"
	"  -o file\toutput the result table to the file.\n"
	"  -c file\tcheck the multi-key chars' valid keys from the file.\n"
	"  -m file\toutput the chars with multiple keys to the file.\n";

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

void check_multi (PinyinTable *pyt, const char *file)
{
	std::ifstream ifs (file);

	if (!ifs) return;

	ucs4_t wc;

	String line;
	String mb;
	String keystr;
	std::vector <String> keylist;

	uint32 i;

	PinyinKeyVector valid_keys, all_keys;

	PinyinValidator validator;

	while (1) {
		line = _get_line (ifs);
		if (line.length () == 0) break;
		if (line [0] == '#') continue;

		mb = _get_param_portion (line);
		keystr = _get_value_portion (line);
		if (!mb.length () || !keystr.length ()) break;

		scim_split_string_list (keylist, keystr, ' ');

		valid_keys.clear ();

		for (i=0; i<keylist.size (); i++)
			valid_keys.push_back (PinyinKey (validator, keylist[i].c_str ()));

		utf8_mbtowc (&wc, (const unsigned char *) mb.c_str (), mb.length ());

		pyt->erase (wc, PinyinKey ());

		for (i=0; i<valid_keys.size (); ++i) {
			pyt->insert (wc, valid_keys [i]);
		}
		std::cout << "." << std::flush;
	}
	std::cout << "\n";
}

void save_multi (PinyinTable *pyt, const char *file)
{
	std::ofstream ofs(file);

	if (!ofs) return;

	std::vector<ucs4_t> chars;

	pyt->get_all_chars (chars);

	PinyinKeyVector keys;
	for (uint32 i=0; i<chars.size (); i++) {
		pyt->find_keys (keys, chars[i]);
		if (keys.size () > 1) {
			utf8_write_wchar (ofs, chars[i]);
			ofs<< " =";

			for (uint32 j=0; j<keys.size (); j++)
				ofs << " " << keys[j];

			ofs << "\n";
		}

	}
}

void self_learn (PinyinTable *pyt, const char *file)
{
	std::ifstream ifs(file);

	if (!ifs) return;

	ucs4_t wc;

	int mb = 0;
	int byte = 0;

	while (!ifs.eof()) {
		if ((wc = utf8_read_wchar (ifs)) <= 0) break;
		if (wc<0x3400 || wc>0x9fa5 ||
		    iswpunct (wc) || iswspace (wc) ||
		    iswdigit (wc) )
			continue;
		pyt->refresh (wc);
		byte ++;
		if (byte >= 1048576) {
			byte = 0;
			mb ++;
			std::cout << mb << " M WideChars\n";
		}
	}
}

int main (int argc, char * argv [])
{
	bool use_tone = true;
	bool binary = false;
	char *corpusfile = NULL;
	char *destfile = NULL;
	char *multikeyfile = NULL;
	char *checkkeyfile = NULL;

	PinyinTable *pinyin_table;
	PinyinValidator *validator;

	PinyinCustomSettings custom;

	if (argc < 2) {
		std::cerr << help_msg;
		return -1;
	}

	int i = 1;
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
		
		if (String ("-s") == argv [i]) {
			if (++i >= argc) {
				std::cerr << "No argument for option " << argv [i-1] << "\n";
				return -1;
			}
			corpusfile = argv [i];
			continue;
		}

		if (String ("-o") == argv [i]) {
			if (++i >= argc) {
				std::cerr << "No argument for option " << argv [i-1] << "\n";
				return -1;
			}
			destfile = argv [i];
			continue;
		}

		if (String ("-c") == argv [i]) {
			if (++i >= argc) {
				std::cerr << "No argument for option " << argv [i-1] << "\n";
				return -1;
			}
			checkkeyfile = argv [i];
			continue;
		}

		if (String ("-m") == argv [i]) {
			if (++i >= argc) {
				std::cerr << "No argument for option " << argv [i-1] << "\n";
				return -1;
			}
			multikeyfile = argv [i];
			continue;
		}

		std::cerr << "Invalid option: " << argv [i] << "\n";
		return -1;
	};

	if (!destfile) destfile = argv [1];

	custom.use_tone = use_tone;
	custom.use_incomplete = false;
	custom.use_dynamic_adjust = false;
	
	for (int i = 0; i<=SCIM_PINYIN_AmbLast; i++)
		custom.use_ambiguities [i] = false;

	validator = new PinyinValidator ();
	pinyin_table = new PinyinTable (custom, validator);

	if (!pinyin_table->load_table (argv [1])) {
		std::cerr << "pinyin table load failed!" << "\n";
		return -1;
	}

	validator->initialize (pinyin_table);

	if (corpusfile != NULL) {
		std::cout << "Adjusting char ages...\n";
		self_learn (pinyin_table, corpusfile);
	}

	if (checkkeyfile) {
		std::cout << "Checking chars with multiple keys!" << "\n";
		check_multi (pinyin_table, checkkeyfile);
	}

	if (multikeyfile) {
		std::cout << "Saving chars with multiple keys!" << "\n";
		save_multi (pinyin_table, multikeyfile);
	}

	std::cout << "Saving pinyin table!" << "\n";
	if (!pinyin_table->save_table (destfile, binary)) {
		std::cerr << "pinyin table save failed!" << "\n";
	}

	delete pinyin_table;
	delete validator;
	return 0;
}
