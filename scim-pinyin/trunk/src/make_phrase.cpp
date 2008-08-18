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
#define Uses_C_WCTYPE

#include <scim.h>
#include <time.h>
#include "scim_pinyin_private.h"
#include "scim_phrase.h"
static const char *help_msg = 
	"Too few argument!\n"
	"Usage:\n"
	"  make_phrase <phrase_lib> [options]\n\n"
	"  phrase_lib\tthe phrase library file\n"
	"  -b\t\tuse binary format\n"
	"  -r\t\trefine phrase library\n"
	"  -z\t\tzero all phrases' frequencies\n"
	"  -s file\tspecifiy the source file to count phrase ages.\n"
	"  -o file\toutput result phrase library to this file.\n"
	"  -a shift\tadjust all phrases' frequency by shift.\n";

void self_learn (PhraseLib *lib, const char *file)
{
	std::vector <ucs4_t> buffer;
	std::ifstream ifs(file);

	if (!ifs) return;

	uint32 kb = 0;
	uint32 byte = 0;

	WideString str;
	Phrase phrase;

	uint32 maxlen = lib->get_max_phrase_length ();

	ucs4_t wc;
	bool skip;
	char wheel [] = {'-', '\\', '|', '/', 0};
	int wheel_state;

//	buffer.reserve (1048576*32);

	skip = false;
	wheel_state = 0;

	while (!ifs.eof()) {
		buffer.clear ();
		// Read a line
		while (!ifs.eof ()) {
			if ((wc = utf8_read_wchar (ifs)) == 0) break;
			if (wc == L'\n') break;
			else if (wc<0x3400 || wc>0x9fa5 ||
			    iswpunct (wc) || iswspace (wc) || iswdigit (wc) ) {
				if (!skip) {
					buffer.push_back (0);
					skip = true;
				}
			} else {
				buffer.push_back (wc);
				skip = false;
			}
		}

		buffer.push_back (0);
		for (unsigned int i=0; i<buffer.size (); i++) {
			str = WideString ();
			for (unsigned int j=0; j<maxlen; j++) {
				if (buffer [j+i] == 0)
					break;
				str.push_back (buffer [j+i]);
				phrase = lib->find (str);
				if (phrase.valid ()) {
					phrase.refresh (31);
				}
			}
			byte ++;
			if (byte == 1024) {
				byte = 0;
				kb ++;
				std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
				std::cout << kb << "\tK ("
				     << wheel [wheel_state/2] << ") " << std::flush;
				wheel_state = (wheel_state+1) % 8;
			}
		}	
	}

	std::cout << "\n";
}

int main (int argc, char * argv [])
{
	bool binary = false;
	bool refine = false;
	bool zerofreq = false;

	char *corpusfile = NULL;
	char *destfile = NULL;

	int shift=0;

	PhraseLib  *phrase_lib;
	phrase_lib = new PhraseLib ();

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

		if (String ("-r") == argv [i]) {
			refine = true;
			continue;
		}

		if (String ("-z") == argv [i]) {
			zerofreq = true;
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

		if (String ("-a") == argv [i]) {
			if (++i >= argc) {
				std::cerr << "No argument for option " << argv [i-1] << "\n";
				return -1;
			}
			shift = atoi (argv [i]);
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

		std::cerr << "Invalid option: " << argv [i] << "\n";
		return -1;
	};
	
	if (!destfile) destfile = argv [1];

	if (!phrase_lib->load_lib (argv [1])) {
		std::cerr << "phrase library load failed!" << "\n";
		return -1;
	}

	phrase_lib->set_burst_stack_size (1);

	std::cout << "size of phrase library (in memory, byts): "
	     << phrase_lib->size () << "\n";

	if (refine) {
		std::cout << "Refining phrase library...\n";
		phrase_lib->refine_library (true);
		std::cout << "size of phrase library (in memory, byts) after refine: "
		     << phrase_lib->size () << "\n";
	}

	if (shift && !zerofreq) {
		Phrase phrase;
		std::cout << "Adjusting all phrases' frequencies...\n";
		if (shift > 0) {
			for (i = 0; i<(int)phrase_lib->number_of_phrases (); ++i) {
				phrase = phrase_lib->get_phrase_by_index (i);
				phrase.set_frequency (phrase.frequency () >> shift);
			}
		} else {
			shift = -shift;
			for (i = 0; i<(int)phrase_lib->number_of_phrases (); ++i) {
				phrase = phrase_lib->get_phrase_by_index (i);
				phrase.set_frequency (phrase.frequency () << shift);
			}
		}
	}

	if (zerofreq) {
		std::cout << "Setting all phrases' frequencies to zero...\n";
		for (i = 0; i<(int)phrase_lib->number_of_phrases (); ++i) {
			phrase_lib->get_phrase_by_index (i).set_frequency (0);
		}
	}

	if (corpusfile != NULL) {
		std::cout << "Adjusting phrase age...\n";
		self_learn (phrase_lib, corpusfile);
	}

	if (!phrase_lib->save_lib (destfile, binary)) {
		std::cout << "phrase library save failed!" << "\n";
	}

	delete phrase_lib;
	return 0;
}
