#include <TextAnalyzer.hpp>

using namespace tean;
using namespace std;

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
	setlocale(0, LC_ALL);
	fs::path file_path{};
	if (argc >= 2)
		file_path = argv[1];
	else
	{
		cout << "Input file path >> ";
		string file_path_str{};
		getline(cin, file_path_str);
		erase(file_path_str, ' ');
		erase(file_path_str, '\n');
		cout << file_path_str << endl;
		file_path = file_path_str;
		cout << file_path << endl;
	}
	if (file_path == " " || !filesystem::exists(file_path))
	{
		cout << "Invalid file path" << endl;
		return -1;
	}
	cout << format("Path to .txt file - {}", file_path.string()) << endl;
	string substr{};
	if (argc >= 3)
	{
		substr = string(argv[2]);
	}
	else
	{
		cout << "Input required substring >> ";
		getline(cin, substr);
	}

	erase(substr, ' ');
	cout << format("String to find - {}", substr) << endl;

	int threads_count{};

	try
	{
		if (argc >= 4)
			threads_count = stoi(argv[3]);
		else
		{
			cout << "Input threads count >> ";
			cin >> threads_count;
		}

		if(threads_count == 0)
		{
			throw out_of_range("Zero threads given");
		}

		cout << format("Threads count - {}", threads_count) << endl;
	}
	catch(const std::out_of_range& e)
	{
		std::cerr << e.what() << '\n';
		cout << format("Threads count set to ", THREADS_DEFAULT) << endl;
		threads_count = THREADS_DEFAULT;
	}
	
	tean::text_analyzer text1(file_path.string());

	auto begin = chrono::system_clock::now();

	size_t positions_count = text1.find_substr(substr, threads_count);

	auto end = chrono::system_clock::now();

	if (argc >= 5 && stoi(argv[4]) == 1)
	{
		auto text_positions = text1.get_indexes();

		for (const auto& pos : text_positions)
		{
			cout << pos;
		}
	}
	cout << format("Indexes found - {}", positions_count) << endl;
	cout << format("Program time - {} mcs\n", chrono::duration_cast<chrono::microseconds>(end - begin).count());
	//cout << format("New operator memory usage - {} bytes", new_bytes.load()) << endl;

	return 0;
}