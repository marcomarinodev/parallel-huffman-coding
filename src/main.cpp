#include "../includes/tests.h"
#include "../includes/huffman.h"
#include "../includes/seq_solution.h"
#include "../includes/ff_solution.h"
#include "../includes/nt_solution.h"

using namespace std;

enum mode
{
	seq = 0,
	nt_par = 1,
	ff_par = 2
};

int main(int argc, char *argv[])
{

	bool perform_tests = argc > 1 ? strcmp(argv[1], "test") == 0 : false;
	int mode = argc > 2 ? atoi(argv[2]) : ff_par;
	int num_threads = argc > 3 ? atoi(argv[3]) : 1;
	string filename = argc > 4 ? argv[4] : "./inputs/10M";
	bool do_parallel_policy = mode != seq;

	if (perform_tests)
	{
		cout << "=> Performing tests <=" << endl;
		test_fastflow_encoding(1000);
		test_native_threads_encoding(1000);
		return 0;
	}

	cout << "analyzing: " << filename << endl;
	vector<char> chars;
	string encoded_input;
	long read_time;
	{
		utimer read_timer("read timer", &read_time);
		chars = read_file(filename);
	}
	cout << "read time: " << read_time << endl;

	for (int j = 0; j < 10; j++)
	{
		long execution_time;
		{
			utimer execution_timer("exec time", &execution_time);
			// ----------------------------
			// ------ Conting chars -------
			// ----------------------------
			map<char, int> par_map_chars, seq_map_chars;

			if (do_parallel_policy)
			{
				cout << "file big => parallel counting" << endl;
				if (mode == nt_par)
				{
					par_map_chars = nt_solution::count_chars(chars, num_threads);
					// par_map_chars = nt_solution::Gmr(num_threads, num_threads / 2 + 1).count_chars(chars);
				}

				else
					par_map_chars = ff_solution::count_chars(chars, num_threads);
			}
			else
			{
				cout << "file small => sequential counting" << endl;
				seq_map_chars = seq_solution::count_chars(chars);
			}

			// ----------------------------------
			// ------ Build Encoding Table ------
			// ----------------------------------
			min_heap_node *huffman_tree = build_huffman_tree(do_parallel_policy ? par_map_chars : seq_map_chars);
			unordered_map<char, string> encoding_table = build_encoding_table(huffman_tree);

			// -----------------------------
			// ------ Encoding phase -------
			// -----------------------------
			string encoded_string;
			if (do_parallel_policy)
			{
				cout << "file big => parallel encoding" << endl;
				if (mode == nt_par)
					encoded_string = nt_solution::encode(chars, encoding_table, num_threads);
				else
					encoded_string = ff_solution::encode(chars, encoding_table, num_threads);
			}
			else
			{
				cout << "file small => sequential encoding" << endl;
				encoded_string = seq_solution::encode(chars, encoding_table);
			}

			// -----------------------------
			// --------- Compress ----------
			// -----------------------------

			if (do_parallel_policy)
			{
				if (mode == nt_par)
					encoded_input = nt_solution::compress(encoded_string, num_threads);
				else
					encoded_input = ff_solution::compress(encoded_string, num_threads);
			}
			else
			{
				encoded_input = seq_solution::compress(encoded_string);
			}
		}

		cout << "execution time: " << execution_time << endl;
	}

	cout << "writing compressed file " << endl;
	seq_write_file_string("./outputs/compressed", encoded_input);

	return 0;
}
